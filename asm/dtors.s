.include "macros.inc"

.section .dtors, "wa"  # 0x8016A740 - 0x8016A760

.balign 0x20

glabel _dtors
	.4byte func_801525F4
	.4byte func_80153654
