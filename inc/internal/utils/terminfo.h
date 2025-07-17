// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<terminfo.h>>

#pragma once

#include "defs.h"

#include "vector.h"
#include "terminfo_caps.h"

#define TI_NOT_BOOL	-1
#define TI_NOT_NUM	-2
#define TI_NOT_STR	((const char *)-1)

#define TI_ABS_NUM		0xFFFFFFFF
#define TI_ABS_STR		NULL

typedef u8	boolean_cap;

typedef struct __numeric_cap {
	u32	value;
	u8	name;
}	numeric_cap;

#define numeric_cap(val, nme)	((numeric_cap){.value = val, .name = nme})

typedef struct __string_cap {
	const char	*value;
	u16			name;
}	string_cap;

#define string_cap(val, nme)	((string_cap){.value = val, .name = nme})

typedef struct __header {
	u16	bit_count;
	u16	name_length;
	struct {
		u16	boolean;
		u16	numeric;
		u16	string;
	}	capability_counts;
	u16	next_free;
}	header;

typedef struct __entry {
	union {
		const u32	*u32;
		const u16	*u16;
	}			numbers;
	const u16	*offsets;
	const char	*term_names;
	const char	*string_table;
	const u8	*booleans;
	header		header;
}	entry;

const char	*ti_getstr(const u16 name);

void	ti_unload(void);

i32	ti_getflag(const u8 name);
i32	ti_getnum(const u8 name);

u8	ti_load(const char *term);
