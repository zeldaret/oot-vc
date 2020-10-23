.include "macros.inc"

.section .text, "ax"

glabel __init_hardware
/* 800042E0 000003E0  7C 00 00 A6 */	mfmsr r0
/* 800042E4 000003E4  60 00 20 00 */	ori r0, r0, 0x2000
/* 800042E8 000003E8  7C 00 01 24 */	mtmsr r0
/* 800042EC 000003EC  7F E8 02 A6 */	mflr r31
/* 800042F0 000003F0  48 08 64 35 */	bl __init_ps_regs
/* 800042F4 000003F4  48 08 52 15 */	bl __init_fp_regs
/* 800042F8 000003F8  48 08 75 DD */	bl __OSCacheInit
/* 800042FC 000003FC  7F E8 03 A6 */	mtlr r31
/* 80004300 00000400  4E 80 00 20 */	blr 