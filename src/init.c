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

#define _TERM_QUERY_ENHANCEMENTS	CSI "?u"
#define _TERM_QUERY_PRIMARY_ATTRS	CSI "c"

#define _TERM_ENABLE_ENHANCEMENTS	CSI "11u"
#define _TERM_ENABLE_ENHANCEMENTS_RAT	CSI "27u"

extern u8	input_protocol;

struct {
	struct termios	old;
	struct termios	new;
}	term_settings;

u8	kbinput_init(void) {
	ssize_t	bytes_read;
	size_t	i;
	char	buf[128];

	if (tcgetattr(0, &term_settings.old) == -1)
		return 0;
	term_settings.new = term_settings.old;
	term_settings.new.c_iflag &= ~(ICRNL | IXON);
	term_settings.new.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	if (tcsetattr(0, TCSANOW, &term_settings.new) == -1)
		return 0;
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
			input_protocol = INPUT_PROTOCOL_KITTY;
			write(1, _TERM_ENABLE_ENHANCEMENTS, sizeof(_TERM_ENABLE_ENHANCEMENTS));
			break ;
		case 'c':
			input_protocol = INPUT_PROTOCOL_LEGACY;
			break ;
		default:
			input_protocol = INPUT_PROTOCOL_ERROR;
			return 0;
	}
	return 1;
}

void	kbinput_cleanup(void) {
	tcsetattr(0, TCSANOW, &term_settings.old);
}
