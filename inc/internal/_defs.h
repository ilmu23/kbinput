// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<_defs.h>>

#pragma once

#define CSI	"\x1b["

#define MAX_LISTENERS	10

#define TERM_MODE_RAW		0
#define TERM_MODE_NORMAL	1

#define TERM_SHOW_CURSOR	CSI "?25h"
#define TERM_HIDE_CURSOR	CSI "?25l"

#define TERM_ENABLE_ENHANCEMENTS	CSI "=27u"
#define TERM_DISABLE_ENHANCEMENTS	CSI "=0u"

#define LEGACY_SEQ_NO_MATCH	0xFFFFU

#define min(x, y)	((x < y) ? x : y)
#define max(x, y)	((x > y) ? x : y)
