#ifndef _MIPS_H
#define _MIPS_H

#define MIPS_OP(inst) ((inst >> 0x1A) & 0x3F)

#define OPC_SPECIAL 0
#define OPC_REGIMM 1
#define OPC_J 2
#define OPC_JAL 3
#define OPC_BEQ 4
#define OPC_BNE 5
#define OPC_BLEZ 6
#define OPC_BGTZ 7
#define OPC_ADDI 8
#define OPC_ADDIU 9
#define OPC_SLTI 10
#define OPC_SLTIU 11
#define OPC_ANDI 12
#define OPC_ORI 13
#define OPC_XORI 14
#define OPC_LUI 15
#define OPC_CP0 16
#define OPC_CP1 17
#define OPC_BEQL 20
#define OPC_BNEL 21
#define OPC_BLEZL 22
#define OPC_BGTZL 23
#define OPC_DADDI 24
#define OPC_DADDIU 25
#define OPC_LDL 26
#define OPC_LDR 27
#define OPC_LB 32
#define OPC_LH 33
#define OPC_LWL 34
#define OPC_LW 35
#define OPC_LBU 36
#define OPC_LHU 37
#define OPC_LWR 38
#define OPC_LWU 39
#define OPC_SB 40
#define OPC_SH 41
#define OPC_SWL 42
#define OPC_SW 43
#define OPC_SDL 44
#define OPC_SDR 45
#define OPC_SWR 46
#define OPC_CACHE 47
#define OPC_LL 48
#define OPC_LWC1 49
#define OPC_LDC1 53
#define OPC_LD 55
#define OPC_SC 56
#define OPC_SWC1 57
#define OPC_SDC1 61
#define OPC_SD 63

#define SPEC_FUNCT(inst) ((inst >> 0) & 0x3F)
#define SPEC_SLL 0
#define SPEC_SRL 2
#define SPEC_SRA 3
#define SPEC_SLLV 4
#define SPEC_SRLV 6
#define SPEC_SRAV 7
#define SPEC_JR 8
#define SPEC_JALR 9
#define SPEC_SYSCALL 12
#define SPEC_BREAK 13
#define SPEC_MFHI 16
#define SPEC_MTHI 17
#define SPEC_MFLO 18
#define SPEC_MTLO 19
#define SPEC_DSLLV 20
#define SPEC_DSRLV 22
#define SPEC_DSRAV 23
#define SPEC_MULT 24
#define SPEC_MULTU 25
#define SPEC_DIV 26
#define SPEC_DIVU 27
#define SPEC_DMULT 28
#define SPEC_DMULTU 29
#define SPEC_DDIV 30
#define SPEC_DDIVU 31
#define SPEC_ADD 32
#define SPEC_ADDU 33
#define SPEC_SUB 34
#define SPEC_SUBU 35
#define SPEC_AND 36
#define SPEC_OR 37
#define SPEC_XOR 38
#define SPEC_NOR 39
#define SPEC_SLT 42
#define SPEC_SLTU 43
#define SPEC_DADD 44
#define SPEC_DADDU 45
#define SPEC_DSUB 46
#define SPEC_DSUBU 47
#define SPEC_TGE 48
#define SPEC_TGEU 49
#define SPEC_TLT 50
#define SPEC_TLTU 51 
#define SPEC_TEQ 52
#define SPEC_TNE 54
#define SPEC_DSLL 56
#define SPEC_DSRL 58
#define SPEC_DSRA 59
#define SPEC_DSLL32 60
#define SPEC_DSRL32 62
#define SPEC_DSRA32 63

#define REGIMM_SUB(inst) ((inst >> 16) & 0x1F)
#define REGIMM_BLTZ 0
#define REGIMM_BGEZ 1
#define REGIMM_BLTZL 2
#define REGIMM_BGEZL 3
#define REGIMM_TGEI 8
#define REGIMM_TGEIU 9
#define REGIMM_TLTI 10
#define REGIMM_TLTIU 11
#define REGIMM_TEQI 12
#define REGIMM_TNEI 14
#define REGIMM_BLTZAL 16
#define REGIMM_BGEZAL 17
#define REGIMM_BLTZALL 18
#define REGIMM_BGEZALL 19

#define MIPS_RD(inst) ((inst >> 0xB) & 0x1F)
#define MIPS_RT(inst) ((inst >> 0x10) & 0x1F)
#define MIPS_RS(inst) ((inst >> 0x15) & 0x1F)

#define MIPS_FDT(inst) ((inst >> 0) & 0x7FF)
#define MIPS_FR(inst) ((inst >> 0x15) & 0x1F)
#define MIPS_FT(inst) ((inst >> 0x10) & 0x1F)
#define MIPS_FS(inst) ((inst >> 0xB) & 0x1F)
#define MIPS_FD(inst) ((inst >> 6) & 0x1F)
#define MIPS_FSUB(inst) ((inst >> 0x15) & 0x1F)
#define MIPS_FFMT(inst) ((inst >> 0x15) & 0x1F)
#define MIPS_FFUNC(inst) ((inst >> 0x0) & 0x3F)
#define MIPS_FCC(inst) ((inst >> 8) & 7)
#define MIPS_FFMT3(inst) ((inst >> 0) & 7)
#define MIPS_FOP4(inst) ((inst >> 3) & 7)

#define MIPS_CP0FUNC(inst) ((inst >> 0) & 0x3F)

#define MIPS_FMT_SINGLE 16
#define MIPS_FMT_DOUBLE 17
#define MIPS_FMT_WORD 20
#define MIPS_FMT_DBLWORD 21

#define MIPS_FSUB_MFC 0
#define MIPS_FSUB_DMFC 1
#define MIPS_FSUB_CFC 2
#define MIPS_FSUB_MTC 4
#define MIPS_FSUB_DMTC 5
#define MIPS_FSUB_CTC 6

#define MIPS_IMM(inst) ((s16)(inst & 0xFFFF))
#define MIPS_SA(inst ) ((inst >> 6) & 0x1F)

#define CP0_INDEX 0
#define CP0_RANDOM 1
#define CP0_ENTRYLO0 2
#define CP0_ENTRYLO1 3
#define CP0_CONTEXT 4
#define CP0_PAGEMASK 5
#define CP0_WIRED 6
#define CP0_BADVADDR 8
#define CP0_COUNT 9
#define CP0_ENTRYHI 10
#define CP0_COMPARE 11
#define CP0_STATUS 12
#define CP0_CAUSE 13
#define CP0_EPC 14
#define CP0_PRID 15
#define CP0_CONFIG 16
#define CP0_LLADDR
#define CP0_WATCHLO 18
#define CP0_WATCHI 19
#define CP0_XCONTEXT 20
#define CP0_PERR 26
#define CP0_CACHEERR 27
#define CP0_TAGLO 28
#define CP0_TAGHI 29
#define CP0_ERROREPC 30

#define STATUS_KM64 (1 << 7)
#define STATUS_SM64 (1 << 6)
#define STATUS_UM64 (1 << 5)
#define STATUS_IE (1 << 0)
#define STATUS_EXL (1 << 1)
#define STATUS_ERL (1 << 2)

#define CPU_MODE_USER 2
#define CPU_MODE_SUPERVISOR 1
#define CPU_MODE_KERNEL 0

#define CAUSE_BD (1 << 31)

#define TLB_PGSZ_MASK 0x01FFE000
#define TLB_PGSZ_4K 0
#define TLB_PGSZ_16K 0x6000
#define TLB_PGSZ_64K 0x1E000
#define TLB_PGSZ_256K 0x7E000
#define TLB_PGSZ_1M 0x1FE000
#define TLB_PGSZ_4M 0x7FE000
#define TLB_PGSZ_16M 0x1FFE000

#define TLB_LO_GLBL (1 << 0)
#define TLB_LO_VALD (1 << 1)
#define TLB_LO_DRTY (1 << 2)
#define TLB_PFN_MASK 0x3FFFFC0

#define TLB_HI_ASID_MASK 0xFF
#define TLB__HIVPN2_MASK 0xFFFFE000

#define CP0_EX_NONE -1
#define CP0_EX_INTERRUPT 0
#define CP0_EX_TLBMOD 1
#define CP0_EX_TLBMISS_LD 2
#define CP0_EX_TLBMISS_ST 3
#define CP0_EX_ADDRERR_LD 4
#define CP0_EX_ADDRERR_ST 5
#define CP0_EX_BUSERR_INS 6
#define CP0_EX_BUSERR_DATA 7
#define CP0_EX_SYSCALL 8
#define CP0_EX_BREAK 9
#define CP0_EX_RSVD_INS 10
#define CP0_EX_COP_UNUSE 11
#define CP0_EX_OVERFLOW 12
#define CP0_EX_TRAP 13
#define CP0_EX_RFU14 14
#define CP0_EX_FP 15
#define CP0_EX_RFU16 16
#define CP0_EX_RFU17 17
#define CP0_EX_RFU18 18
#define CP0_EX_RFU19 19
#define CP0_EX_RFU20 20
#define CP0_EX_RFU21 21
#define CP0_EX_RFU22 22
#define CP0_EX_WATCH 23
#define CP0_EX_RFU24 24
#define CP0_EX_RFU25 25
#define CP0_EX_RFU26 26
#define CP0_EX_RFU27 27
#define CP0_EX_RFU28 28
#define CP0_EX_RFU29 29
#define CP0_EX_RFU30 30
#define CP0_EX_RFU31 31


#define MREG_R0 0
#define MREG_AT 1
#define MREG_V0 2
#define MREG_V1 3
#define MREG_A0 4
#define MREG_A1 5
#define MREG_A2 6
#define MREG_A3 7
#define MREG_T0 8
#define MREG_T1 9
#define MREG_T2 10
#define MREG_T3 11
#define MREG_T4 12
#define MREG_T5 13
#define MREG_T6 14
#define MREG_T7 15
#define MREG_S0 16
#define MREG_S1 17
#define MREG_S2 18
#define MREG_S3 19
#define MREG_S4 20
#define MREG_S5 21
#define MREG_S6 22
#define MREG_S7 23
#define MREG_T8 24
#define MREG_T9 25
#define MREG_K0 26
#define MREG_K1 27
#define MREG_GP 28
#define MREG_SP 29
#define MREG_FP 30
#define MREG_RA 31

#endif
