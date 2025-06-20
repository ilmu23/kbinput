// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<defs.h>>

#pragma once

#include <stdint.h>

typedef	int8_t		i8;
typedef	int16_t		i16;
typedef	int32_t		i32;
typedef	int64_t		i64;

typedef	uint8_t		u8;
typedef	uint16_t	u16;
typedef	uint32_t	u32;
typedef	uint64_t	u64;

#define KB_MOD_SHIFT	0x01U
#define KB_MOD_ALT		0x02U
#define KB_MOD_CTRL		0x04U
#define KB_MOD_SUPER	0x08U
#define KB_MOD_HYPER	0x10U
#define KB_MOD_META		0x20U
#define KB_MOD_CAPS_LCK	0x40U
#define KB_MOD_NUM_LCK	0x80U

#define KB_EVENT_PRESS		1
#define KB_EVENT_REPEAT		2
#define KB_EVENT_RELEASE	3

#define KB_KEY_TYPE_UNICODE	1
#define KB_KEY_TYPE_SPECIAL	2

typedef struct __key {
	struct {
		union {
			u32	unicode;
			u32	special;
		};
		u8	type;
	}	code;
	u8	modifiers;
	u8	event_type;
}	kbinput_key;
