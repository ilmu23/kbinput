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

void	*press(void *arg) {
	const kbinput_key	*key;
	u8					mod;

	key = arg;
	fprintf(stdout, "event: ");
	for (mod = KB_MOD_SHIFT; mod < KB_MOD_HYPER; mod *= 2) {
		switch (key->modifiers & mod) {
			case KB_MOD_SHIFT:
				fprintf(stdout, "SHIFT + ");
				break ;
			case KB_MOD_ALT:
				fprintf(stdout, "ALT + ");
				break ;
			case KB_MOD_CTRL:
				fprintf(stdout, "CTRL + ");
				break ;
			case KB_MOD_SUPER:
				fprintf(stdout, "SUPER + ");
		}
	}
	fprintf(stdout, "%c pressed", toupper(key->code.unicode));
	if (key->text)
		fprintf(stdout, ", text: %u", key->text);
	fputc('\n', stdout);
	return NULL;
}

void	*release(void *arg) {
	const kbinput_key	*key;
	u8					mod;

	key = arg;
	fprintf(stdout, "event: ");
	for (mod = KB_MOD_SHIFT; mod < KB_MOD_HYPER; mod *= 2) {
		switch (key->modifiers & mod) {
			case KB_MOD_SHIFT:
				fprintf(stdout, "SHIFT + ");
				break ;
			case KB_MOD_ALT:
				fprintf(stdout, "ALT + ");
				break ;
			case KB_MOD_CTRL:
				fprintf(stdout, "CTRL + ");
				break ;
			case KB_MOD_SUPER:
				fprintf(stdout, "SUPER + ");
		}
	}
	fprintf(stdout, "%c released\n", toupper(key->code.unicode));
	return NULL;
}

void	*quit([[gnu::unused]] void *arg) {
	exit(0);
}

#define key(t, c, m, e, f)	((kbinput_key){.code.type = t, .code.unicode = c, .modifiers = KB_MOD_IGN_LCK | m, .event_type = e, .fn = f})

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
	size_t	i;
	size_t	j;
	char	c;

	write(1, "\x1b[=0u", 5);
	for (c = 'a'; c <= 'z'; c++) {
		for (i = 0; i < EVENT_COUNT; i++) {
			for (j = 0; j < MOD_COUNT; j++) {
				assert(kbinput_add_listener(id, key(KB_KEY_TYPE_UNICODE, c, mods[j], events[i].et, events[i].fn)) > 0);
			}
		}
	}
	for (c = '0'; c <= '9'; c++) {
		for (i = 0; i < EVENT_COUNT; i++) {
			for (j = 0; j < MOD_COUNT; j++) {
				assert(kbinput_add_listener(id, key(KB_KEY_TYPE_UNICODE, c, mods[j], events[i].et, events[i].fn)) > 0);
			}
		}
	}
	assert(kbinput_add_listener(id, key(KB_KEY_TYPE_UNICODE, 'c', KB_MOD_CTRL, KB_EVENT_PRESS, quit)) > 0);
	write(1, "\x1b[=27u", 6);
}

i32	main(void) {
	kbinput_listener_id	listener;
	const kbinput_key	*key;

	listener = kbinput_new_listener();
	if (listener < 0)
		return 1;
	_init_listeners(listener);
	while (1) {
		key = kbinput_listen(listener);
		if (!key)
			return 1;
		key->fn((void *)key);
	}
}
