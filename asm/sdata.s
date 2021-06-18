.include "macros.inc"

.section .sdata, "wa"  # 0x8025C6C0 - 0x8025CFE0

.balign 0x20

glabel lbl_8025C6C0
	.incbin "00000001.app", 0x19CDC0, 0x8

glabel lbl_8025C6C8
	.incbin "00000001.app", 0x19CDC8, 0x4

glabel lbl_8025C6CC
	.incbin "00000001.app", 0x19CDCC, 0x4

glabel lbl_8025C6D0
	.incbin "00000001.app", 0x19CDD0, 0x4

glabel lbl_8025C6D4
	.incbin "00000001.app", 0x19CDD4, 0xC

glabel lbl_8025C6E0
	.incbin "00000001.app", 0x19CDE0, 0x8

glabel lbl_8025C6E8
	.incbin "00000001.app", 0x19CDE8, 0x4

glabel lbl_8025C6EC
	.incbin "00000001.app", 0x19CDEC, 0x4

glabel lbl_8025C6F0
	.incbin "00000001.app", 0x19CDF0, 0x4

glabel lbl_8025C6F4
	.incbin "00000001.app", 0x19CDF4, 0x4

glabel lbl_8025C6F8
	.incbin "00000001.app", 0x19CDF8, 0x4

glabel lbl_8025C6FC
	.incbin "00000001.app", 0x19CDFC, 0x4

glabel lbl_8025C700
	.incbin "00000001.app", 0x19CE00, 0x4

glabel lbl_8025C704
	.incbin "00000001.app", 0x19CE04, 0x8

glabel lbl_8025C70C
	.incbin "00000001.app", 0x19CE0C, 0x44

glabel cpu_class_name
	.incbin "00000001.app", 0x19CE50, 0x8

glabel lbl_8025C758
	.incbin "00000001.app", 0x19CE58, 0x8
