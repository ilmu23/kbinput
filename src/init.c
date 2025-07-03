// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<init.c>>

#include <unistd.h>
#include <termios.h>

#include "internal/_kbinput.h"

#define _TERM_POP_FLAGS				CSI "<u"
#define _TERM_PUSH_FLAGS			CSI ">u"
#define _TERM_QUERY_ENHANCEMENTS	CSI "?u"
#define _TERM_QUERY_PRIMARY_ATTRS	CSI "c"

extern u8	input_protocol;

struct {
	struct termios	old;
	struct termios	new;
}	term_settings;

[[gnu::constructor]] void	kbinput_init(void) {
	ssize_t	bytes_read;
	size_t	i;
	char	buf[128];

	if (tcgetattr(0, &term_settings.old) == -1)
		return ;
	term_settings.new = term_settings.old;
	term_settings.new.c_iflag &= ~(ICRNL | IXON);
	term_settings.new.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	if (tcsetattr(0, TCSANOW, &term_settings.new) == -1)
		return ;
	write(1, _TERM_QUERY_ENHANCEMENTS, sizeof(_TERM_QUERY_ENHANCEMENTS));
	write(1, _TERM_QUERY_PRIMARY_ATTRS, sizeof(_TERM_QUERY_PRIMARY_ATTRS));
_kbinput_init_read_response:
	bytes_read = read(0, buf, sizeof(buf));
	for (i = 0; i < (size_t)bytes_read && buf[i]; i++)
		if (buf[i] == 'u' || buf[i] == 'c')
			break ;
	if (i == (size_t)bytes_read)
		goto _kbinput_init_read_response;
	switch (buf[i]) {
		case 'u':
			input_protocol = KB_INPUT_PROTOCOL_KITTY;
			write(1, _TERM_PUSH_FLAGS, sizeof(_TERM_PUSH_FLAGS));
			break ;
		case 'c':
			input_protocol = KB_INPUT_PROTOCOL_LEGACY;
			break ;
		default:
			input_protocol = KB_INPUT_PROTOCOL_ERROR;
			return ;
	}
	return ;
}

[[gnu::destructor]] void	kbinput_cleanup(void) {
	kbinput_listener_id	id;

	for (id = 0; id < MAX_LISTENERS; id++)
		kbinput_delete_listener(id);
	write(1, _TERM_POP_FLAGS, sizeof(_TERM_POP_FLAGS));
	tcsetattr(0, TCSANOW, &term_settings.old);
}
