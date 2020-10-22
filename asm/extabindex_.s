.include "macros.inc"

.section extabindex_, "wa"  # 0x800069A0 - 0x80007020

.balign 0x20

glabel lbl_800069A0
	.incbin "00000001.app", 0x166160, 0x660

glabel lbl_80007000
	.incbin "00000001.app", 0x1667C0, 0x20
