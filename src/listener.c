// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<listener.c>>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "defs.h"
#include "internal/_kbinput.h"
#include "internal/utils/string.h"
#include "internal/utils/vector.h"

typedef struct __key_group {
	vector	keys;
	u32		code;
}	key_group;

#define _BUFFER_SIZE	64

#define _LEGACY_MOD_MASK	(KB_MOD_IGN_LCK | KB_MOD_ALT | KB_MOD_CTRL)

#define get_group(v, i)	((key_group **)vector_get(v, i))
#define get_key(v, i)	((kbinput_key **)vector_get(v, i))
#define in_bounds(p)	(p != (void *)VECTOR_INDEX_OUT_OF_BOUNDS)

#define check_mods(x, y)	((y & KB_MOD_IGN_LCK) ? (x & ~(KB_MOD_CAPS_LCK | KB_MOD_NUM_LCK)) == (y & ~KB_MOD_IGN_LCK) : x == y)

static inline kbinput_key	*_listen_kitty(const kbinput_listener_id id);
static inline kbinput_key	*_listen_legacy(const kbinput_listener_id id);
static inline kbinput_key	*_find_key(const vector vec, const kbinput_key *key);
static inline kbinput_key	*_new_key(const kbinput_key *key);
static inline key_group		*_new_group(const u32 code);
static inline size_t		_find_group(const vector vec, const kbinput_key *key);
static inline size_t		_seqlen(const char *s);
static inline void			_kitty_parse_modifiers(const char *param, kbinput_key *key);
static inline void			_kitty_parse_key_code(const char *param, kbinput_key *key, const u8 key_type);
static inline void			_legacy_check_backspace(const u8 c, kbinput_key *key);
static inline u32			_legacy_parse_codepoint(const char *buf, const size_t buf_len);
static inline u8			_legacy_parse_escape(const char *seq, kbinput_key *key, size_t *seq_len);
static inline u8			_check_legacy_compatability(const kbinput_key *key);
static inline u8			_insert_key(vector vec, const kbinput_key *key);
static inline u8			_show_cursor(void);
static inline u8			_hide_cursor(void);

static void	_free_key_group(void *group);

static struct {
	const kbinput_listener_id	id;
	vector						keys;
	char						buf[_BUFFER_SIZE + 1];
	u8							in_use;
}	listeners[MAX_LISTENERS] = {
	{.id = 0, .keys = NULL, .in_use = 0},
	{.id = 1, .keys = NULL, .in_use = 0},
	{.id = 2, .keys = NULL, .in_use = 0},
	{.id = 3, .keys = NULL, .in_use = 0},
	{.id = 4, .keys = NULL, .in_use = 0},
	{.id = 5, .keys = NULL, .in_use = 0},
	{.id = 6, .keys = NULL, .in_use = 0},
	{.id = 7, .keys = NULL, .in_use = 0},
	{.id = 8, .keys = NULL, .in_use = 0},
	{.id = 9, .keys = NULL, .in_use = 0}
};

struct {
	kbinput_cursor_mode	desired;
	kbinput_cursor_mode	current;
}	cursor_mode = {
	.desired = SHOW,
	.current = ON
};

u8	input_protocol;

kbinput_listener_id	kbinput_new_listener(void) {
	size_t	i;

	for (i = 0; i < MAX_LISTENERS; i++)
		if (!listeners[i].in_use)
			break ;
	if (i == MAX_LISTENERS)
		return KB_LISTENER_LIST_FULL;
	switch (input_protocol) {
		case KB_INPUT_PROTOCOL_KITTY:
			listeners[i].keys = vector(key_group *, 64, _free_key_group);
			break ;
		case KB_INPUT_PROTOCOL_LEGACY:
			listeners[i].keys = vector(key_group *, 64, _free_key_group);
			break ;
		case KB_INPUT_PROTOCOL_ERROR:
			return -1;
	}
	if (!listeners[i].keys) {
		vector_delete(listeners[i].keys);
		return -1;
	}
	listeners[i].in_use = 1;
	return listeners[i].id;
}

const kbinput_key	*kbinput_listen(const kbinput_listener_id id) {
	const kbinput_key	*key;

	if (switch_term_mode(TERM_MODE_RAW) == -1 || !_show_cursor())
		return NULL;
	switch (input_protocol) {
		case KB_INPUT_PROTOCOL_KITTY:
			if (write(1, TERM_ENABLE_ENHANCEMENTS, sizeof(TERM_ENABLE_ENHANCEMENTS)) == -1)
				return NULL;
			key = _listen_kitty(id);
			if (write(1, TERM_DISABLE_ENHANCEMENTS, sizeof(TERM_DISABLE_ENHANCEMENTS)) == -1)
				return NULL;
			break ;
		case KB_INPUT_PROTOCOL_LEGACY:
			key = _listen_legacy(id);
			break ;
		default:
			key = NULL;
	}
	return (switch_term_mode(TERM_MODE_NORMAL) == -1 || !_hide_cursor()) ? NULL : key;
}

void	kbinput_delete_listener(const kbinput_listener_id id) {
	if (id >= 0 && id < MAX_LISTENERS && listeners[id].in_use) {
		vector_delete(listeners[id].keys);
		listeners[id].in_use = 0;
	}
}

i8	kbinput_add_listener(const kbinput_listener_id id, const kbinput_key key) {
	if (id < 0 || id >= MAX_LISTENERS)
		return KB_INVALID_LISTENER_ID;
	if (input_protocol == KB_INPUT_PROTOCOL_LEGACY && !_check_legacy_compatability(&key))
		return KB_OPTION_NOT_SUPPORTED;
	return (_insert_key(listeners[id].keys, &key));
}

u8	kbinput_set_cursor_mode(const kbinput_cursor_mode mode) {
	cursor_mode.desired = mode;
	return _hide_cursor();
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
	u8			key_type;

	out = NULL;
	while (!out) {
		key = kbinput_key(0, 0, KB_EVENT_PRESS, NULL);
		if (!*listeners[id].buf) {
			rv = read(0, listeners[id].buf, _BUFFER_SIZE);
			if (rv < 1)
				return NULL;
#ifdef __DEBUG_ECHO_SEQS
			_print_buf(id);
#endif /* __DEBUG_ECHO_SEQS */
		}
		seq_len = _seqlen(listeners[id].buf);
		for (i = j = sizeof(CSI) - 1; listeners[id].buf[j] && listeners[id].buf[j] != ';'; j++)
			;
		params[0] = __substr(listeners[id].buf, i, j - i);
		for (i = ++j; listeners[id].buf[j] && listeners[id].buf[j] != ';'; j++)
			;
		params[1] = __substr(listeners[id].buf, i, j - i);
		key_type = listeners[id].buf[seq_len - 1];
		_kitty_parse_key_code(params[0], &key, key_type);
		_kitty_parse_modifiers(params[1], &key);
		out = _find_key(listeners[id].keys, &key);
		if (out)
			out->text = (listeners[id].buf[j++] == '\0') ? 0 : strtoul(&listeners[id].buf[j], NULL, 10);
		buf_len = strlen(listeners[id].buf);
		if (buf_len > seq_len) {
			memmove(listeners[id].buf, &listeners[id].buf[seq_len], buf_len - seq_len);
			memset(&listeners[id].buf[buf_len - seq_len], 0, seq_len);
		} else
			memset(listeners[id].buf, 0, buf_len);
		free(params[1]);
		free(params[0]);
	}
	return out;
}

static inline kbinput_key	*_listen_legacy(const kbinput_listener_id id) {
	kbinput_key	*out;
	kbinput_key	key;
	ssize_t		rv;
	size_t		buf_len;
	size_t		i;

	out = NULL;
	while (!out) {
		key = kbinput_key(0, 0, KB_EVENT_PRESS, NULL);
		if (!*listeners[id].buf) {
			rv = read(0, listeners[id].buf, 8);
			if (rv < 1)
				return NULL;
		} else
			rv = strlen(listeners[id].buf);
		i = 0;
		if (_legacy_parse_escape(listeners[id].buf, &key, &i))
			goto _listen_legacy_match_key;
		if (listeners[id].buf[0] == '\x1b') switch (listeners[id].buf[1]) {
			case '\0':
				key = kbinput_key('\x1b', 0, KB_EVENT_PRESS, NULL);
				goto _listen_legacy_match_key;
			default:
				key.modifiers |= KB_MOD_ALT;
				i++;
		}
		if (listeners[id].buf[i] == '\x7f' || listeners[id].buf[i] == '\x8')
			_legacy_check_backspace(listeners[id].buf[i], &key);
		else if (isascii(listeners[id].buf[i])) {
			if (listeners[id].buf[i] == '\x1b')
				key.code = '\x1b';
			else if (listeners[id].buf[i] < ' ') switch (listeners[id].buf[i]) {
				case '\x9':
					key.code = '\t';
					break ;
				case '\x0':
					key.modifiers |= KB_MOD_CTRL;
					key.code = ' ';
					break ;
				case '\xd':
					key.code = '\r';
					break ;
				case '\x1c':
					key.modifiers |= KB_MOD_CTRL;
					key.code = '\\';
					break ;
				case '\x1d':
					key.modifiers |= KB_MOD_CTRL;
					key.code = ']';
					break ;
				case '\x1e':
					key.modifiers |= KB_MOD_CTRL;
					key.code = '^';
					break ;
				case '\x1f':
					key.modifiers |= KB_MOD_CTRL;
					key.code = '/';
					break ;
				default:
					key.modifiers |= KB_MOD_CTRL;
					key.code = listeners[id].buf[i] + '`';
			} else
				key.code = listeners[id].buf[i];
		} else {
			key.code = _legacy_parse_codepoint(&listeners[id].buf[i], rv - i);
			if (key.code == UINT32_MAX)
				return NULL;
			if (key.code > 0x7F)
				i = min(i + 1, (size_t)rv);
			else if (key.code > 0x7FF)
				i = min(i + 2, (size_t)rv);
			else
				i = min(i + 3, (size_t)rv);
		}
_listen_legacy_match_key:
		out = _find_key(listeners[id].keys, &key);
		buf_len = max(strlen(listeners[id].buf), 1);
		i++;
		memmove(listeners[id].buf, &listeners[id].buf[i], buf_len - i);
		memset(&listeners[id].buf[buf_len - i], 0, i);
	}
	return out;
}

static inline kbinput_key	*_find_key(const vector vec, const kbinput_key *key) {
	kbinput_key	*out;
	key_group	*group;
	size_t		size;
	size_t		i;
	void		*tmp;

	tmp = get_group(vec, _find_group(vec, key));
	group = (tmp && in_bounds(tmp)) ? *(key_group **)tmp : NULL;
	if (!group || group->code != key->code)
		return NULL;
	for (i = 0, size = vector_size(group->keys); i < size; i++) {
		out = *get_key(group->keys, i);
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
			.fn = key->fn,
			.code = key->code,
			.modifiers = key->modifiers,
			.event_type = key->event_type
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
		group = *get_group(vec, i);
		if (group->code >= key->code)
			break ;
	}
	return i;
}

static inline size_t	_seqlen(const char *s) {
	size_t	i;

	for (i = sizeof(CSI) - 1; s[i] && s[i] != '\x1b'; i++)
		;
	return i;
}

static inline u8	_legacy_parse_escape(const char *seq, kbinput_key *key, size_t *seq_len) {
	size_t	i;
	u16		escape_name;

	escape_name = parse_legacy_key_seq(seq);
	if (escape_name == LEGACY_SEQ_NO_MATCH) {
		if (strncmp(seq, "\x1b[", 2) == 0) {
			for (i = 2; seq[i]; i++)
				if (seq[i] >= '\x40')
					break ;
			*seq_len = i - 1;
		}
		return 0;
	}
	*seq_len = strlen(ti_getstr(escape_name)) - 1;
	switch (escape_name) {
		case ti_kf0:
			*key = kbinput_key(KB_KEY_LEGACY_F0, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf1:
			*key = kbinput_key(KB_KEY_LEGACY_F1, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf2:
			*key = kbinput_key(KB_KEY_LEGACY_F2, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf3:
			*key = kbinput_key(KB_KEY_LEGACY_F3, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf4:
			*key = kbinput_key(KB_KEY_LEGACY_F4, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf5:
			*key = kbinput_key(KB_KEY_LEGACY_F5, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf6:
			*key = kbinput_key(KB_KEY_LEGACY_F6, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf7:
			*key = kbinput_key(KB_KEY_LEGACY_F7, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf8:
			*key = kbinput_key(KB_KEY_LEGACY_F8, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf9:
			*key = kbinput_key(KB_KEY_LEGACY_F9, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf10:
			*key = kbinput_key(KB_KEY_LEGACY_F10, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf11:
			*key = kbinput_key(KB_KEY_LEGACY_F11, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf12:
			*key = kbinput_key(KB_KEY_LEGACY_F12, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf13:
			*key = kbinput_key(KB_KEY_LEGACY_F13, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf14:
			*key = kbinput_key(KB_KEY_LEGACY_F14, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf15:
			*key = kbinput_key(KB_KEY_LEGACY_F15, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf16:
			*key = kbinput_key(KB_KEY_LEGACY_F16, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf17:
			*key = kbinput_key(KB_KEY_LEGACY_F17, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf18:
			*key = kbinput_key(KB_KEY_LEGACY_F18, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf19:
			*key = kbinput_key(KB_KEY_LEGACY_F19, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf20:
			*key = kbinput_key(KB_KEY_LEGACY_F20, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf21:
			*key = kbinput_key(KB_KEY_LEGACY_F21, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf22:
			*key = kbinput_key(KB_KEY_LEGACY_F22, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf23:
			*key = kbinput_key(KB_KEY_LEGACY_F23, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf24:
			*key = kbinput_key(KB_KEY_LEGACY_F24, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf25:
			*key = kbinput_key(KB_KEY_LEGACY_F25, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf26:
			*key = kbinput_key(KB_KEY_LEGACY_F26, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf27:
			*key = kbinput_key(KB_KEY_LEGACY_F27, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf28:
			*key = kbinput_key(KB_KEY_LEGACY_F28, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf29:
			*key = kbinput_key(KB_KEY_LEGACY_F29, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf30:
			*key = kbinput_key(KB_KEY_LEGACY_F30, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf31:
			*key = kbinput_key(KB_KEY_LEGACY_F31, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf32:
			*key = kbinput_key(KB_KEY_LEGACY_F32, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf33:
			*key = kbinput_key(KB_KEY_LEGACY_F33, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf34:
			*key = kbinput_key(KB_KEY_LEGACY_F34, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf35:
			*key = kbinput_key(KB_KEY_LEGACY_F35, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf36:
			*key = kbinput_key(KB_KEY_LEGACY_F36, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf37:
			*key = kbinput_key(KB_KEY_LEGACY_F37, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf38:
			*key = kbinput_key(KB_KEY_LEGACY_F38, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf39:
			*key = kbinput_key(KB_KEY_LEGACY_F39, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf40:
			*key = kbinput_key(KB_KEY_LEGACY_F40, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf41:
			*key = kbinput_key(KB_KEY_LEGACY_F41, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf42:
			*key = kbinput_key(KB_KEY_LEGACY_F42, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf43:
			*key = kbinput_key(KB_KEY_LEGACY_F43, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf44:
			*key = kbinput_key(KB_KEY_LEGACY_F44, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf45:
			*key = kbinput_key(KB_KEY_LEGACY_F45, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf46:
			*key = kbinput_key(KB_KEY_LEGACY_F46, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf47:
			*key = kbinput_key(KB_KEY_LEGACY_F47, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf48:
			*key = kbinput_key(KB_KEY_LEGACY_F48, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf49:
			*key = kbinput_key(KB_KEY_LEGACY_F49, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf50:
			*key = kbinput_key(KB_KEY_LEGACY_F50, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf51:
			*key = kbinput_key(KB_KEY_LEGACY_F51, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf52:
			*key = kbinput_key(KB_KEY_LEGACY_F52, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf53:
			*key = kbinput_key(KB_KEY_LEGACY_F53, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf54:
			*key = kbinput_key(KB_KEY_LEGACY_F54, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf55:
			*key = kbinput_key(KB_KEY_LEGACY_F55, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf56:
			*key = kbinput_key(KB_KEY_LEGACY_F56, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf57:
			*key = kbinput_key(KB_KEY_LEGACY_F57, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf58:
			*key = kbinput_key(KB_KEY_LEGACY_F58, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf59:
			*key = kbinput_key(KB_KEY_LEGACY_F59, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf60:
			*key = kbinput_key(KB_KEY_LEGACY_F60, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf61:
			*key = kbinput_key(KB_KEY_LEGACY_F61, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf62:
			*key = kbinput_key(KB_KEY_LEGACY_F62, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kf63:
			*key = kbinput_key(KB_KEY_LEGACY_F63, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kcuu1:
			*key = kbinput_key(KB_KEY_LEGACY_UP, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kcud1:
			*key = kbinput_key(KB_KEY_LEGACY_DOWN, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kcub1:
			*key = kbinput_key(KB_KEY_LEGACY_LEFT, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kcuf1:
			*key = kbinput_key(KB_KEY_LEGACY_RIGHT, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kich1:
			*key = kbinput_key(KB_KEY_LEGACY_INSERT, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_khome:
			*key = kbinput_key(KB_KEY_LEGACY_HOME, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_knp:
			*key = kbinput_key(KB_KEY_LEGACY_PAGE_UP, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kdch1:
			*key = kbinput_key(KB_KEY_LEGACY_DELETE, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kend:
			*key = kbinput_key(KB_KEY_LEGACY_END, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kpp:
			*key = kbinput_key(KB_KEY_LEGACY_PAGE_DOWN, 0, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kri:
			*key = kbinput_key(KB_KEY_LEGACY_UP, KB_MOD_SHIFT, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kind:
			*key = kbinput_key(KB_KEY_LEGACY_DOWN, KB_MOD_SHIFT, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kLFT:
			*key = kbinput_key(KB_KEY_LEGACY_LEFT, KB_MOD_SHIFT, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kRIT:
			*key = kbinput_key(KB_KEY_LEGACY_RIGHT, KB_MOD_SHIFT, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kIC:
			*key = kbinput_key(KB_KEY_LEGACY_INSERT, KB_MOD_SHIFT, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kHOM:
			*key = kbinput_key(KB_KEY_LEGACY_HOME, KB_MOD_SHIFT, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kNXT:
			*key = kbinput_key(KB_KEY_LEGACY_PAGE_UP, KB_MOD_SHIFT, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kDC:
			*key = kbinput_key(KB_KEY_LEGACY_DELETE, KB_MOD_SHIFT, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kEND:
			*key = kbinput_key(KB_KEY_LEGACY_END, KB_MOD_SHIFT, KB_EVENT_PRESS, NULL);
			break ;
		case ti_kPRV:
			*key = kbinput_key(KB_KEY_LEGACY_PAGE_DOWN, KB_MOD_SHIFT, KB_EVENT_PRESS, NULL);
	}
	return 1;
}

static inline void	_kitty_parse_modifiers(const char *param, kbinput_key *key) {
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

static inline void	_kitty_parse_key_code(const char *param, kbinput_key *key, const u8 key_type) {
	u64	n;

	switch (key_type) {
		case 'u':
			key->code = strtoul(param, NULL, 10);
			break ;
		case '~':
			n = strtoul(param, NULL, 10);
			switch (n) {
				case 2:
					key->code = KB_KEY_INSERT;
					break ;
				case 3:
					key->code = KB_KEY_DELETE;
					break ;
				case 5:
					key->code = KB_KEY_PAGE_UP;
					break ;
				case 6:
					key->code = KB_KEY_PAGE_DOWN;
					break ;
				case 7:
					key->code = KB_KEY_HOME;
					break ;
				case 8:
					key->code = KB_KEY_END;
					break ;
				case 11:
					key->code = KB_KEY_F1;
					break ;
				case 12:
					key->code = KB_KEY_F2;
					break ;
				case 13:
					key->code = KB_KEY_F3;
					break ;
				case 14:
					key->code = KB_KEY_F4;
					break ;
				case 15:
					key->code = KB_KEY_F5;
					break ;
				case 17:
					key->code = KB_KEY_F6;
					break ;
				case 18:
					key->code = KB_KEY_F7;
					break ;
				case 19:
					key->code = KB_KEY_F8;
					break ;
				case 20:
					key->code = KB_KEY_F9;
					break ;
				case 21:
					key->code = KB_KEY_F10;
					break ;
				case 23:
					key->code = KB_KEY_F11;
					break ;
				case 24:
					key->code = KB_KEY_F12;
					break ;
				case 57427:
					key->code = KB_KEY_KP_BEGIN;
			}
			break ;
		case 'A':
			key->code = KB_KEY_UP;
			break ;
		case 'B':
			key->code = KB_KEY_DOWN;
			break ;
		case 'C':
			key->code = KB_KEY_RIGHT;
			break ;
		case 'D':
			key->code = KB_KEY_LEFT;
			break ;
		case 'E':
			key->code = KB_KEY_KP_BEGIN;
			break ;
		case 'F':
			key->code = KB_KEY_END;
			break ;
		case 'H':
			key->code = KB_KEY_HOME;
			break ;
		case 'P':
			key->code = KB_KEY_F1;
			break ;
		case 'Q':
			key->code = KB_KEY_F2;
			break ;
		case 'S':
			key->code = KB_KEY_F4;
	}
}

static inline void	_legacy_check_backspace(const u8 c, kbinput_key *key) {
	key->code = '\b';
	if (*ti_getstr(ti_kbs) != c)
		key->modifiers |= KB_MOD_CTRL;
}

static inline u32	_legacy_parse_codepoint(const char *buf, const size_t buf_len) {
	size_t	len;
	size_t	i;
	char	_buf[4];

	switch (*buf & 0x70) {
		case 0x70:
			len = 4;
			break ;
		case 0x60:
			len = 3;
			break ;
		case 0x40:
			len = 2;
			break ;
		default:
			len = 1;
	}
	for (i = 0; i < buf_len && i < len; i++)
		_buf[i] = buf[i];
	if (i < len && read(0, &_buf[i], len - i) != (ssize_t)(len - i))
		return UINT32_MAX;
	return utf8_decode(_buf);
}

static inline u8	_check_legacy_compatability(const kbinput_key *key) {
	if (key->event_type != KB_EVENT_PRESS)
		return 0;
	if (key->modifiers & ~(_LEGACY_MOD_MASK)) {
		if (!(key->modifiers & ~(KB_MOD_SHIFT | _LEGACY_MOD_MASK))) switch (key->code) {
			case '\t':
			case KB_KEY_LEGACY_UP:
			case KB_KEY_LEGACY_DOWN:
			case KB_KEY_LEGACY_LEFT:
			case KB_KEY_LEGACY_RIGHT:
			case KB_KEY_LEGACY_INSERT:
			case KB_KEY_LEGACY_HOME:
			case KB_KEY_LEGACY_PAGE_UP:
			case KB_KEY_LEGACY_DELETE:
			case KB_KEY_LEGACY_END:
			case KB_KEY_LEGACY_PAGE_DOWN:
				break ;
			default:
				return 0;
		} else
			return 0;
	}
	return 1;
}

static inline u8	_insert_key(vector vec, const kbinput_key *key) {
	kbinput_key	*_key;
	key_group	*group;
	size_t		size;
	size_t		i;
	void		*tmp;
	u8			match;

	i = _find_group(vec, key);
	tmp = get_group(vec, i);
	group = (tmp && in_bounds(tmp)) ? *(key_group **)tmp : NULL;
	if (!group || group->code != key->code) {
		group = _new_group(key->code);
		if (!group || !vector_insert(vec, i, group))
			return 0;
		_key = _new_key(key);
		return (_key) ? vector_push(group->keys, _key) : 0;
	}
	for (i = match = 0, size = vector_size(group->keys); i < size; i++) {
		_key = *get_key(group->keys, i);
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

static inline u8	_show_cursor(void) {

	ssize_t	rv;

	rv = 1;
	switch (cursor_mode.desired) {
		case ON:
		case SHOW:
			if (cursor_mode.current == OFF) {
				rv = write(1, TERM_SHOW_CURSOR, sizeof(TERM_SHOW_CURSOR));
				if (rv != -1)
					cursor_mode.current = ON;
			}
			break ;
		case OFF:
			if (cursor_mode.current == ON) {
				rv = write(1, TERM_HIDE_CURSOR, sizeof(TERM_HIDE_CURSOR));
				if (rv != -1)
					cursor_mode.current = OFF;
			}
	}
	return rv != -1;
}

static inline u8	_hide_cursor(void) {
	ssize_t	rv;

	rv = 1;
	switch (cursor_mode.desired) {
		case ON:
			if (cursor_mode.current == OFF) {
				rv = write(1, TERM_SHOW_CURSOR, sizeof(TERM_SHOW_CURSOR));
				if (rv != -1)
					cursor_mode.current = ON;
			}
			break ;
		case OFF:
		case SHOW:
			if (cursor_mode.current == ON) {
				rv = write(1, TERM_HIDE_CURSOR, sizeof(TERM_HIDE_CURSOR));
				if (rv != -1)
					cursor_mode.current = OFF;
			}
	}
	return rv != -1;
}
