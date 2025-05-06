#ifndef METROTRK_MEM_TRK_H
#define METROTRK_MEM_TRK_H

#include "metrotrk/trk.h"
#include "revolution/types.h"

typedef struct memRange {
    u8* start;
    u8* end;
    bool readable;
    bool writeable;
} memRange;

const memRange gTRKMemMap[] = {{(u8*)0x00000000, (u8*)0xFFFFFFFF, true, true}};

#endif
