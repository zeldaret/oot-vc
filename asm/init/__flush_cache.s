.include "macros.inc"

.section .text, "ax"

glabel __flush_cache
/* 80004304 00000404  3C A0 FF FF */	lis r5, 0xFFFFFFF1@h
/* 80004308 00000408  60 A5 FF F1 */	ori r5, r5, 0xFFFFFFF1@l
/* 8000430C 0000040C  7C A5 18 38 */	and r5, r5, r3
/* 80004310 00000410  7C 65 18 50 */	subf r3, r5, r3
/* 80004314 00000414  7C 84 1A 14 */	add r4, r4, r3
lbl_80004318:
/* 80004318 00000418  7C 00 28 6C */	dcbst 0, r5
/* 8000431C 0000041C  7C 00 04 AC */	sync 0
/* 80004320 00000420  7C 00 2F AC */	icbi 0, r5
/* 80004324 00000424  30 A5 00 08 */	addic r5, r5, 8
/* 80004328 00000428  34 84 FF F8 */	addic. r4, r4, -8
/* 8000432C 0000042C  40 80 FF EC */	bge lbl_80004318
/* 80004330 00000430  4C 00 01 2C */	isync 
/* 80004334 00000434  4E 80 00 20 */	blr 
