// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<init.c>>

#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "internal/_kbinput.h"

#define _TERM_POP_FLAGS			CSI "<u"
#define _TERM_PUSH_FLAGS		CSI ">u"
#define _TERM_QUERY_PROTOCOL	CSI "?u" CSI "c"

extern u8	input_protocol;

extern struct {
	kbinput_cursor_mode	desired;
	kbinput_cursor_mode	current;
}	cursor_mode;

struct {
	struct termios	old;
	struct termios	new;
}	term_settings;

struct {
	u8	init_done;
	u8	clean_done;
}	status = {
	.init_done = 0,
	.clean_done = 0
};

void	kbinput_init(void) {
	const char	*smkx;
	ssize_t		bytes_read;
	size_t		i;
	char		buf[128];

	if (status.init_done)
		return ;
	if (tcgetattr(0, &term_settings.old) == -1)
		return ;
	term_settings.new = term_settings.old;
	term_settings.new.c_iflag &= ~(ICRNL | IXON);
	term_settings.new.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
#ifdef __DEBUG_FORCE_LEGACY_MODE
	input_protocol = KB_INPUT_PROTOCOL_LEGACY;
	if (ti_load(getenv("TERM"))) {
		smkx = ti_getstr(ti_smkx);
		if (smkx != TI_ABS_STR && write(1, smkx, strlen(smkx)) != (ssize_t)strlen(smkx))
			input_protocol = KB_INPUT_PROTOCOL_ERROR;
		else
			setup_legacy_key_seqs();
	}
	status.init_done = 1;
	return ;
#endif
	switch_term_mode(TERM_MODE_RAW);
	if (write(1, _TERM_QUERY_PROTOCOL, sizeof(_TERM_QUERY_PROTOCOL)) == -1) {
		input_protocol = KB_INPUT_PROTOCOL_ERROR;
		return ;
	}
_kbinput_init_read_response:
	bytes_read = read(0, buf, sizeof(buf));
	for (i = 0; i < (size_t)bytes_read && buf[i]; i++)
		if (buf[i] == 'u' || buf[i] == 'c')
			break ;
	if (i == (size_t)bytes_read)
		goto _kbinput_init_read_response;
	switch (buf[i]) {
		case 'u':
			if (write(1, _TERM_PUSH_FLAGS TERM_ENABLE_ENHANCEMENTS, sizeof(_TERM_PUSH_FLAGS TERM_ENABLE_ENHANCEMENTS)) == -1)
				input_protocol = KB_INPUT_PROTOCOL_ERROR;
			else
				input_protocol = KB_INPUT_PROTOCOL_KITTY;
			break ;
		case 'c':
			input_protocol = KB_INPUT_PROTOCOL_LEGACY;
			if (ti_load(getenv("TERM"))) {
				smkx = ti_getstr(ti_smkx);
				if (smkx != TI_ABS_STR && write(1, smkx, strlen(smkx)) != (ssize_t)strlen(smkx))
					input_protocol = KB_INPUT_PROTOCOL_ERROR;
				else
					setup_legacy_key_seqs();
			}
			break ;
		default:
	}
	switch_term_mode(TERM_MODE_NORMAL);
	status.init_done = 1;
	return ;
}

void	kbinput_cleanup(void) {
	kbinput_listener_id	id;
	const char			*rmkx;

	if (status.clean_done)
		return ;
	for (id = 0; id < MAX_LISTENERS; id++)
		kbinput_delete_listener(id);
	if (input_protocol == KB_INPUT_PROTOCOL_LEGACY) {
		rmkx = ti_getstr(ti_rmkx);
		if (rmkx != TI_ABS_STR)
			write(1, rmkx, strlen(rmkx));
	} else
		write(1, _TERM_POP_FLAGS, sizeof(_TERM_POP_FLAGS));

	switch_term_mode(TERM_MODE_NORMAL);
	if (cursor_mode.current == OFF && write(1, TERM_SHOW_CURSOR, sizeof(TERM_SHOW_CURSOR)) == -1) { ; }
	status.clean_done = 1;
}

i32	switch_term_mode(const u8 term_mode) {
	return tcsetattr(0, TCSAFLUSH, (term_mode == TERM_MODE_RAW) ? &term_settings.new : &term_settings.old);
}
