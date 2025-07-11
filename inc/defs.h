// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<defs.h>>

#pragma once

#include <stddef.h>
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
#define KB_MOD_IGN_LCK	0x100U

#define KB_EVENT_PRESS		0x1U // canon value: 1
#define KB_EVENT_REPEAT		0x2U // canon value: 2
#define KB_EVENT_RELEASE	0x4U // canon value: 3

#define KB_KEY_TYPE_UNICODE	1
#define KB_KEY_TYPE_SPECIAL	2

#define KB_LISTENER_LIST_FULL	-1
#define KB_INVALID_LISTENER_ID	-2
#define KB_OPTION_NOT_SUPPORTED	-3

#define KB_INPUT_PROTOCOL_ERROR		0
#define KB_INPUT_PROTOCOL_KITTY		1
#define KB_INPUT_PROTOCOL_LEGACY	2

typedef i8	kbinput_listener_id;

typedef void *	(*kbinput_fn)(void *);

typedef struct __key {
	kbinput_fn	fn;
	struct {
		union {
			u32	unicode;
			u32	special;
		};
		u8	type;
	}	code;
	u32	text;
	u16	modifiers;
	u8	event_type;
}	kbinput_key;

#define kbinput_key(key_type, key_code, mods, event, function)	(\
		(kbinput_key){\
			.code.type = key_type,\
			.code.unicode = key_code,\
			.modifiers = mods,\
			.event_type = event,\
			.fn = function\
		})
