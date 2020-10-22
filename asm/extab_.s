.include "macros.inc"

.section extab_, "wa"  # 0x800064E0 - 0x800069A0

.balign 0x20

glabel lbl_800064E0
	.incbin "00000001.app", 0x165CA0, 0x4C0
