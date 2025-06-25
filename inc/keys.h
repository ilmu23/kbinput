// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<keys.h>>

#pragma once

// sw.kovidgoyal.net/kitty/keyboard-protocol/#key-codes

#define KB_KEY_ESCAPE				27
#define KB_KEY_ENTER				13
#define KB_KEY_TAB					9
#define KB_KEY_BACKSPACE			127
#define KB_KEY_INSERT				32306	/* 2~ */
#define KB_KEY_DELETE				32307	/* 3~ */
#define KB_KEY_LEFT					68		/* D */
#define KB_KEY_RIGHT				67		/* C */
#define KB_KEY_UP					65		/* A */
#define KB_KEY_DOWN					66		/* B */
#define KB_KEY_PAGE_UP				32309	/* 5~ */
#define KB_KEY_PADE_DOWN			32310	/* 6~ */
#define KB_KEY_HOME					72		/* H */
#define KB_KEY_HOME_2				32311	/* 7~ */
#define KB_KEY_END					70		/* F */
#define KB_KEY_END_2				32312	/* 8~ */
#define KB_KEY_CAPS_LOCK			57358
#define KB_KEY_SCROLL_LOCK			57359
#define KB_KEY_NUM_LOCK				57360
#define KB_KEY_PRINT_SCREEN			57361
#define KB_KEY_PAUSE				57362
#define KB_KEY_MENU					57363
#define KB_KEY_F1					80		/* P */
#define KB_KEY_F1_2					8270129	/* 11~ */
#define KB_KEY_F2					81		/* Q */
#define KB_KEY_F2_2					8270305	/* 12~ */
#define KB_KEY_F3					8270641 /* 13~ */
#define KB_KEY_F4					83		/* S */
#define KB_KEY_F4_2					8270897 /* 14~ */
#define KB_KEY_F5					8271153 /* 15~ */
#define KB_KEY_F6					8271665 /* 17~ */
#define KB_KEY_F7					8271921 /* 18~ */
#define KB_KEY_F8					8272177 /* 19~ */
#define KB_KEY_F9					8269874 /* 20~ */
#define KB_KEY_F10					8270130 /* 21~ */			
#define KB_KEY_F11					8270642 /* 23~ */
#define KB_KEY_F12					8270898 /* 24~ */
#define KB_KEY_F13					57376
#define KB_KEY_F14 					57377
#define KB_KEY_F15 					57378
#define KB_KEY_F16 					57379
#define KB_KEY_F17 					57380
#define KB_KEY_F18 					57381
#define KB_KEY_F19 					57382
#define KB_KEY_F20 					57383
#define KB_KEY_F21 					57384
#define KB_KEY_F22 					57385
#define KB_KEY_F23 					57386
#define KB_KEY_F24 					57387
#define KB_KEY_F25 					57388
#define KB_KEY_F26 					57389
#define KB_KEY_F27 					57390
#define KB_KEY_F28 					57391
#define KB_KEY_F29 					57392
#define KB_KEY_F30 					57393
#define KB_KEY_F31 					57394
#define KB_KEY_F32 					57395
#define KB_KEY_F33 					57396
#define KB_KEY_F34 					57397
#define KB_KEY_F35 					57398
#define KB_KEY_KP_0					57399
#define KB_KEY_KP_1					57400
#define KB_KEY_KP_2					57401
#define KB_KEY_KP_3					57402
#define KB_KEY_KP_4					57403
#define KB_KEY_KP_5					57404
#define KB_KEY_KP_6					57405
#define KB_KEY_KP_7					57406
#define KB_KEY_KP_8					57407
#define KB_KEY_KP_9					57408
#define KB_KEY_KP_DECIMAL			57409
#define KB_KEY_KP_DIVIDE			57410
#define KB_KEY_KP_MULTIPLY			57411
#define KB_KEY_KP_SUBTRACT			57412
#define KB_KEY_KP_ADD				57413
#define KB_KEY_KP_ENTER				57414
#define KB_KEY_KP_EQUAL				57415
#define KB_KEY_KP_SEPARATOR			57416
#define KB_KEY_KP_LEFT				57417
#define KB_KEY_KP_RIGHT				57418
#define KB_KEY_KP_UP				57419
#define KB_KEY_KP_DOWN				57420
#define KB_KEY_KP_PAGE_UP			57421
#define KB_KEY_KP_PAGE_DOWN			57422
#define KB_KEY_KP_HOME				57423
#define KB_KEY_KP_END				57424
#define KB_KEY_KP_INSERT			57425
#define KB_KEY_KP_DELETE			57426
#define KB_KEY_KP_BEGIN				69		/* E */
#define KB_KEY_KP_BEGIN_2			138775530583861	/* 57427~ */
#define KB_KEY_MEDIA_PLAY			57428
#define KB_KEY_MEDIA_PAUSE			57429
#define KB_KEY_MEDIA_PLAY_PAUSE		57430
#define KB_KEY_MEDIA_REVERSE		57431
#define KB_KEY_MEDIA_STOP			57432
#define KB_KEY_MEDIA_FAST_FORWARD	57433
#define KB_KEY_MEDIA_REWIND			57434
#define KB_KEY_MEDIA_TRACK_NEXT		57435
#define KB_KEY_MEDIA_TRACK_PREVIOUS	57436
#define KB_KEY_MEDIA_RECORD			57437
#define KB_KEY_LOWER_VOLUME			57438
#define KB_KEY_RAISE_VOLUME			57439
#define KB_KEY_MUTE_VOLUME			57440
#define KB_KEY_LEFT_SHIFT			57441
#define KB_KEY_LEFT_CONTROL			57442
#define KB_KEY_LEFT_ALT				57443
#define KB_KEY_LEFT_SUPER			57444
#define KB_KEY_LEFT_HYPER			57445
#define KB_KEY_LEFT_META			57446
#define KB_KEY_RIGHT_SHIFT			57447
#define KB_KEY_RIGHT_CONTROL		57448
#define KB_KEY_RIGHT_ALT			57449
#define KB_KEY_RIGHT_SUPER			57450
#define KB_KEY_RIGHT_HYPER			57451
#define KB_KEY_RIGHT_META			57452
#define KB_KEY_ISO_LEVEL3_SHIFT		57453
#define KB_KEY_ISO_LEVEL5_SHIFT		57454
