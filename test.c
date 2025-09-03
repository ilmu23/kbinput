// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<test.c>>

#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "kbinput.h"

static inline void	_print_mods(const kbinput_key *key) {
	if (key->modifiers & KB_MOD_SHIFT)
		fprintf(stdout, "SHIFT + ");
	if (key->modifiers & KB_MOD_ALT)
		fprintf(stdout, "ALT + ");
	if (key->modifiers & KB_MOD_CTRL)
		fprintf(stdout, "CTRL + ");
	if (key->modifiers & KB_MOD_SUPER)
		fprintf(stdout, "SUPER + ");
}

void	*press(void *arg) {
	const kbinput_key	*key;

	key = arg;
	fprintf(stdout, "event: ");
	_print_mods(key);
	switch (key->code) {
		case KB_KEY_UP:
		case KB_KEY_LEGACY_UP:
			fprintf(stdout, "<UP> pressed");
			break ;
		case KB_KEY_DOWN:
		case KB_KEY_LEGACY_DOWN:
			fprintf(stdout, "<DOWN> pressed");
			break ;
		case KB_KEY_LEFT:
		case KB_KEY_LEGACY_LEFT:
			fprintf(stdout, "<LEFT> pressed");
			break ;
		case KB_KEY_RIGHT:
		case KB_KEY_LEGACY_RIGHT:
			fprintf(stdout, "<RIGHT> pressed");
			break ;
		case KB_KEY_INSERT:
		case KB_KEY_LEGACY_INSERT:
			fprintf(stdout, "<INS> pressed");
			break ;
		case KB_KEY_HOME:
		case KB_KEY_LEGACY_HOME:
			fprintf(stdout, "<HME> pressed");
			break ;
		case KB_KEY_PAGE_UP:
		case KB_KEY_LEGACY_PAGE_UP:
			fprintf(stdout, "<PGU> pressed");
			break ;
		case KB_KEY_DELETE:
		case KB_KEY_LEGACY_DELETE:
			fprintf(stdout, "<DEL> pressed");
			break ;
		case KB_KEY_END:
		case KB_KEY_LEGACY_END:
			fprintf(stdout, "<END> pressed");
			break ;
		case KB_KEY_PAGE_DOWN:
		case KB_KEY_LEGACY_PAGE_DOWN:
			fprintf(stdout, "<PGD> pressed");
			break ;
		default:
			if (isprint(key->code))
				fprintf(stdout, "%c pressed", toupper(key->code));
	}
	if (key->text)
		fprintf(stdout, ", text: %u", key->text);
	fputc('\n', stdout);
	return NULL;
}

void	*release(void *arg) {
	const kbinput_key	*key;

	key = arg;
	fprintf(stdout, "event: ");
	_print_mods(key);
	fprintf(stdout, "%c released\n", toupper(key->code));
	return NULL;
}

void	*quit([[gnu::unused]] void *arg) {
	kbinput_cleanup();
	exit(0);
}

#define EVENT_COUNT	2
#define MOD_COUNT	15

static const struct {
	kbinput_fn	fn;
	u8			et;
}	events[EVENT_COUNT] = {
	{.fn = press, .et = KB_EVENT_PRESS},
	{.fn = release, .et = KB_EVENT_RELEASE}
};

static u16	mods[MOD_COUNT] = {
	0,
	KB_MOD_SHIFT,
	KB_MOD_ALT,
	KB_MOD_CTRL,
	KB_MOD_SUPER,
	KB_MOD_SHIFT | KB_MOD_ALT,
	KB_MOD_SHIFT | KB_MOD_CTRL,
	KB_MOD_SHIFT | KB_MOD_SUPER,
	KB_MOD_ALT | KB_MOD_CTRL,
	KB_MOD_ALT | KB_MOD_SUPER,
	KB_MOD_CTRL | KB_MOD_SUPER,
	KB_MOD_SHIFT | KB_MOD_ALT | KB_MOD_CTRL,
	KB_MOD_SHIFT | KB_MOD_ALT | KB_MOD_SUPER,
	KB_MOD_ALT | KB_MOD_CTRL | KB_MOD_SUPER,
	KB_MOD_SHIFT | KB_MOD_ALT | KB_MOD_CTRL | KB_MOD_SUPER
};

static inline void	_init_listeners(const kbinput_listener_id id) {
#ifdef __DEBUG_IGNORE_ADD_LISTENER_FAIL
	static u8	ign = 1;
#else
	static u8	ign = 0;
#endif
	size_t	i;
	size_t	j;
	char	c;

	for (c = 'a'; c <= 'z'; c++) {
		for (i = 0; i < EVENT_COUNT; i++) {
			for (j = 0; j < MOD_COUNT; j++) {
				assert(kbinput_add_listener(id, kbinput_key(c, mods[j], events[i].et, events[i].fn)) > 0 || ign);
			}
		}
	}
	for (c = '0'; c <= '9'; c++) {
		for (i = 0; i < EVENT_COUNT; i++) {
			for (j = 0; j < MOD_COUNT; j++) {
				assert(kbinput_add_listener(id, kbinput_key(c, mods[j], events[i].et, events[i].fn)) > 0 || ign);
			}
		}
	}
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_UP, 0, KB_EVENT_PRESS, press)) > 0  || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_DOWN, 0, KB_EVENT_PRESS, press)) > 0  || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_LEFT, 0, KB_EVENT_PRESS, press)) > 0  || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_RIGHT, 0, KB_EVENT_PRESS, press)) > 0  || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_INSERT, 0, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_HOME, 0, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_PAGE_UP, 0, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_DELETE, 0, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_END, 0, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_PAGE_DOWN, 0, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_UP, KB_MOD_SHIFT, KB_EVENT_PRESS, press)) > 0  || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_DOWN, KB_MOD_SHIFT, KB_EVENT_PRESS, press)) > 0  || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_LEFT, KB_MOD_SHIFT, KB_EVENT_PRESS, press)) > 0  || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_RIGHT, KB_MOD_SHIFT, KB_EVENT_PRESS, press)) > 0  || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_INSERT, KB_MOD_SHIFT, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_HOME, KB_MOD_SHIFT, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_PAGE_UP, KB_MOD_SHIFT, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_DELETE, KB_MOD_SHIFT, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_END, KB_MOD_SHIFT, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key(KB_KEY_PAGE_DOWN, KB_MOD_SHIFT, KB_EVENT_PRESS, press)) > 0 || ign);
	assert(kbinput_add_listener(id, kbinput_key('c', KB_MOD_CTRL, KB_EVENT_PRESS, quit)) > 0 || ign);
}

i32	main(void) {
	kbinput_listener_id	listener;
	const kbinput_key	*key;

	kbinput_init();
	write(1, "\x1b[?1h", 5);
	listener = kbinput_new_listener();
	if (listener < 0)
		return 1;
	_init_listeners(listener);
	while (1) {
		key = kbinput_listen(listener);
		if (!key) {
			write(1, "\x1b[?1l", 5);
			kbinput_cleanup();
			return 1;
		}
		key->fn((void *)key);
	}
	write(1, "\x1b[?1l", 5);
	kbinput_cleanup();
}
