// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<kbinput.h>>

#pragma once

#include "defs.h"
#include "keys.h"

kbinput_listener_id	kbinput_new_listener(void);

const kbinput_key	*kbinput_listen(const kbinput_listener_id listener);

void	kbinput_delete_listener(const kbinput_listener_id listener);
void	kbinput_cleanup(void);
void	kbinput_init(void);

u8	kbinput_add_listener(const kbinput_listener_id listener, const kbinput_key key);
u8	kbinput_get_input_protocol(void);
