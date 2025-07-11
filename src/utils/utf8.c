// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<utf8.c>>

#include "internal/_kbinput.h"

#define decode_start_2(c)	((c & 0x1FU) << 6)
#define decode_start_3(c)	((c & 0x0FU) << 12)
#define decode_start_4(c)	((c & 0x07U) << 18)
#define decode_cont(c, r)	((c & 0x3FU) << (6 * r))

u32	utf8_decode(const char *c) {
	u32	out;

	if (!(*c & 0x80U))
		out = *c;
	else switch (*c & 0x70U) {
		case 0x70U:
			out = decode_start_4(c[0]) | decode_cont(c[1], 2) | decode_cont(c[2], 1) | decode_cont(c[3], 0);
			break ;
		case 0x60U:
			out = decode_start_3(c[0]) | decode_cont(c[1], 1) | decode_cont(c[2], 0);
			break ;
		case 0x40U:
			out = decode_start_2(c[0]) | decode_cont(c[1], 0);
	}
	return out;
}
