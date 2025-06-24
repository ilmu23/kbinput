// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<listener.c>>

#include "internal/_kbinput.h"
#include "internal/utils/vector.h"

typedef struct __key_group {
	vector	keys;
	u32		code;
}	key_group;

#define get_group(v, i)	((key_group *)vector_get(v, i))
#define get_key(v, i)	((kbinput_key *)vector_get(v, i))
#define in_bounds(p)	(p != (void *)VECTOR_INDEX_OUT_OF_BOUNDS)

static inline kbinput_key	*_listen_kitty(kbinput_listener_id listener);
static inline kbinput_key	*_listen_legacy(kbinput_listener_id listener);
static inline kbinput_key	*_new_key(const kbinput_key *key);
static inline key_group		*_new_group(const u32 code);
static inline size_t		_find_group(const vector vec, const kbinput_key *key);
static inline u8			_insert_key(vector vec, const kbinput_key *key);

static void	_free_key_group(void *group);

static struct {
	const kbinput_listener_id	id;
	vector						uc_keys;
	vector						sp_keys;
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

const kbinput_key	*kbinput_listen(const kbinput_listener_id listener) {
	switch (input_protocol) {
		case INPUT_PROTOCOL_KITTY:
			return _listen_kitty(listener);
		case INPUT_PROTOCOL_LEGACY:
			return _listen_legacy(listener);
	}
	return NULL;
}

void	kbinput_delete_listener(const kbinput_listener_id listener) {
	if (listener >= 0 && listener < MAX_LISTENERS) {
		vector_delete(listeners[listener].uc_keys);
		vector_delete(listeners[listener].sp_keys);
		listeners[listener].in_use = 0;
	}
}

u8	kbinput_add_listener(const kbinput_listener_id listener, const kbinput_key key) {
	vector	vec;

	if (listener < 0 || listener >= MAX_LISTENERS)
		return 0;
	vec = (key.code.type == KB_KEY_TYPE_UNICODE) ? listeners[listener].uc_keys : listeners[listener].sp_keys;
	return (_insert_key(vec, &key));
}

static inline kbinput_key	*_listen_kitty(kbinput_listener_id listener) {
	return (void *)(uintptr_t)listener;
}

static inline kbinput_key	*_listen_legacy(kbinput_listener_id listener) {
	return (void *)(uintptr_t)listener;
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
	size_t		search_width;
	size_t		i;

	i = vector_size(vec) / 2;
	search_width = i;
	for (group = get_group(vec, i); search_width && key->code.unicode != group->code; group = get_group(vec, i)) {
		search_width /= 2;
		if (key->code.unicode < group->code)
			i -= search_width;
		else
			i += search_width;
	}
	return i;
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
	for (i = match = 0, _key = get_key(group->keys, i); !match && in_bounds(_key); _key = get_key(group->keys, ++i)) {
		if (key->modifiers == _key->modifiers && key->event_type == _key->event_type)
			match = 1;
		else if (key->modifiers > _key->modifiers)
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
	if (group)
		vector_delete(((key_group *)group)->keys);
}
