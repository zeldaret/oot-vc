#include "revolution/exi/exicommon.h"
#include "revolution/exi/exibios.h"

const u32 __EXIFreq = EXI_FREQ_16MHZ;

bool EXIWriteReg(EXIChannel chan, u32 dev, u32 cmd, const void* buf, s32 len) {
    bool error = false;
    u32 write_val;

    switch (len) {
        case 1:
            write_val = *(u8*)buf << 24;
            break;
        case 2:
            write_val = *(u16*)buf << 24 | (*(u16*)buf & 0xFF00) << 8;
            break;
        default:
            write_val = __EXISwap32(*(u32*)buf);
            break;
    }

    error |= !EXILock(chan, dev, NULL);
    if (error) {
        return false;
    }

    error |= !EXISelect(chan, dev, EXI_FREQ_16MHZ);
    if (error) {
        EXIUnlock(chan);
        return false;
    }

    error |= !EXIImm(chan, &cmd, sizeof(cmd), EXI_WRITE, NULL);
    error |= !EXISync(chan);
    error |= !EXIImm(chan, &write_val, sizeof(write_val), EXI_WRITE, NULL);
    error |= !EXISync(chan);
    error |= !EXIDeselect(chan);
    error |= !EXIUnlock(chan);

    return error == false;
}
