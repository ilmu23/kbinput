// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<listener.c>>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "internal/_kbinput.h"
#include "internal/utils/vector.h"

typedef struct __key_group {
	vector	keys;
	u32		code;
}	key_group;

#define _BUFFER_SIZE	64

#define get_group(v, i)	((key_group *)vector_get(v, i))
#define get_key(v, i)	((kbinput_key *)vector_get(v, i))
#define in_bounds(p)	(p != (void *)VECTOR_INDEX_OUT_OF_BOUNDS)

#define check_mods(x, y)	((y & KB_MOD_IGN_LCK) ? (x & ~(KB_MOD_CAPS_LCK | KB_MOD_NUM_LCK)) == (y & ~KB_MOD_IGN_LCK) : x == y)

static inline kbinput_key	*_listen_kitty(const kbinput_listener_id id);
static inline kbinput_key	*_listen_legacy(const kbinput_listener_id id);
static inline kbinput_key	*_find_key(const vector vec, const kbinput_key *key);
static inline kbinput_key	*_new_key(const kbinput_key *key);
static inline key_group		*_new_group(const u32 code);
static inline size_t		_find_group(const vector vec, const kbinput_key *key);
static inline size_t		_strlcpy(char * dst, const char *src, const size_t n);
static inline size_t		_seqlen(const char *s);
static inline char			*_substr(const char *s, const size_t start, const size_t len);
static inline void			_parse_modifiers(const char *param, kbinput_key *key);
static inline void			_parse_key_code(const char *param, kbinput_key *key);
static inline u8			_insert_key(vector vec, const kbinput_key *key);

static void	_free_key_group(void *group);

static struct {
	const kbinput_listener_id	id;
	vector						uc_keys;
	vector						sp_keys;
	char						buf[_BUFFER_SIZE + 1];
	u8							in_use;
}	listeners[MAX_LISTENERS] = {
	{.id = 0, .uc_keys = NULL, .sp_keys = NULL, .in_use = 0},
	{.id = 1, .uc_keys = NULL, .sp_keys = NULL, .in_use = 0},
	{.id = 2, .uc_keys = NULL, .sp_keys = NULL, .in_use = 0},
	{.id = 3, .uc_keys = NULL, .sp_keys = NULL, .in_use = 0},
	{.id = 4, .uc_keys = NULL, .sp_keys = NULL, .in_use = 0},
	{.id = 5, .uc_keys = NULL, .sp_keys = NULL, .in_use = 0},
	{.id = 6, .uc_keys = NULL, .sp_keys = NULL, .in_use = 0},
	{.id = 7, .uc_keys = NULL, .sp_keys = NULL, .in_use = 0},
	{.id = 8, .uc_keys = NULL, .sp_keys = NULL, .in_use = 0},
	{.id = 9, .uc_keys = NULL, .sp_keys = NULL, .in_use = 0}
};

u8	input_protocol;

kbinput_listener_id	kbinput_new_listener(void) {
	size_t	i;

	for (i = 0; i < MAX_LISTENERS; i++)
		if (!listeners[i].in_use)
			break ;
	if (i == MAX_LISTENERS)
		return KB_LISTENER_LIST_FULL;
	listeners[i].uc_keys = vector(key_group *, 64, _free_key_group);
	listeners[i].sp_keys = vector(key_group *, 16, _free_key_group);
	if (!listeners[i].uc_keys || !listeners[i].sp_keys) {
		vector_delete(listeners[i].uc_keys);
		vector_delete(listeners[i].sp_keys);
		return -1;
	}
	listeners[i].in_use = 1;
	return listeners[i].id;
}

const kbinput_key	*kbinput_listen(const kbinput_listener_id id) {
	const kbinput_key	*key;

	switch (input_protocol) {
		case KB_INPUT_PROTOCOL_KITTY:
			key = _listen_kitty(id);
			break ;
		case KB_INPUT_PROTOCOL_LEGACY:
			key = _listen_legacy(id);
			break ;
		default:
			key = NULL;
	}
	return key;
}

void	kbinput_delete_listener(const kbinput_listener_id id) {
	if (id >= 0 && id < MAX_LISTENERS && listeners[id].in_use) {
		vector_delete(listeners[id].uc_keys);
		vector_delete(listeners[id].sp_keys);
		listeners[id].in_use = 0;
	}
}

u8	kbinput_add_listener(const kbinput_listener_id id, const kbinput_key key) {
	vector	vec;

	if (id < 0 || id >= MAX_LISTENERS)
		return 0;
	vec = (key.code.type == KB_KEY_TYPE_UNICODE) ? listeners[id].uc_keys : listeners[id].sp_keys;
	return (_insert_key(vec, &key));
}

u8	kbinput_get_input_protocol(void) {
	return input_protocol;
}

[[maybe_unused]] static inline void	_print_buf(const kbinput_listener_id id) {
	size_t	i;

	for (i = 0; listeners[id].buf[i]; i++) {
		if (listeners[id].buf[i] == '\x1b')
			fprintf(stderr, "\\x1b");
		else
			fputc(listeners[id].buf[i], stderr);
	}
	fputc('\n', stderr);
}

static inline kbinput_key	*_listen_kitty(const kbinput_listener_id id) {
	kbinput_key	key;
	kbinput_key	*out;
	ssize_t		rv;
	size_t		seq_len;
	size_t		buf_len;
	size_t		j;
	size_t		i;
	char		*params[2];

	out = NULL;
	while (!out) {
		if (!*listeners[id].buf) {
			rv = read(0, listeners[id].buf, _BUFFER_SIZE);
			if (rv == -1)
				return NULL;
#ifdef __DEBUG_ECHO_SEQS
			_print_buf();
#endif /* __DEBUG_ECHO_SEQS */
		}
		seq_len = _seqlen(listeners[id].buf);
		for (i = j = sizeof(CSI) - 1; listeners[id].buf[j] && listeners[id].buf[j] != ';'; j++)
			;
		params[0] = _substr(listeners[id].buf, i, j - i);
		for (i = ++j; listeners[id].buf[j] && listeners[id].buf[j] != ';'; j++)
			;
		params[1] = _substr(listeners[id].buf, i, j - i);
		key.code.type = (listeners[id].buf[seq_len - 1] == 'u') ? KB_KEY_TYPE_UNICODE : KB_KEY_TYPE_SPECIAL;
		_parse_key_code(params[0], &key);
		_parse_modifiers(params[1], &key);
		out = _find_key((key.code.type == KB_KEY_TYPE_UNICODE) ? listeners[id].uc_keys : listeners[id].sp_keys, &key);
		buf_len = strlen(listeners[id].buf);
		memmove(listeners[id].buf, &listeners[id].buf[seq_len], buf_len - seq_len);
		for (i = 0; i < seq_len; i++)
			listeners[id].buf[buf_len - seq_len + i] = '\0';
		memset(&key, 0, sizeof(key));
		free(params[1]);
		free(params[0]);
	}
	return out;
}

static inline kbinput_key	*_listen_legacy(const kbinput_listener_id id) {
	return (void *)(uintptr_t)id;
}

static inline kbinput_key	*_find_key(const vector vec, const kbinput_key *key) {
	kbinput_key	*out;
	key_group	*group;
	size_t		size;
	size_t		i;

	group = get_group(vec, _find_group(vec, key));
	if (!in_bounds(group) || group->code != key->code.unicode)
		return NULL;
	for (i = 0, size = vector_size(group->keys); i < size; i++) {
		out = get_key(group->keys, i);
		if (check_mods(key->modifiers, out->modifiers) && key->event_type == out->event_type)
			break ;
	}
	return (i < size) ? out : NULL;
}

static inline kbinput_key	*_new_key(const kbinput_key *key) {
	kbinput_key	*out;

	out = malloc(sizeof(*out));
	if (out) {
		*out = (kbinput_key){
			.code.unicode = key->code.unicode,
			.code.type = key->code.type,
			.modifiers = key->modifiers,
			.event_type = key->event_type,
			.fn = key->fn
		};
	}
	return out;
}

static inline key_group	*_new_group(const u32 code) {
	key_group	*out;

	out = malloc(sizeof(*out));
	if (out) {
		*out = (key_group){
			.keys = vector(kbinput_key *, 8, free),
			.code = code
		};
		if (!out->keys) {
			free(out);
			out = NULL;
		}
	}
	return out;
}

static inline size_t	_find_group(const vector vec, const kbinput_key *key) {
	key_group	*group;
	size_t		size;
	size_t		i;

	for (i = 0, size = vector_size(vec); i < size; i++) {
		group = get_group(vec, i);
		if (group->code >= key->code.unicode)
			break ;
	}
	return i;
}

static inline size_t	_strlcpy(char * dst, const char *src, const size_t n) {
	size_t	len;

	len = strnlen(src, n - 1);
	memcpy(dst, src, len);
	dst[len] = '\0';
	return strlen(src);
}

static inline size_t	_seqlen(const char *s) {
	size_t	i;

	for (i = sizeof(CSI) - 1; s[i] && s[i] != '\x1b'; i++)
		;
	return i;
}

static inline char	*_substr(const char *s, const size_t start, const size_t len) {
	size_t	_len;
	char	*out;

	_len = strnlen(&s[start], len) + 1;
	out = malloc(_len * sizeof(*out));
	if (out)
		_strlcpy(out, &s[start], _len);
	return out;
}

static inline void	_parse_modifiers(const char *param, kbinput_key *key) {
	const char	*end;
	u8			canon_event;

	key->modifiers = strtoul(param, (char **)&end, 10);
	if (key->modifiers)
		key->modifiers--;
	canon_event = (*end == ':') ? strtoul(++end, NULL, 10) : 1;
	switch(canon_event) {
		case 1:
			key->event_type = KB_EVENT_PRESS;
			break ;
		case 2:
			key->event_type = KB_EVENT_REPEAT;
			break ;
		case 3:
			key->event_type = KB_EVENT_RELEASE;
	}
}

static inline void	_parse_key_code(const char *param, kbinput_key *key) {
	size_t	i;

	if (key->code.type == KB_KEY_TYPE_UNICODE)
		key->code.unicode = strtoul(param, NULL, 10);
	else for (i = 0; param[i]; i++)
		key->code.special |= param[i] << (i * 8);
}

static inline u8	_insert_key(vector vec, const kbinput_key *key) {
	kbinput_key	*_key;
	key_group	*group;
	size_t		i;
	u8			match;

	i = _find_group(vec, key);
	group = get_group(vec, i);
	if (!in_bounds(group) || group->code != key->code.unicode) {
		group = _new_group(key->code.unicode);
		if (!group || !vector_insert(vec, i, group))
			return 0;
		_key = _new_key(key);
		return (_key) ? vector_push(group->keys, _key) : 0;
	}
	for (i = match = 0, _key = get_key(group->keys, i); in_bounds(_key); _key = get_key(group->keys, ++i)) {
		if (key->modifiers == _key->modifiers && key->event_type == _key->event_type) {
			match = 1;
			break ;
		} else if (key->modifiers > _key->modifiers)
			break ;
	}
	if (!match) {
		_key = _new_key(key);
		return (_key) ? vector_insert(group->keys, i, _key) : 0;
	} else
		_key->fn = key->fn;
	return 1;
}

static void	_free_key_group(void *group) {
	if (group) {
		vector_delete(((key_group *)group)->keys);
		free(group);
	}
}
