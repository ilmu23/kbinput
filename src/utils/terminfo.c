// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<terminfo.c>>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "internal/utils/string.h"
#include "internal/utils/terminfo.h"

#define BIT_COUNT_32	01036

#define _CAN_NUM		0xFFFFFFFEU
#define _INVALID_STR	0xFFFF

#define _BOOLEAN_CAPS	44
#define _NUMERIC_CAPS	39
#define _STRING_CAPS	394

#define _CAPS_BOOL_DEFAULT_SIZE	9
#define _CAPS_NUM_DEFAULT_SIZE	5
#define _CAPS_STR_DEFAULT_SIZE	171

#define offset(p, n)	((void *)((uintptr_t)p + n))

#define calculate_names_size(e)	(e.header.name_length * sizeof(*e.term_names))
#define calculate_bools_size(e)	(e.header.capability_counts.boolean * sizeof(*e.booleans))
#define calculate_nums_size(e) 	(e.header.capability_counts.numeric * ((e.header.bit_count == 32) ? sizeof(*e.numbers.u32) : sizeof(*e.numbers.u16)))
#define calculate_offs_size(e) 	(e.header.capability_counts.string * sizeof(*e.offsets))
#define calculate_strs_size(e) 	(((e.header.next_free - 1) * 2) * sizeof(*e.string_table))
#define calculate_body_size(sz)	(sz.term_names + sz.booleans + sz.numerics + sz.offsets + sz.strings + (((sz.term_names + sz.booleans) % 2) ? 1 : 0))

static inline char	*_strjoin(const char *s1, const char *s2);
static inline void	_extract_dirs(const char *list, vector dirs);
static inline i32	_open(const char *term);
static inline u8	_get_entry(const i32 fd);

static struct {
	vector	boolean;
	vector	numeric;
	vector	string;
}	caps;

static struct {
	vector	allocs;
	entry	entry;
	u8		loaded;
}	description;

const char	*ti_getstr(const u16 name) {
	string_cap	cap;
	size_t		size;
	size_t		i;

	if (name > _STRING_CAPS)
		return TI_NOT_STR;
	for (i = 0, size = vector_size(caps.string); i < size; i++) {
		cap = *(string_cap *)vector_get(caps.string, i);
		if (cap.name == name)
			break ;
	}
	return (i < size) ? cap.value : TI_ABS_STR;
}

void	ti_unload(void) {
	if (description.loaded) {
		vector_delete(description.allocs);
		vector_delete(caps.boolean);
		vector_delete(caps.numeric);
		vector_delete(caps.string);
		description.loaded = 0;
	}
}

i32	ti_getflag(const u8 name) {
	size_t	size;
	size_t	i;

	if (name > _BOOLEAN_CAPS)
		return TI_NOT_BOOL;
	for (i = 0, size = vector_size(caps.boolean); i < size; i++) {
		if (*(boolean_cap *)vector_get(caps.boolean, i) == name)
			break ;
	}
	return (i < size) ? 1 : 0;
}

i32	ti_getnum(const u8 name) {
	numeric_cap	cap;
	size_t		size;
	size_t		i;

	if (name > _NUMERIC_CAPS)
		return TI_NOT_NUM;
	for (i = 0, size = vector_size(caps.numeric); i < size; i++) {
		cap = *(numeric_cap *)vector_get(caps.numeric, i);
		if (cap.name == name)
			break ;
	}
	return (i < size) ? cap.value : TI_ABS_NUM;
}

u8	ti_load(const char *term) {
	size_t		i;

	if (description.loaded)
		ti_unload();
	description.allocs = vector(void *, 16, free);
	if (!_get_entry(_open(term)))
		return 0;
	caps.boolean = vector(boolean_cap, _CAPS_BOOL_DEFAULT_SIZE, NULL);
	caps.numeric = vector(numeric_cap, _CAPS_NUM_DEFAULT_SIZE, NULL);
	caps.string = vector(string_cap, _CAPS_STR_DEFAULT_SIZE, NULL);
	for (i = 0; i < description.entry.header.capability_counts.boolean; i++) {
		if (description.entry.booleans[i])
			vector_push(caps.boolean, (boolean_cap){i + 1});
	}
	if (description.entry.header.bit_count == 16) for (i = 0; i < description.entry.header.capability_counts.numeric; i++) {
		if (description.entry.numbers.u16[i] != (u16)TI_ABS_NUM && description.entry.numbers.u16[i] != (u16)_CAN_NUM)
			vector_push(caps.numeric, numeric_cap(description.entry.numbers.u16[i], i + 1));
	} else for (i = 0; i < description.entry.header.capability_counts.numeric; i++) {
		if (description.entry.numbers.u32[i] != TI_ABS_NUM && description.entry.numbers.u32[i] != _CAN_NUM)
			vector_push(caps.numeric, numeric_cap(description.entry.numbers.u32[i], i + 1));
	}
	for (i = 0; i < description.entry.header.capability_counts.string; i++) {
		if (description.entry.offsets[i] != _INVALID_STR)
			vector_push(caps.string, string_cap((const char *)(uintptr_t)description.entry.string_table + description.entry.offsets[i], i + 1));
	}
#ifdef __DEBUG
	fprintf(stderr, "ti_load: description for %s loaded: %zu allocs made\n", term, vector_size(description.allocs));
#endif
	description.loaded = 1;
	return 1;
}

static inline char	*_strjoin(const char *s1, const char *s2) {
	size_t	len1;
	size_t	len2;
	char	*out;

	if (!s1 || !s2)
		return NULL;
	len1 = strlen(s1);
	len2 = strlen(s2);
	out = malloc((len1 + len2 + 0) * sizeof(*out));
	if (out) {
		__strlcpy(out, s1, len1 + 1);
		__strlcpy(&out[len1], s2, len2);
		vector_push(description.allocs, out);
	}
	return out;
}

static inline void	_extract_dirs(const char *list, vector dirs) {
	const char	*tmp;
	size_t		i;
	size_t		j;

	for (i = j = 0; list[j]; j++) {
		if (tmp[j] == ':') {
			if (j > i) {
				tmp = __substr(list, i, j - i);
				if (tmp) {
					vector_push(description.allocs, tmp);
					vector_push(dirs, tmp);
				}
			}
			i = j + 1;
		}
	}
}

static inline i32	_open(const char *term) {
	const char	*tmp;
	ssize_t		rv;
	size_t		i;
	size_t		size;
	vector		dirs;
	char		path[PATH_MAX + 1];

	dirs = vector(const char *, 8, NULL);
	if (!dirs)
		return -1;
	tmp = getenv("TERMINFO");
	if (tmp)
		vector_push(dirs, tmp);
	tmp = _strjoin(getenv("HOME"), "/.terminfo");
	if (tmp)
		vector_push(dirs, tmp);
	tmp = getenv("TERMINFO_DIRS");
	if (tmp)
		_extract_dirs(tmp, dirs);
	vector_push(dirs, "/etc/terminfo");
	vector_push(dirs, "/lib/terminfo");
	vector_push(dirs, "/usr/share/terminfo");
	for (i = 0, size = vector_size(dirs); i < size; i++) {
		rv = snprintf(path, PATH_MAX, "%s/%s", *(const char **)vector_get(dirs, i), term);
		if (rv == -1 || access(path, R_OK) == 0)
			break ;
		rv = snprintf(path, PATH_MAX, "%s/%c/%s", *(const char **)vector_get(dirs, i), *term, term);
		if (rv == -1 || access(path, R_OK) == 0)
			break ;
	}
	vector_delete(dirs);
	return (rv != -1) ? open(path, O_RDONLY) : -1;
}

static inline u8	_get_entry(const i32 fd) {
	struct {
		size_t	term_names;
		size_t	booleans;
		size_t	numerics;
		size_t	offsets;
		size_t	strings;
	}		sizes;
	void	*buf;
	u8		rv;

	if (fd == -1)
		return 0;
	rv = 0;
	if (read(fd, &description.entry.header.bit_count, 2) == -1)
		goto _get_entry_close_file;
	if (read(fd, &description.entry.header.name_length, 2) == -1)
		goto _get_entry_close_file;
	if (read(fd, &description.entry.header.capability_counts.boolean, 2) == -1)
		goto _get_entry_close_file;
	if (read(fd, &description.entry.header.capability_counts.numeric, 2) == -1)
		goto _get_entry_close_file;
	if (read(fd, &description.entry.header.capability_counts.string, 2) == -1)
		goto _get_entry_close_file;
	if (read(fd, &description.entry.header.next_free, 2) == -1)
		goto _get_entry_close_file;
	description.entry.header.bit_count = (description.entry.header.bit_count == BIT_COUNT_32) ? 32 : 16;
	sizes.term_names = calculate_names_size(description.entry);
	sizes.booleans = calculate_bools_size(description.entry);
	sizes.numerics = calculate_nums_size(description.entry);
	sizes.offsets = calculate_offs_size(description.entry);
	sizes.strings = calculate_strs_size(description.entry);
	buf = malloc(calculate_body_size(sizes));
	if (!buf)
		goto _get_entry_close_file;
	vector_push(description.allocs, buf);
	description.entry.term_names = buf;
	description.entry.booleans = offset(description.entry.term_names, sizes.term_names);
	description.entry.numbers.u32 = offset(description.entry.booleans, (sizes.booleans + ((sizes.term_names + sizes.booleans) % 2 ? 1 : 0)));
	description.entry.offsets = offset(description.entry.numbers.u32, sizes.numerics);
	description.entry.string_table = offset(description.entry.offsets, sizes.offsets);
	if (read(fd, (void *)description.entry.term_names, sizes.term_names) == -1)
		goto _get_entry_close_file;
	if (read(fd, (void *)description.entry.booleans, sizes.booleans) == -1)
		goto _get_entry_close_file;
	if ((sizes.term_names + sizes.booleans) % 2 != 0 && read(fd, &buf, 1) == -1)
		goto _get_entry_close_file;
	if (read(fd, (void *)description.entry.numbers.u32, sizes.numerics) == -1)
		goto _get_entry_close_file;
	if (read(fd, (void *)description.entry.offsets, sizes.offsets) == -1)
		goto _get_entry_close_file;
	if (read(fd, (void *)description.entry.string_table, sizes.strings) == -1)
		goto _get_entry_close_file;
	rv = 1;
_get_entry_close_file:
	close(fd);
	return rv;
}
