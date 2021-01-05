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
    CFZE = 0x43465a45,
    CLBE = 0x434c4245,
    CLBJ = 0x434c424a,
    CLBP = 0x434c4250,
    CZLE = 0x435a4c45,
    CZLJ = 0x435a4c4a,
    NAFE = 0x4e414645,
    NAFJ = 0x4e41464a,
    NAFP = 0x4e414650,
    NALE = 0x4e414c45,
    NALJ = 0x4e414c4a,
    NBCE = 0x4e424345,
    NBCJ = 0x4e42434a,
    NBCP = 0x4e424350,
    NBNE = 0x4e424e45,
    NBNJ = 0x4e424e4a,
    NBNP = 0x4e424e50,
    NBYE = 0x4e425945,
    NBYJ = 0x4e42594a,
    NBYP = 0x4e425950,
    NCUE = 0x4e435545,
    NCUJ = 0x4e43554a,
    NCUP = 0x4e435550,
    NDOE = 0x4e444f45,
    NDOJ = 0x4e444f4a,
    NDOP = 0x4e444f50,
    NDYE = 0x4e445945,
    NDYJ = 0x4e44594a,
    NDYP = 0x4e445950,
    NFXE = 0x4e465845,
    NFXJ = 0x4e46584a,
    NFXP = 0x4e465850,
    NFZJ = 0x4e465a4a,
    NFZP = 0x4e465a50,
    NGUE = 0x4e475545,
    NGUJ = 0x4e47554a,
    NGUP = 0x4e475550,
    NK4E = 0x4e4b3445,
    NK4J = 0x4e4b344a,
    NK4P = 0x4e4b3450,
    NKTE = 0x4e4b5445,
    NKTJ = 0x4e4b544a,
    NKTP = 0x4e4b5450,
    NLRE = 0x4e4c5245,
    NLRJ = 0x4e4c524a,
    NLRP = 0x4e4c5250,
    NMFE = 0x4e4d4645,
    NMFJ = 0x4e4d464a,
    NMQE = 0x4e4d5145,
    NMQJ = 0x4e4d514a,
    NMQP = 0x4e4d5150,
    NMVE = 0x4e4d5645,
    NMVJ = 0x4e4d564a,
    NMVP = 0x4e4d5650,
    NMWE = 0x4e4d5745,
    NMWJ = 0x4e4d574a,
    NMWP = 0x4e4d5750,
    NN6E = 0x4e4e3645,
    NN6J = 0x4e4e364a,
    NN6P = 0x4e4e3650,
    NOBE = 0x4e4f4245,
    NOBJ = 0x4e4f424a,
    NOBP = 0x4e4f4250,
    NPOE = 0x4e504f45,
    NPOJ = 0x4e504f4a,
    NPOP = 0x4e504f50,
    NPWE = 0x4e505745,
    NPWJ = 0x4e50574a,
    NPWP = 0x4e505750,
    NKQJ = 0x4e514b45,
    NQKJ = 0x4e514b4a,
    NKQP = 0x4e514b50,
    NRBE = 0x4e524245,
    NRBJ = 0x4e52424a,
    NRBP = 0x4e524250,
    NRIE = 0x4e524945,
    NRIJ = 0x4e52494a,
    NRIP = 0x4e524950,
    NRXE = 0x4e525845,
    NRXJ = 0x4e52584a,
    NRXP = 0x4e525850,
    NSIJ = 0x4e53494a,
    NSME = 0x4e534d45,
    NSMJ = 0x4e534d4a,
    NSMP = 0x4e534d50,
    NSQE = 0x4e535145,
    NSQJ = 0x4e53514a,
    NSQP = 0x4e535150,
    NTEA = 0x4e544541,
    NTEJ = 0x4e54454a,
    NTEP = 0x4e544550,
    NTUE = 0x4e545545,
    NTUJ = 0x4e54554a,
    NTUP = 0x4e545550,
    NWRE = 0x4e575245,
    NWRJ = 0x4e57524a,
    NWRP = 0x4e575250,
    NYLE = 0x4e594c45,
    NYLJ = 0x4e594c4a,
    NYLP = 0x4e594c50,
    NYSE = 0x4e595345,
    NTSJ = 0x4e59534a,
    NYSP = 0x4e595350,
    NZLP = 0x4e5a4c50,
    NZSE = 0x4e5a5345,
    NZSJ = 0x4e5a534a,
    NZSP = 0x4e5a5350
} rom_id_t;

s32 romCopy(rom_class_t *rom, void* dst, u32 addr, s32 len, unk_rom_callback arg4);

#endif
