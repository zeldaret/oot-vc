.include "macros.inc"

.section .data, "wa"  # 0x8016F7A0 - 0x801A0CC0

.balign 0x20

glabel lbl_8016F7A0
	.incbin "00000001.app", 0x16B8A0, 0x180

glabel lbl_8016F920
	.incbin "00000001.app", 0x16BA20, 0x190

glabel lbl_8016FAB0
	.incbin "00000001.app", 0x16BBB0, 0x3F0

glabel lbl_8016FEA0
	.incbin "00000001.app", 0x16BFA0, 0x164

glabel lbl_80170004
	.incbin "00000001.app", 0x16C104, 0x1C4

glabel lbl_801701C8
	.incbin "00000001.app", 0x16C2C8, 0x40

glabel lbl_80170208
	.incbin "00000001.app", 0x16C308, 0x10

glabel lbl_80170218
	.incbin "00000001.app", 0x16C318, 0x54

glabel lbl_8017026C
	.incbin "00000001.app", 0x16C36C, 0x64

glabel lbl_801702D0
	.incbin "00000001.app", 0x16C3D0, 0xD4

glabel lbl_801703A4
	.incbin "00000001.app", 0x16C4A4, 0xE4

glabel lbl_80170488
	.incbin "00000001.app", 0x16C588, 0x64

glabel lbl_801704EC
	.incbin "00000001.app", 0x16C5EC, 0x74

glabel lbl_80170560
	.incbin "00000001.app", 0x16C660, 0xC4

glabel lbl_80170624
	.incbin "00000001.app", 0x16C724, 0xD4

glabel lbl_801706F8
	.incbin "00000001.app", 0x16C7F8, 0x34

glabel lbl_8017072C
	.incbin "00000001.app", 0x16C82C, 0x54
