#ifndef _ROM_H_
#define _ROM_H_

#include "class.h"

typedef s32 unk_rom_callback(void);

typedef struct {
    /* 0x00000 */ class_ref_t common;
    /* 0x00008 */ s32 unk_8;
    /* 0x0000C */ s32 unk_C;
    /* 0x00010 */ char rom_fn[10];
    /* 0x0001A */ char unk_1A[0x1FA];
    /* 0x00214 */ s32 unk_214;
    /* 0x00218 */ char unk_218[0x4];
    /* 0x0021C */ s32 unk_21C;
    /* 0x00220 */ char unk_220[0x18000];
    /* 0x18220 */ s32 unk_18220;
    /* 0x18224 */ list_type_t* unk_18224;
    /* 0x18228 */ char unk_18228[0x1800];
    /* 0x19A28 */ s32 unk_19A28;
    /* 0x19A2C */ unk_rom_callback* unk_19A2C;
    /* 0x19A30 */ void* unk_19A30;
    /* 0x19A34 */ s32 unk_19A34;
    /* 0x19A38 */ s32 unk_19A38;
    /* 0x19A3C */ s32 unk_19A3C;
    /* 0x19A40 */ s32 unk_19A40;
    /* 0x19A44 */ u32 unk_19A44;
    /* 0x19A48 */ s32 unk_19A48;
    /* 0x19A4C */ char unk_19A4C[0xC];
    /* 0x19A58 */ s32 unk_19A58;
    /* 0x19A5C */ s32 unk_19A5C;
    /* 0x19A60 */ s32 unk_19A60;
    /* 0x19A64 */ char unk_19A64[0x4];
    /* 0x19A68 */ s32 unk_19A68;
    /* 0x19A6C */ s32 unk_19A6C;
    /* 0x19A70 */ s32 unk_19A70; // rom_size
    /* 0x19A74 */ u8 unk_19A74;
    /* 0x19A75 */ u8 unk_19A75;
    /* 0x19A76 */ char unk_19A76[0x38];
    /* 0x19AAE */ u8 unk_19AAE;
    /* 0x19AAF */ u8 unk_19AAF;
    /* 0x19AB0 */ u8 unk_19AB0;
    /* 0x19AB1 */ u8 unk_19AB1;
    /* 0x19AB1 */ u8 unk_19AB2;
    /* 0x19AB4 */ s32* unk_19AB4;
    /* 0x19AB8 */ s32 unk_19AB8;
    /* 0x19ABC */ s32 unk_19ABC;
    /* 0x19AC0 */ s32 unk_19AC0;
    /* 0x19AC4 */ s32 unk_19AC4;
    /* 0x19AC8 */ char unk_19AC8[0x30];
    /* 0x19AF8 */ u32 unk_19AF8;
    /* 0x19AFC */ u32 unk_19AFC;
    /* 0x19B00 */ u32 unk_19B00;
} rom_class_t; // size = 19AB2 +

typedef enum {
    CFZE = 'CFZE',
    CLBE = 'CLBE',
    CLBJ = 'CLBJ',
    CLBP = 'CLBP',
    CZLE = 'CZLE',
    CZLJ = 'CZLJ',
    NAFE = 'NAFE',
    NAFJ = 'NAFJ',
    NAFP = 'NAFP',
    NALE = 'NALE',
    NALJ = 'NALJ',
    NBCE = 'NBCE',
    NBCJ = 'NBCJ',
    NBCP = 'NBCP',
    NBNE = 'NBNE',
    NBNJ = 'NBNJ',
    NBNP = 'NBNP',
    NBYE = 'NBYE',
    NBYJ = 'NBYJ',
    NBYP = 'NBYP',
    NCUE = 'NCUE',
    NCUJ = 'NCUJ',
    NCUP = 'NCUP',
    NDOE = 'NDOE',
    NDOJ = 'NDOJ',
    NDOP = 'NDOP',
    NDYE = 'NDYE',
    NDYJ = 'NDYJ',
    NDYP = 'NDYP',
    NFXE = 'NFXE',
    NFXJ = 'NFXJ',
    NFXP = 'NFXP',
    NFZJ = 'NFZJ',
    NFZP = 'NFZP',
    NGUE = 'NGUE',
    NGUJ = 'NGUJ',
    NGUP = 'NGUP',
    NK4E = 'NK4E',
    NK4J = 'NK4J',
    NK4P = 'NK4P',
    NKTE = 'NKTE',
    NKTJ = 'NKTJ',
    NKTP = 'NKTP',
    NLRE = 'NLRE',
    NLRJ = 'NLRJ',
    NLRP = 'NLRP',
    NMFE = 'NMFE',
    NMFJ = 'NMFJ',
    NMQE = 'NMQE',
    NMQJ = 'NMQJ',
    NMQP = 'NMQP',
    NMVE = 'NMVE',
    NMVJ = 'NMVJ',
    NMVP = 'NMVP',
    NMWE = 'NMWE',
    NMWJ = 'NMWJ',
    NMWP = 'NMWP',
    NN6E = 'NN6E',
    NN6J = 'NN6J',
    NN6P = 'NN6P',
    NOBE = 'NOBE',
    NOBJ = 'NOBJ',
    NOBP = 'NOBP',
    NPOE = 'NPOE',
    NPOJ = 'NPOJ',
    NPOP = 'NPOP',
    NPWE = 'NPWE',
    NPWJ = 'NPWJ',
    NPWP = 'NPWP',
    NKQJ = 'NKQJ',
    NQKJ = 'NQKJ',
    NKQP = 'NKQP',
    NRBE = 'NRBE',
    NRBJ = 'NRBJ',
    NRBP = 'NRBP',
    NRIE = 'NRIE',
    NRIJ = 'NRIJ',
    NRIP = 'NRIP',
    NRXE = 'NRXE',
    NRXJ = 'NRXJ',
    NRXP = 'NRXP',
    NSIJ = 'NSIJ',
    NSME = 'NSME',
    NSMJ = 'NSMJ',
    NSMP = 'NSMP',
    NSQE = 'NSQE',
    NSQJ = 'NSQJ',
    NSQP = 'NSQP',
    NTEA = 'NTEA',
    NTEJ = 'NTEJ',
    NTEP = 'NTEP',
    NTUE = 'NTUE',
    NTUJ = 'NTUJ',
    NTUP = 'NTUP',
    NWRE = 'NWRE',
    NWRJ = 'NWRJ',
    NWRP = 'NWRP',
    NYLE = 'NYLE',
    NYLJ = 'NYLJ',
    NYLP = 'NYLP',
    NYSE = 'NYSE',
    NTSJ = 'NTSJ',
    NYSP = 'NYSP',
    NZLP = 'NZLP',
    NZSE = 'NZSE',
    NZSJ = 'NZSJ',
    NZSP = 'NZSP'
} rom_id_t;

s32 romCopy(rom_class_t* rom, void* dst, u32 addr, s32 len, unk_rom_callback arg4);
s32 romGetBuffer(rom_class_t* rom, void** param_2, u32 addr, s32* param_4);
s32 romUpdate(rom_class_t* rom);

#endif
