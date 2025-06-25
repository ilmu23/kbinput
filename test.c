// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<test.c>>

#include <stdio.h>
#include <stdlib.h>

#include "kbinput.h"

void	*press_a([[gnu::unused]] void *arg) {
	printf("event: a pressed\n");
	return NULL;
}

void	*release_a([[gnu::unused]] void *arg) {
	printf("event: a released\n");
	return NULL;
}

void	*press_shift_a([[gnu::unused]] void *arg) {
	printf("event: A pressed\n");
	return NULL;
}

void	*release_shift_a([[gnu::unused]] void *arg) {
	printf("event: A released\n");
	return NULL;
}

void	*quit([[gnu::unused]] void *arg) {
	exit(0);
}

#define key(t, c, m, e, f)	((kbinput_key){.code.type = t, .code.unicode = c, .modifiers = m, .event_type = e, .fn = f})

i32	main(void) {
	kbinput_listener_id	listener;
	kbinput_fn			fn;

	listener = kbinput_new_listener();
	if (listener < 0)
		return 1;
	if (!kbinput_add_listener(listener, key(KB_KEY_TYPE_UNICODE, 'a', 0, KB_EVENT_PRESS, press_a)))
		return 1;
	if (!kbinput_add_listener(listener, key(KB_KEY_TYPE_UNICODE, 'a', 0, KB_EVENT_RELEASE, release_a)))
		return 1;
	if (!kbinput_add_listener(listener, key(KB_KEY_TYPE_UNICODE, 'a', KB_MOD_SHIFT, KB_EVENT_PRESS, press_shift_a)))
		return 1;
	if (!kbinput_add_listener(listener, key(KB_KEY_TYPE_UNICODE, 'a', KB_MOD_SHIFT, KB_EVENT_RELEASE, release_shift_a)))
		return 1;
	if (!kbinput_add_listener(listener, key(KB_KEY_TYPE_UNICODE, 'c', KB_MOD_CTRL, KB_EVENT_PRESS, quit)))
		return 1;
	fn = kbinput_listen(listener);
	if (fn)
		fn(NULL);
	fn = kbinput_listen(listener);
	if (fn)
		fn(NULL);
	fn = kbinput_listen(listener);
	if (fn)
		fn(NULL);
	fn = kbinput_listen(listener);
	if (fn)
		fn(NULL);
	fn = kbinput_listen(listener);
	if (fn)
		fn(NULL);
	fn = kbinput_listen(listener);
	if (fn)
		fn(NULL);
	return 0;
}
