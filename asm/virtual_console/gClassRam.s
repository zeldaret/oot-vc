.include "macros.inc"

.section .text, "ax"

glabel func_80041844
/* 80041844 0003CE04  38 60 00 00 */	li r3, 0
/* 80041848 0003CE08  4E 80 00 20 */	blr 

glabel func_8004184C
/* 8004184C 0003CE0C  38 60 00 00 */	li r3, 0
/* 80041850 0003CE10  4E 80 00 20 */	blr 

glabel func_80041854
/* 80041854 0003CE14  54 80 06 BE */	clrlwi r0, r4, 0x1a
/* 80041858 0003CE18  28 00 00 24 */	cmplwi r0, 0x24
/* 8004185C 0003CE1C  41 81 00 94 */	bgt lbl_800418F0
/* 80041860 0003CE20  3C 80 80 17 */	lis r4, lbl_80172070@ha
/* 80041864 0003CE24  54 00 10 3A */	slwi r0, r0, 2
/* 80041868 0003CE28  38 84 20 70 */	addi r4, r4, lbl_80172070@l
/* 8004186C 0003CE2C  7C 84 00 2E */	lwzx r4, r4, r0
/* 80041870 0003CE30  7C 89 03 A6 */	mtctr r4
/* 80041874 0003CE34  4E 80 04 20 */	bctr 
/* 80041878 0003CE38  80 05 00 00 */	lwz r0, 0(r5)
/* 8004187C 0003CE3C  90 03 00 0C */	stw r0, 0xc(r3)
/* 80041880 0003CE40  48 00 00 78 */	b lbl_800418F8
/* 80041884 0003CE44  80 05 00 00 */	lwz r0, 0(r5)
/* 80041888 0003CE48  90 03 00 10 */	stw r0, 0x10(r3)
/* 8004188C 0003CE4C  48 00 00 6C */	b lbl_800418F8
/* 80041890 0003CE50  80 05 00 00 */	lwz r0, 0(r5)
/* 80041894 0003CE54  90 03 00 14 */	stw r0, 0x14(r3)
/* 80041898 0003CE58  48 00 00 60 */	b lbl_800418F8
/* 8004189C 0003CE5C  80 05 00 00 */	lwz r0, 0(r5)
/* 800418A0 0003CE60  90 03 00 18 */	stw r0, 0x18(r3)
/* 800418A4 0003CE64  48 00 00 54 */	b lbl_800418F8
/* 800418A8 0003CE68  80 05 00 00 */	lwz r0, 0(r5)
/* 800418AC 0003CE6C  90 03 00 1C */	stw r0, 0x1c(r3)
/* 800418B0 0003CE70  48 00 00 48 */	b lbl_800418F8
/* 800418B4 0003CE74  80 05 00 00 */	lwz r0, 0(r5)
/* 800418B8 0003CE78  90 03 00 20 */	stw r0, 0x20(r3)
/* 800418BC 0003CE7C  48 00 00 3C */	b lbl_800418F8
/* 800418C0 0003CE80  80 05 00 00 */	lwz r0, 0(r5)
/* 800418C4 0003CE84  90 03 00 24 */	stw r0, 0x24(r3)
/* 800418C8 0003CE88  48 00 00 30 */	b lbl_800418F8
/* 800418CC 0003CE8C  80 05 00 00 */	lwz r0, 0(r5)
/* 800418D0 0003CE90  90 03 00 28 */	stw r0, 0x28(r3)
/* 800418D4 0003CE94  48 00 00 24 */	b lbl_800418F8
/* 800418D8 0003CE98  80 05 00 00 */	lwz r0, 0(r5)
/* 800418DC 0003CE9C  90 03 00 2C */	stw r0, 0x2c(r3)
/* 800418E0 0003CEA0  48 00 00 18 */	b lbl_800418F8
/* 800418E4 0003CEA4  80 05 00 00 */	lwz r0, 0(r5)
/* 800418E8 0003CEA8  90 03 00 30 */	stw r0, 0x30(r3)
/* 800418EC 0003CEAC  48 00 00 0C */	b lbl_800418F8
lbl_800418F0:
/* 800418F0 0003CEB0  38 60 00 00 */	li r3, 0
/* 800418F4 0003CEB4  4E 80 00 20 */	blr 
lbl_800418F8:
/* 800418F8 0003CEB8  38 60 00 01 */	li r3, 1
/* 800418FC 0003CEBC  4E 80 00 20 */	blr 

glabel func_80041900
/* 80041900 0003CEC0  38 60 00 00 */	li r3, 0
/* 80041904 0003CEC4  4E 80 00 20 */	blr 

glabel func_80041908
/* 80041908 0003CEC8  38 60 00 00 */	li r3, 0
/* 8004190C 0003CECC  4E 80 00 20 */	blr 

glabel func_80041910
/* 80041910 0003CED0  38 60 00 00 */	li r3, 0
/* 80041914 0003CED4  4E 80 00 20 */	blr 

glabel func_80041918
/* 80041918 0003CED8  54 80 06 BE */	clrlwi r0, r4, 0x1a
/* 8004191C 0003CEDC  38 80 00 00 */	li r4, 0
/* 80041920 0003CEE0  28 00 00 24 */	cmplwi r0, 0x24
/* 80041924 0003CEE4  90 85 00 00 */	stw r4, 0(r5)
/* 80041928 0003CEE8  41 81 00 94 */	bgt lbl_800419BC
/* 8004192C 0003CEEC  3C 80 80 17 */	lis r4, lbl_80172104@ha
/* 80041930 0003CEF0  54 00 10 3A */	slwi r0, r0, 2
/* 80041934 0003CEF4  38 84 21 04 */	addi r4, r4, lbl_80172104@l
/* 80041938 0003CEF8  7C 84 00 2E */	lwzx r4, r4, r0
/* 8004193C 0003CEFC  7C 89 03 A6 */	mtctr r4
/* 80041940 0003CF00  4E 80 04 20 */	bctr 
/* 80041944 0003CF04  80 03 00 0C */	lwz r0, 0xc(r3)
/* 80041948 0003CF08  90 05 00 00 */	stw r0, 0(r5)
/* 8004194C 0003CF0C  48 00 00 78 */	b lbl_800419C4
/* 80041950 0003CF10  80 03 00 10 */	lwz r0, 0x10(r3)
/* 80041954 0003CF14  90 05 00 00 */	stw r0, 0(r5)
/* 80041958 0003CF18  48 00 00 6C */	b lbl_800419C4
/* 8004195C 0003CF1C  80 03 00 14 */	lwz r0, 0x14(r3)
/* 80041960 0003CF20  90 05 00 00 */	stw r0, 0(r5)
/* 80041964 0003CF24  48 00 00 60 */	b lbl_800419C4
/* 80041968 0003CF28  80 03 00 18 */	lwz r0, 0x18(r3)
/* 8004196C 0003CF2C  90 05 00 00 */	stw r0, 0(r5)
/* 80041970 0003CF30  48 00 00 54 */	b lbl_800419C4
/* 80041974 0003CF34  80 03 00 1C */	lwz r0, 0x1c(r3)
/* 80041978 0003CF38  90 05 00 00 */	stw r0, 0(r5)
/* 8004197C 0003CF3C  48 00 00 48 */	b lbl_800419C4
/* 80041980 0003CF40  80 03 00 20 */	lwz r0, 0x20(r3)
/* 80041984 0003CF44  90 05 00 00 */	stw r0, 0(r5)
/* 80041988 0003CF48  48 00 00 3C */	b lbl_800419C4
/* 8004198C 0003CF4C  80 03 00 24 */	lwz r0, 0x24(r3)
/* 80041990 0003CF50  90 05 00 00 */	stw r0, 0(r5)
/* 80041994 0003CF54  48 00 00 30 */	b lbl_800419C4
/* 80041998 0003CF58  80 03 00 28 */	lwz r0, 0x28(r3)
/* 8004199C 0003CF5C  90 05 00 00 */	stw r0, 0(r5)
/* 800419A0 0003CF60  48 00 00 24 */	b lbl_800419C4
/* 800419A4 0003CF64  80 03 00 2C */	lwz r0, 0x2c(r3)
/* 800419A8 0003CF68  90 05 00 00 */	stw r0, 0(r5)
/* 800419AC 0003CF6C  48 00 00 18 */	b lbl_800419C4
/* 800419B0 0003CF70  80 03 00 30 */	lwz r0, 0x30(r3)
/* 800419B4 0003CF74  90 05 00 00 */	stw r0, 0(r5)
/* 800419B8 0003CF78  48 00 00 0C */	b lbl_800419C4
lbl_800419BC:
/* 800419BC 0003CF7C  38 60 00 00 */	li r3, 0
/* 800419C0 0003CF80  4E 80 00 20 */	blr 
lbl_800419C4:
/* 800419C4 0003CF84  38 60 00 01 */	li r3, 1
/* 800419C8 0003CF88  4E 80 00 20 */	blr 

glabel func_800419CC
/* 800419CC 0003CF8C  38 60 00 00 */	li r3, 0
/* 800419D0 0003CF90  4E 80 00 20 */	blr 

glabel func_800419D4
/* 800419D4 0003CF94  38 60 00 00 */	li r3, 0
/* 800419D8 0003CF98  4E 80 00 20 */	blr 

glabel func_800419DC
/* 800419DC 0003CF9C  38 60 00 00 */	li r3, 0
/* 800419E0 0003CFA0  4E 80 00 20 */	blr 

glabel func_800419E4
/* 800419E4 0003CFA4  54 80 06 FE */	clrlwi r0, r4, 0x1b
/* 800419E8 0003CFA8  28 00 00 1C */	cmplwi r0, 0x1c
/* 800419EC 0003CFAC  41 81 00 68 */	bgt lbl_80041A54
/* 800419F0 0003CFB0  3C 80 80 17 */	lis r4, lbl_80172198@ha
/* 800419F4 0003CFB4  54 00 10 3A */	slwi r0, r0, 2
/* 800419F8 0003CFB8  38 84 21 98 */	addi r4, r4, lbl_80172198@l
/* 800419FC 0003CFBC  7C 84 00 2E */	lwzx r4, r4, r0
/* 80041A00 0003CFC0  7C 89 03 A6 */	mtctr r4
/* 80041A04 0003CFC4  4E 80 04 20 */	bctr 
/* 80041A08 0003CFC8  80 05 00 00 */	lwz r0, 0(r5)
/* 80041A0C 0003CFCC  54 00 07 3E */	clrlwi r0, r0, 0x1c
/* 80041A10 0003CFD0  90 03 00 34 */	stw r0, 0x34(r3)
/* 80041A14 0003CFD4  48 00 00 48 */	b lbl_80041A5C
/* 80041A18 0003CFD8  80 05 00 00 */	lwz r0, 0(r5)
/* 80041A1C 0003CFDC  54 00 06 7E */	clrlwi r0, r0, 0x19
/* 80041A20 0003CFE0  90 03 00 38 */	stw r0, 0x38(r3)
/* 80041A24 0003CFE4  48 00 00 38 */	b lbl_80041A5C
/* 80041A28 0003CFE8  80 05 00 00 */	lwz r0, 0(r5)
/* 80041A2C 0003CFEC  54 00 07 7E */	clrlwi r0, r0, 0x1d
/* 80041A30 0003CFF0  90 03 00 3C */	stw r0, 0x3c(r3)
/* 80041A34 0003CFF4  48 00 00 28 */	b lbl_80041A5C
/* 80041A38 0003CFF8  80 05 00 00 */	lwz r0, 0(r5)
/* 80041A3C 0003CFFC  90 03 00 40 */	stw r0, 0x40(r3)
/* 80041A40 0003D000  48 00 00 1C */	b lbl_80041A5C
/* 80041A44 0003D004  80 05 00 00 */	lwz r0, 0(r5)
/* 80041A48 0003D008  54 00 07 3E */	clrlwi r0, r0, 0x1c
/* 80041A4C 0003D00C  90 03 00 44 */	stw r0, 0x44(r3)
/* 80041A50 0003D010  48 00 00 0C */	b lbl_80041A5C
lbl_80041A54:
/* 80041A54 0003D014  38 60 00 00 */	li r3, 0
/* 80041A58 0003D018  4E 80 00 20 */	blr 
lbl_80041A5C:
/* 80041A5C 0003D01C  38 60 00 01 */	li r3, 1
/* 80041A60 0003D020  4E 80 00 20 */	blr 

glabel func_80041A64
/* 80041A64 0003D024  38 60 00 00 */	li r3, 0
/* 80041A68 0003D028  4E 80 00 20 */	blr 

glabel func_80041A6C
/* 80041A6C 0003D02C  38 60 00 00 */	li r3, 0
/* 80041A70 0003D030  4E 80 00 20 */	blr 

glabel func_80041A74
/* 80041A74 0003D034  38 60 00 00 */	li r3, 0
/* 80041A78 0003D038  4E 80 00 20 */	blr 

glabel func_80041A7C
/* 80041A7C 0003D03C  54 80 06 FE */	clrlwi r0, r4, 0x1b
/* 80041A80 0003D040  28 00 00 1C */	cmplwi r0, 0x1c
/* 80041A84 0003D044  41 81 00 74 */	bgt lbl_80041AF8
/* 80041A88 0003D048  3C 80 80 17 */	lis r4, lbl_8017220C@ha
/* 80041A8C 0003D04C  54 00 10 3A */	slwi r0, r0, 2
/* 80041A90 0003D050  38 84 22 0C */	addi r4, r4, lbl_8017220C@l
/* 80041A94 0003D054  7C 84 00 2E */	lwzx r4, r4, r0
/* 80041A98 0003D058  7C 89 03 A6 */	mtctr r4
/* 80041A9C 0003D05C  4E 80 04 20 */	bctr 
/* 80041AA0 0003D060  80 03 00 34 */	lwz r0, 0x34(r3)
/* 80041AA4 0003D064  54 00 07 3E */	clrlwi r0, r0, 0x1c
/* 80041AA8 0003D068  90 05 00 00 */	stw r0, 0(r5)
/* 80041AAC 0003D06C  48 00 00 54 */	b lbl_80041B00
/* 80041AB0 0003D070  80 03 00 38 */	lwz r0, 0x38(r3)
/* 80041AB4 0003D074  54 00 06 7E */	clrlwi r0, r0, 0x19
/* 80041AB8 0003D078  90 05 00 00 */	stw r0, 0(r5)
/* 80041ABC 0003D07C  48 00 00 44 */	b lbl_80041B00
/* 80041AC0 0003D080  80 03 00 3C */	lwz r0, 0x3c(r3)
/* 80041AC4 0003D084  54 00 07 7E */	clrlwi r0, r0, 0x1d
/* 80041AC8 0003D088  90 05 00 00 */	stw r0, 0(r5)
/* 80041ACC 0003D08C  48 00 00 34 */	b lbl_80041B00
/* 80041AD0 0003D090  80 03 00 40 */	lwz r0, 0x40(r3)
/* 80041AD4 0003D094  90 05 00 00 */	stw r0, 0(r5)
/* 80041AD8 0003D098  48 00 00 28 */	b lbl_80041B00
/* 80041ADC 0003D09C  80 03 00 44 */	lwz r0, 0x44(r3)
/* 80041AE0 0003D0A0  54 00 07 3E */	clrlwi r0, r0, 0x1c
/* 80041AE4 0003D0A4  90 05 00 00 */	stw r0, 0(r5)
/* 80041AE8 0003D0A8  48 00 00 18 */	b lbl_80041B00
/* 80041AEC 0003D0AC  38 00 00 00 */	li r0, 0
/* 80041AF0 0003D0B0  90 05 00 00 */	stw r0, 0(r5)
/* 80041AF4 0003D0B4  48 00 00 0C */	b lbl_80041B00
lbl_80041AF8:
/* 80041AF8 0003D0B8  38 60 00 00 */	li r3, 0
/* 80041AFC 0003D0BC  4E 80 00 20 */	blr 
lbl_80041B00:
/* 80041B00 0003D0C0  38 60 00 01 */	li r3, 1
/* 80041B04 0003D0C4  4E 80 00 20 */	blr 

glabel func_80041B08
/* 80041B08 0003D0C8  38 60 00 00 */	li r3, 0
/* 80041B0C 0003D0CC  4E 80 00 20 */	blr 

glabel func_80041B10
/* 80041B10 0003D0D0  80 03 00 08 */	lwz r0, 8(r3)
/* 80041B14 0003D0D4  54 84 01 BE */	clrlwi r4, r4, 6
/* 80041B18 0003D0D8  7C 04 00 40 */	cmplw r4, r0
/* 80041B1C 0003D0DC  40 80 00 10 */	bge lbl_80041B2C
/* 80041B20 0003D0E0  88 05 00 00 */	lbz r0, 0(r5)
/* 80041B24 0003D0E4  80 63 00 04 */	lwz r3, 4(r3)
/* 80041B28 0003D0E8  7C 03 21 AE */	stbx r0, r3, r4
lbl_80041B2C:
/* 80041B2C 0003D0EC  38 60 00 01 */	li r3, 1
/* 80041B30 0003D0F0  4E 80 00 20 */	blr 

glabel func_80041B34
/* 80041B34 0003D0F4  80 03 00 08 */	lwz r0, 8(r3)
/* 80041B38 0003D0F8  54 86 01 BE */	clrlwi r6, r4, 6
/* 80041B3C 0003D0FC  7C 06 00 40 */	cmplw r6, r0
/* 80041B40 0003D100  40 80 00 14 */	bge lbl_80041B54
/* 80041B44 0003D104  A0 85 00 00 */	lhz r4, 0(r5)
/* 80041B48 0003D108  54 C0 00 3C */	rlwinm r0, r6, 0, 0, 0x1e
/* 80041B4C 0003D10C  80 63 00 04 */	lwz r3, 4(r3)
/* 80041B50 0003D110  7C 83 03 2E */	sthx r4, r3, r0
lbl_80041B54:
/* 80041B54 0003D114  38 60 00 01 */	li r3, 1
/* 80041B58 0003D118  4E 80 00 20 */	blr 

glabel func_80041B5C
/* 80041B5C 0003D11C  80 03 00 08 */	lwz r0, 8(r3)
/* 80041B60 0003D120  54 86 01 BE */	clrlwi r6, r4, 6
/* 80041B64 0003D124  7C 06 00 40 */	cmplw r6, r0
/* 80041B68 0003D128  40 80 00 14 */	bge lbl_80041B7C
/* 80041B6C 0003D12C  80 85 00 00 */	lwz r4, 0(r5)
/* 80041B70 0003D130  54 C0 00 3A */	rlwinm r0, r6, 0, 0, 0x1d
/* 80041B74 0003D134  80 63 00 04 */	lwz r3, 4(r3)
/* 80041B78 0003D138  7C 83 01 2E */	stwx r4, r3, r0
lbl_80041B7C:
/* 80041B7C 0003D13C  38 60 00 01 */	li r3, 1
/* 80041B80 0003D140  4E 80 00 20 */	blr 

glabel func_80041B84
/* 80041B84 0003D144  80 03 00 08 */	lwz r0, 8(r3)
/* 80041B88 0003D148  54 84 01 BE */	clrlwi r4, r4, 6
/* 80041B8C 0003D14C  7C 04 00 40 */	cmplw r4, r0
/* 80041B90 0003D150  40 80 00 20 */	bge lbl_80041BB0
/* 80041B94 0003D154  80 63 00 04 */	lwz r3, 4(r3)
/* 80041B98 0003D158  54 80 00 38 */	rlwinm r0, r4, 0, 0, 0x1c
/* 80041B9C 0003D15C  80 85 00 00 */	lwz r4, 0(r5)
/* 80041BA0 0003D160  80 A5 00 04 */	lwz r5, 4(r5)
/* 80041BA4 0003D164  7C 63 02 14 */	add r3, r3, r0
/* 80041BA8 0003D168  90 A3 00 04 */	stw r5, 4(r3)
/* 80041BAC 0003D16C  90 83 00 00 */	stw r4, 0(r3)
lbl_80041BB0:
/* 80041BB0 0003D170  38 60 00 01 */	li r3, 1
/* 80041BB4 0003D174  4E 80 00 20 */	blr 

glabel func_80041BB8
/* 80041BB8 0003D178  80 03 00 08 */	lwz r0, 8(r3)
/* 80041BBC 0003D17C  54 84 01 BE */	clrlwi r4, r4, 6
/* 80041BC0 0003D180  7C 04 00 40 */	cmplw r4, r0
/* 80041BC4 0003D184  40 80 00 14 */	bge lbl_80041BD8
/* 80041BC8 0003D188  80 63 00 04 */	lwz r3, 4(r3)
/* 80041BCC 0003D18C  7C 03 20 AE */	lbzx r0, r3, r4
/* 80041BD0 0003D190  98 05 00 00 */	stb r0, 0(r5)
/* 80041BD4 0003D194  48 00 00 0C */	b lbl_80041BE0
lbl_80041BD8:
/* 80041BD8 0003D198  38 00 00 00 */	li r0, 0
/* 80041BDC 0003D19C  98 05 00 00 */	stb r0, 0(r5)
lbl_80041BE0:
/* 80041BE0 0003D1A0  38 60 00 01 */	li r3, 1
/* 80041BE4 0003D1A4  4E 80 00 20 */	blr 

glabel func_80041BE8
/* 80041BE8 0003D1A8  80 03 00 08 */	lwz r0, 8(r3)
/* 80041BEC 0003D1AC  54 84 01 BE */	clrlwi r4, r4, 6
/* 80041BF0 0003D1B0  7C 04 00 40 */	cmplw r4, r0
/* 80041BF4 0003D1B4  40 80 00 18 */	bge lbl_80041C0C
/* 80041BF8 0003D1B8  80 63 00 04 */	lwz r3, 4(r3)
/* 80041BFC 0003D1BC  54 80 00 3C */	rlwinm r0, r4, 0, 0, 0x1e
/* 80041C00 0003D1C0  7C 03 02 AE */	lhax r0, r3, r0
/* 80041C04 0003D1C4  B0 05 00 00 */	sth r0, 0(r5)
/* 80041C08 0003D1C8  48 00 00 0C */	b lbl_80041C14
lbl_80041C0C:
/* 80041C0C 0003D1CC  38 00 00 00 */	li r0, 0
/* 80041C10 0003D1D0  B0 05 00 00 */	sth r0, 0(r5)
lbl_80041C14:
/* 80041C14 0003D1D4  38 60 00 01 */	li r3, 1
/* 80041C18 0003D1D8  4E 80 00 20 */	blr 

glabel func_80041C1C
/* 80041C1C 0003D1DC  80 03 00 08 */	lwz r0, 8(r3)
/* 80041C20 0003D1E0  54 84 01 BE */	clrlwi r4, r4, 6
/* 80041C24 0003D1E4  7C 04 00 40 */	cmplw r4, r0
/* 80041C28 0003D1E8  40 80 00 18 */	bge lbl_80041C40
/* 80041C2C 0003D1EC  80 63 00 04 */	lwz r3, 4(r3)
/* 80041C30 0003D1F0  54 80 00 3A */	rlwinm r0, r4, 0, 0, 0x1d
/* 80041C34 0003D1F4  7C 03 00 2E */	lwzx r0, r3, r0
/* 80041C38 0003D1F8  90 05 00 00 */	stw r0, 0(r5)
/* 80041C3C 0003D1FC  48 00 00 0C */	b lbl_80041C48
lbl_80041C40:
/* 80041C40 0003D200  38 00 00 00 */	li r0, 0
/* 80041C44 0003D204  90 05 00 00 */	stw r0, 0(r5)
lbl_80041C48:
/* 80041C48 0003D208  38 60 00 01 */	li r3, 1
/* 80041C4C 0003D20C  4E 80 00 20 */	blr 

glabel func_80041C50
/* 80041C50 0003D210  80 03 00 08 */	lwz r0, 8(r3)
/* 80041C54 0003D214  54 84 01 BE */	clrlwi r4, r4, 6
/* 80041C58 0003D218  7C 04 00 40 */	cmplw r4, r0
/* 80041C5C 0003D21C  40 80 00 20 */	bge lbl_80041C7C
/* 80041C60 0003D220  54 80 00 38 */	rlwinm r0, r4, 0, 0, 0x1c
/* 80041C64 0003D224  80 63 00 04 */	lwz r3, 4(r3)
/* 80041C68 0003D228  7C 03 00 6E */	lwzux r0, r3, r0
/* 80041C6C 0003D22C  80 63 00 04 */	lwz r3, 4(r3)
/* 80041C70 0003D230  90 65 00 04 */	stw r3, 4(r5)
/* 80041C74 0003D234  90 05 00 00 */	stw r0, 0(r5)
/* 80041C78 0003D238  48 00 00 10 */	b lbl_80041C88
lbl_80041C7C:
/* 80041C7C 0003D23C  38 00 00 00 */	li r0, 0
/* 80041C80 0003D240  90 05 00 04 */	stw r0, 4(r5)
/* 80041C84 0003D244  90 05 00 00 */	stw r0, 0(r5)
lbl_80041C88:
/* 80041C88 0003D248  38 60 00 01 */	li r3, 1
/* 80041C8C 0003D24C  4E 80 00 20 */	blr 

glabel func_80041C90
/* 80041C90 0003D250  94 21 FF F0 */	stwu r1, -0x10(r1)
/* 80041C94 0003D254  7C 08 02 A6 */	mflr r0
/* 80041C98 0003D258  90 01 00 14 */	stw r0, 0x14(r1)
/* 80041C9C 0003D25C  81 84 00 08 */	lwz r12, 8(r4)
/* 80041CA0 0003D260  2C 0C 00 00 */	cmpwi r12, 0
/* 80041CA4 0003D264  41 82 00 24 */	beq lbl_80041CC8
/* 80041CA8 0003D268  7C 83 23 78 */	mr r3, r4
/* 80041CAC 0003D26C  38 80 00 01 */	li r4, 1
/* 80041CB0 0003D270  7D 89 03 A6 */	mtctr r12
/* 80041CB4 0003D274  4E 80 04 21 */	bctrl 
/* 80041CB8 0003D278  2C 03 00 00 */	cmpwi r3, 0
/* 80041CBC 0003D27C  40 82 00 0C */	bne lbl_80041CC8
/* 80041CC0 0003D280  38 60 00 00 */	li r3, 0
/* 80041CC4 0003D284  48 00 00 08 */	b lbl_80041CCC
lbl_80041CC8:
/* 80041CC8 0003D288  38 60 00 01 */	li r3, 1
lbl_80041CCC:
/* 80041CCC 0003D28C  80 01 00 14 */	lwz r0, 0x14(r1)
/* 80041CD0 0003D290  7C 08 03 A6 */	mtlr r0
/* 80041CD4 0003D294  38 21 00 10 */	addi r1, r1, 0x10
/* 80041CD8 0003D298  4E 80 00 20 */	blr 

glabel ramGetBuffer
/* 80041CDC 0003D29C  80 E3 00 08 */	lwz r7, 8(r3)
/* 80041CE0 0003D2A0  54 A5 01 BE */	clrlwi r5, r5, 6
/* 80041CE4 0003D2A4  2C 07 00 00 */	cmpwi r7, 0
/* 80041CE8 0003D2A8  40 82 00 0C */	bne lbl_80041CF4
/* 80041CEC 0003D2AC  38 60 00 00 */	li r3, 0
/* 80041CF0 0003D2B0  4E 80 00 20 */	blr 
lbl_80041CF4:
/* 80041CF4 0003D2B4  2C 06 00 00 */	cmpwi r6, 0
/* 80041CF8 0003D2B8  41 82 00 28 */	beq lbl_80041D20
/* 80041CFC 0003D2BC  80 06 00 00 */	lwz r0, 0(r6)
/* 80041D00 0003D2C0  7C 05 02 14 */	add r0, r5, r0
/* 80041D04 0003D2C4  7C 00 38 40 */	cmplw r0, r7
/* 80041D08 0003D2C8  41 80 00 18 */	blt lbl_80041D20
/* 80041D0C 0003D2CC  7C 05 38 51 */	subf. r0, r5, r7
/* 80041D10 0003D2D0  90 06 00 00 */	stw r0, 0(r6)
/* 80041D14 0003D2D4  40 80 00 0C */	bge lbl_80041D20
/* 80041D18 0003D2D8  38 00 00 00 */	li r0, 0
/* 80041D1C 0003D2DC  90 06 00 00 */	stw r0, 0(r6)
lbl_80041D20:
/* 80041D20 0003D2E0  80 03 00 04 */	lwz r0, 4(r3)
/* 80041D24 0003D2E4  38 60 00 01 */	li r3, 1
/* 80041D28 0003D2E8  7C 00 2A 14 */	add r0, r0, r5
/* 80041D2C 0003D2EC  90 04 00 00 */	stw r0, 0(r4)
/* 80041D30 0003D2F0  4E 80 00 20 */	blr 

glabel func_80041D34
/* 80041D34 0003D2F4  94 21 FF F0 */	stwu r1, -0x10(r1)
/* 80041D38 0003D2F8  7C 08 02 A6 */	mflr r0
/* 80041D3C 0003D2FC  90 01 00 14 */	stw r0, 0x14(r1)
/* 80041D40 0003D300  80 83 00 08 */	lwz r4, 8(r3)
/* 80041D44 0003D304  2C 04 00 00 */	cmpwi r4, 0
/* 80041D48 0003D308  41 82 00 20 */	beq lbl_80041D68
/* 80041D4C 0003D30C  80 63 00 04 */	lwz r3, 4(r3)
/* 80041D50 0003D310  38 A0 00 00 */	li r5, 0
/* 80041D54 0003D314  48 03 FF 91 */	bl xlHeapFill32
/* 80041D58 0003D318  2C 03 00 00 */	cmpwi r3, 0
/* 80041D5C 0003D31C  40 82 00 0C */	bne lbl_80041D68
/* 80041D60 0003D320  38 60 00 00 */	li r3, 0
/* 80041D64 0003D324  48 00 00 08 */	b lbl_80041D6C
lbl_80041D68:
/* 80041D68 0003D328  38 60 00 01 */	li r3, 1
lbl_80041D6C:
/* 80041D6C 0003D32C  80 01 00 14 */	lwz r0, 0x14(r1)
/* 80041D70 0003D330  7C 08 03 A6 */	mtlr r0
/* 80041D74 0003D334  38 21 00 10 */	addi r1, r1, 0x10
/* 80041D78 0003D338  4E 80 00 20 */	blr 

glabel func_80041D7C
/* 80041D7C 0003D33C  94 21 FF F0 */	stwu r1, -0x10(r1)
/* 80041D80 0003D340  7C 08 02 A6 */	mflr r0
/* 80041D84 0003D344  90 01 00 14 */	stw r0, 0x14(r1)
/* 80041D88 0003D348  93 E1 00 0C */	stw r31, 0xc(r1)
/* 80041D8C 0003D34C  93 C1 00 08 */	stw r30, 8(r1)
/* 80041D90 0003D350  7C 7E 1B 78 */	mr r30, r3
/* 80041D94 0003D354  80 03 00 04 */	lwz r0, 4(r3)
/* 80041D98 0003D358  2C 00 00 00 */	cmpwi r0, 0
/* 80041D9C 0003D35C  41 82 00 0C */	beq lbl_80041DA8
/* 80041DA0 0003D360  38 60 00 00 */	li r3, 0
/* 80041DA4 0003D364  48 00 00 30 */	b lbl_80041DD4
lbl_80041DA8:
/* 80041DA8 0003D368  38 04 03 FF */	addi r0, r4, 0x3ff
/* 80041DAC 0003D36C  38 63 00 04 */	addi r3, r3, 4
/* 80041DB0 0003D370  54 1F 00 2A */	rlwinm r31, r0, 0, 0, 0x15
/* 80041DB4 0003D374  67 E4 30 00 */	oris r4, r31, 0x3000
/* 80041DB8 0003D378  48 03 F3 41 */	bl xlHeapTake
/* 80041DBC 0003D37C  2C 03 00 00 */	cmpwi r3, 0
/* 80041DC0 0003D380  40 82 00 0C */	bne lbl_80041DCC
/* 80041DC4 0003D384  38 60 00 00 */	li r3, 0
/* 80041DC8 0003D388  48 00 00 0C */	b lbl_80041DD4
lbl_80041DCC:
/* 80041DCC 0003D38C  93 FE 00 08 */	stw r31, 8(r30)
/* 80041DD0 0003D390  38 60 00 01 */	li r3, 1
lbl_80041DD4:
/* 80041DD4 0003D394  80 01 00 14 */	lwz r0, 0x14(r1)
/* 80041DD8 0003D398  83 E1 00 0C */	lwz r31, 0xc(r1)
/* 80041DDC 0003D39C  83 C1 00 08 */	lwz r30, 8(r1)
/* 80041DE0 0003D3A0  7C 08 03 A6 */	mtlr r0
/* 80041DE4 0003D3A4  38 21 00 10 */	addi r1, r1, 0x10
/* 80041DE8 0003D3A8  4E 80 00 20 */	blr 

glabel func_80041DEC
/* 80041DEC 0003D3AC  2C 04 00 00 */	cmpwi r4, 0
/* 80041DF0 0003D3B0  41 82 00 0C */	beq lbl_80041DFC
/* 80041DF4 0003D3B4  80 03 00 08 */	lwz r0, 8(r3)
/* 80041DF8 0003D3B8  90 04 00 00 */	stw r0, 0(r4)
lbl_80041DFC:
/* 80041DFC 0003D3BC  38 60 00 01 */	li r3, 1
/* 80041E00 0003D3C0  4E 80 00 20 */	blr 

glabel func_80041E04
/* 80041E04 0003D3C4  94 21 FF F0 */	stwu r1, -0x10(r1)
/* 80041E08 0003D3C8  7C 08 02 A6 */	mflr r0
/* 80041E0C 0003D3CC  2C 04 10 02 */	cmpwi r4, 0x1002
/* 80041E10 0003D3D0  90 01 00 14 */	stw r0, 0x14(r1)
/* 80041E14 0003D3D4  93 E1 00 0C */	stw r31, 0xc(r1)
/* 80041E18 0003D3D8  7C BF 2B 78 */	mr r31, r5
/* 80041E1C 0003D3DC  41 82 00 54 */	beq lbl_80041E70
/* 80041E20 0003D3E0  40 80 00 28 */	bge lbl_80041E48
/* 80041E24 0003D3E4  2C 04 00 02 */	cmpwi r4, 2
/* 80041E28 0003D3E8  41 82 00 38 */	beq lbl_80041E60
/* 80041E2C 0003D3EC  40 80 00 10 */	bge lbl_80041E3C
/* 80041E30 0003D3F0  2C 04 00 00 */	cmpwi r4, 0
/* 80041E34 0003D3F4  40 80 02 18 */	bge lbl_8004204C
/* 80041E38 0003D3F8  48 00 02 0C */	b lbl_80042044
lbl_80041E3C:
/* 80041E3C 0003D3FC  2C 04 00 04 */	cmpwi r4, 4
/* 80041E40 0003D400  40 80 02 04 */	bge lbl_80042044
/* 80041E44 0003D404  48 00 02 08 */	b lbl_8004204C
lbl_80041E48:
/* 80041E48 0003D408  2C 04 10 07 */	cmpwi r4, 0x1007
/* 80041E4C 0003D40C  41 82 02 00 */	beq lbl_8004204C
/* 80041E50 0003D410  40 80 01 F4 */	bge lbl_80042044
/* 80041E54 0003D414  2C 04 10 05 */	cmpwi r4, 0x1005
/* 80041E58 0003D418  40 80 01 EC */	bge lbl_80042044
/* 80041E5C 0003D41C  48 00 01 F0 */	b lbl_8004204C
lbl_80041E60:
/* 80041E60 0003D420  38 00 00 00 */	li r0, 0
/* 80041E64 0003D424  90 03 00 08 */	stw r0, 8(r3)
/* 80041E68 0003D428  90 03 00 04 */	stw r0, 4(r3)
/* 80041E6C 0003D42C  48 00 01 E0 */	b lbl_8004204C
lbl_80041E70:
/* 80041E70 0003D430  80 05 00 00 */	lwz r0, 0(r5)
/* 80041E74 0003D434  54 00 06 3E */	clrlwi r0, r0, 0x18
/* 80041E78 0003D438  2C 00 00 01 */	cmpwi r0, 1
/* 80041E7C 0003D43C  41 82 00 C8 */	beq lbl_80041F44
/* 80041E80 0003D440  40 80 00 10 */	bge lbl_80041E90
/* 80041E84 0003D444  2C 00 00 00 */	cmpwi r0, 0
/* 80041E88 0003D448  40 80 00 14 */	bge lbl_80041E9C
/* 80041E8C 0003D44C  48 00 01 C0 */	b lbl_8004204C
lbl_80041E90:
/* 80041E90 0003D450  2C 00 00 03 */	cmpwi r0, 3
/* 80041E94 0003D454  40 80 01 B8 */	bge lbl_8004204C
/* 80041E98 0003D458  48 00 01 2C */	b lbl_80041FC4
lbl_80041E9C:
/* 80041E9C 0003D45C  80 6D 89 20 */	lwz r3, lbl_8025CFE0-_SDA_BASE_(r13)
/* 80041EA0 0003D460  3C A0 80 04 */	lis r5, func_80041C90@ha
/* 80041EA4 0003D464  7F E4 FB 78 */	mr r4, r31
/* 80041EA8 0003D468  80 63 00 10 */	lwz r3, 0x10(r3)
/* 80041EAC 0003D46C  38 A5 1C 90 */	addi r5, r5, func_80041C90@l
/* 80041EB0 0003D470  4B FF A8 F5 */	bl func_8003C7A4
/* 80041EB4 0003D474  2C 03 00 00 */	cmpwi r3, 0
/* 80041EB8 0003D478  40 82 00 0C */	bne lbl_80041EC4
/* 80041EBC 0003D47C  38 60 00 00 */	li r3, 0
/* 80041EC0 0003D480  48 00 01 90 */	b lbl_80042050
lbl_80041EC4:
/* 80041EC4 0003D484  80 6D 89 20 */	lwz r3, lbl_8025CFE0-_SDA_BASE_(r13)
/* 80041EC8 0003D488  3C A0 80 04 */	lis r5, func_80041B10@ha
/* 80041ECC 0003D48C  3C C0 80 04 */	lis r6, func_80041B34@ha
/* 80041ED0 0003D490  3C E0 80 04 */	lis r7, func_80041B5C@ha
/* 80041ED4 0003D494  3D 00 80 04 */	lis r8, func_80041B84@ha
/* 80041ED8 0003D498  80 63 00 10 */	lwz r3, 0x10(r3)
/* 80041EDC 0003D49C  7F E4 FB 78 */	mr r4, r31
/* 80041EE0 0003D4A0  38 A5 1B 10 */	addi r5, r5, func_80041B10@l
/* 80041EE4 0003D4A4  38 C6 1B 34 */	addi r6, r6, func_80041B34@l
/* 80041EE8 0003D4A8  38 E7 1B 5C */	addi r7, r7, func_80041B5C@l
/* 80041EEC 0003D4AC  39 08 1B 84 */	addi r8, r8, func_80041B84@l
/* 80041EF0 0003D4B0  4B FF A8 D9 */	bl func_8003C7C8
/* 80041EF4 0003D4B4  2C 03 00 00 */	cmpwi r3, 0
/* 80041EF8 0003D4B8  40 82 00 0C */	bne lbl_80041F04
/* 80041EFC 0003D4BC  38 60 00 00 */	li r3, 0
/* 80041F00 0003D4C0  48 00 01 50 */	b lbl_80042050
lbl_80041F04:
/* 80041F04 0003D4C4  80 6D 89 20 */	lwz r3, lbl_8025CFE0-_SDA_BASE_(r13)
/* 80041F08 0003D4C8  3C A0 80 04 */	lis r5, func_80041BB8@ha
/* 80041F0C 0003D4CC  3C C0 80 04 */	lis r6, func_80041BE8@ha
/* 80041F10 0003D4D0  3C E0 80 04 */	lis r7, func_80041C1C@ha
/* 80041F14 0003D4D4  3D 00 80 04 */	lis r8, func_80041C50@ha
/* 80041F18 0003D4D8  80 63 00 10 */	lwz r3, 0x10(r3)
/* 80041F1C 0003D4DC  7F E4 FB 78 */	mr r4, r31
/* 80041F20 0003D4E0  38 A5 1B B8 */	addi r5, r5, func_80041BB8@l
/* 80041F24 0003D4E4  38 C6 1B E8 */	addi r6, r6, func_80041BE8@l
/* 80041F28 0003D4E8  38 E7 1C 1C */	addi r7, r7, func_80041C1C@l
/* 80041F2C 0003D4EC  39 08 1C 50 */	addi r8, r8, func_80041C50@l
/* 80041F30 0003D4F0  4B FF A8 81 */	bl func_8003C7B0
/* 80041F34 0003D4F4  2C 03 00 00 */	cmpwi r3, 0
/* 80041F38 0003D4F8  40 82 01 14 */	bne lbl_8004204C
/* 80041F3C 0003D4FC  38 60 00 00 */	li r3, 0
/* 80041F40 0003D500  48 00 01 10 */	b lbl_80042050
lbl_80041F44:
/* 80041F44 0003D504  80 6D 89 20 */	lwz r3, lbl_8025CFE0-_SDA_BASE_(r13)
/* 80041F48 0003D508  3C A0 80 04 */	lis r5, func_800419D4@ha
/* 80041F4C 0003D50C  3C C0 80 04 */	lis r6, func_800419DC@ha
/* 80041F50 0003D510  3C E0 80 04 */	lis r7, func_800419E4@ha
/* 80041F54 0003D514  3D 00 80 04 */	lis r8, func_80041A64@ha
/* 80041F58 0003D518  80 63 00 10 */	lwz r3, 0x10(r3)
/* 80041F5C 0003D51C  7F E4 FB 78 */	mr r4, r31
/* 80041F60 0003D520  38 A5 19 D4 */	addi r5, r5, func_800419D4@l
/* 80041F64 0003D524  38 C6 19 DC */	addi r6, r6, func_800419DC@l
/* 80041F68 0003D528  38 E7 19 E4 */	addi r7, r7, func_800419E4@l
/* 80041F6C 0003D52C  39 08 1A 64 */	addi r8, r8, func_80041A64@l
/* 80041F70 0003D530  4B FF A8 59 */	bl func_8003C7C8
/* 80041F74 0003D534  2C 03 00 00 */	cmpwi r3, 0
/* 80041F78 0003D538  40 82 00 0C */	bne lbl_80041F84
/* 80041F7C 0003D53C  38 60 00 00 */	li r3, 0
/* 80041F80 0003D540  48 00 00 D0 */	b lbl_80042050
lbl_80041F84:
/* 80041F84 0003D544  80 6D 89 20 */	lwz r3, lbl_8025CFE0-_SDA_BASE_(r13)
/* 80041F88 0003D548  3C A0 80 04 */	lis r5, func_80041A6C@ha
/* 80041F8C 0003D54C  3C C0 80 04 */	lis r6, func_80041A74@ha
/* 80041F90 0003D550  3C E0 80 04 */	lis r7, func_80041A7C@ha
/* 80041F94 0003D554  3D 00 80 04 */	lis r8, func_80041B08@ha
/* 80041F98 0003D558  80 63 00 10 */	lwz r3, 0x10(r3)
/* 80041F9C 0003D55C  7F E4 FB 78 */	mr r4, r31
/* 80041FA0 0003D560  38 A5 1A 6C */	addi r5, r5, func_80041A6C@l
/* 80041FA4 0003D564  38 C6 1A 74 */	addi r6, r6, func_80041A74@l
/* 80041FA8 0003D568  38 E7 1A 7C */	addi r7, r7, func_80041A7C@l
/* 80041FAC 0003D56C  39 08 1B 08 */	addi r8, r8, func_80041B08@l
/* 80041FB0 0003D570  4B FF A8 01 */	bl func_8003C7B0
/* 80041FB4 0003D574  2C 03 00 00 */	cmpwi r3, 0
/* 80041FB8 0003D578  40 82 00 94 */	bne lbl_8004204C
/* 80041FBC 0003D57C  38 60 00 00 */	li r3, 0
/* 80041FC0 0003D580  48 00 00 90 */	b lbl_80042050
lbl_80041FC4:
/* 80041FC4 0003D584  80 6D 89 20 */	lwz r3, lbl_8025CFE0-_SDA_BASE_(r13)
/* 80041FC8 0003D588  3C A0 80 04 */	lis r5, func_80041844@ha
/* 80041FCC 0003D58C  3C C0 80 04 */	lis r6, func_8004184C@ha
/* 80041FD0 0003D590  3C E0 80 04 */	lis r7, func_80041854@ha
/* 80041FD4 0003D594  3D 00 80 04 */	lis r8, func_80041900@ha
/* 80041FD8 0003D598  80 63 00 10 */	lwz r3, 0x10(r3)
/* 80041FDC 0003D59C  7F E4 FB 78 */	mr r4, r31
/* 80041FE0 0003D5A0  38 A5 18 44 */	addi r5, r5, func_80041844@l
/* 80041FE4 0003D5A4  38 C6 18 4C */	addi r6, r6, func_8004184C@l
/* 80041FE8 0003D5A8  38 E7 18 54 */	addi r7, r7, func_80041854@l
/* 80041FEC 0003D5AC  39 08 19 00 */	addi r8, r8, func_80041900@l
/* 80041FF0 0003D5B0  4B FF A7 D9 */	bl func_8003C7C8
/* 80041FF4 0003D5B4  2C 03 00 00 */	cmpwi r3, 0
/* 80041FF8 0003D5B8  40 82 00 0C */	bne lbl_80042004
/* 80041FFC 0003D5BC  38 60 00 00 */	li r3, 0
/* 80042000 0003D5C0  48 00 00 50 */	b lbl_80042050
lbl_80042004:
/* 80042004 0003D5C4  80 6D 89 20 */	lwz r3, lbl_8025CFE0-_SDA_BASE_(r13)
/* 80042008 0003D5C8  3C A0 80 04 */	lis r5, func_80041908@ha
/* 8004200C 0003D5CC  3C C0 80 04 */	lis r6, func_80041910@ha
/* 80042010 0003D5D0  3C E0 80 04 */	lis r7, func_80041918@ha
/* 80042014 0003D5D4  3D 00 80 04 */	lis r8, func_800419CC@ha
/* 80042018 0003D5D8  80 63 00 10 */	lwz r3, 0x10(r3)
/* 8004201C 0003D5DC  7F E4 FB 78 */	mr r4, r31
/* 80042020 0003D5E0  38 A5 19 08 */	addi r5, r5, func_80041908@l
/* 80042024 0003D5E4  38 C6 19 10 */	addi r6, r6, func_80041910@l
/* 80042028 0003D5E8  38 E7 19 18 */	addi r7, r7, func_80041918@l
/* 8004202C 0003D5EC  39 08 19 CC */	addi r8, r8, func_800419CC@l
/* 80042030 0003D5F0  4B FF A7 81 */	bl func_8003C7B0
/* 80042034 0003D5F4  2C 03 00 00 */	cmpwi r3, 0
/* 80042038 0003D5F8  40 82 00 14 */	bne lbl_8004204C
/* 8004203C 0003D5FC  38 60 00 00 */	li r3, 0
/* 80042040 0003D600  48 00 00 10 */	b lbl_80042050
lbl_80042044:
/* 80042044 0003D604  38 60 00 00 */	li r3, 0
/* 80042048 0003D608  48 00 00 08 */	b lbl_80042050
lbl_8004204C:
/* 8004204C 0003D60C  38 60 00 01 */	li r3, 1
lbl_80042050:
/* 80042050 0003D610  80 01 00 14 */	lwz r0, 0x14(r1)
/* 80042054 0003D614  83 E1 00 0C */	lwz r31, 0xc(r1)
/* 80042058 0003D618  7C 08 03 A6 */	mtlr r0
/* 8004205C 0003D61C  38 21 00 10 */	addi r1, r1, 0x10
/* 80042060 0003D620  4E 80 00 20 */	blr 

glabel func_80042064
/* 80042064 0003D624  80 8D 89 20 */	lwz r4, lbl_8025CFE0-_SDA_BASE_(r13)
/* 80042068 0003D628  38 00 00 01 */	li r0, 1
/* 8004206C 0003D62C  38 60 00 01 */	li r3, 1
/* 80042070 0003D630  80 84 00 1C */	lwz r4, 0x1c(r4)
/* 80042074 0003D634  90 04 00 08 */	stw r0, 8(r4)
/* 80042078 0003D638  4E 80 00 20 */	blr 