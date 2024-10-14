#include "macros.h"
#include "revolution/nand.h"
#include "revolution/os.h"
#include "string.h"

static OSStateFlags StateFlags ATTRIBUTE_ALIGN(32);

static u32 CheckSum(const OSStateFlags* state) {
    int i;
    const u32* ptr = (const u32*)&state->BYTE_0x4;
    u32 checksum = 0;

    for (i = 0; i < (sizeof(OSStateFlags) / sizeof(u32)) - 1; i++) {
        checksum += *ptr++;
    }

    return checksum;
}

bool __OSWriteStateFlags(const OSStateFlags* newState) {
    NANDFileInfo file;
    bool bResult;

    memcpy(&StateFlags, newState, sizeof(OSStateFlags));
    StateFlags.checksum = CheckSum(&StateFlags);

    if (NANDOpen("/title/00000001/00000002/data/state.dat", &file, NAND_ACCESS_WRITE) == NAND_RESULT_OK) {
        bResult = NANDWrite(&file, &StateFlags, sizeof(OSStateFlags));
        NANDClose(&file);

        if (bResult != NAND_RESULT_OK) {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

bool __OSReadStateFlags(OSStateFlags* state) {
    NANDFileInfo file;

    if (NANDOpen("/title/00000001/00000002/data/state.dat", &file, NAND_ACCESS_READ) == NAND_RESULT_OK) {

        s32 bytesRead = NANDRead(&file, &StateFlags, sizeof(OSStateFlags));
        NANDClose(&file);

        if (bytesRead != sizeof(OSStateFlags)) {
            NANDDelete("/title/00000001/00000002/data/state.dat");
            memset(state, 0, sizeof(OSStateFlags));
            return false;
        }
    } else {
        memset(state, 0, sizeof(OSStateFlags));
        return false;
    }

    if (CheckSum(&StateFlags) != StateFlags.checksum) {
        memset(state, 0, sizeof(OSStateFlags));
        return false;
    }

    memcpy(state, &StateFlags, sizeof(OSStateFlags));
    return true;
}
