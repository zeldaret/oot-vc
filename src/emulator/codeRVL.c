#include "emulator/codeRVL.h"

_XL_OBJECTTYPE gClassCode = {
    "Code",
    sizeof(Code),
    NULL,
    (EventFunc)codeEvent,
};

bool codeEvent(Code* pCode, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            break;
        case 0:
        case 1:
        case 3:
        case 5:
        case 6:
            break;
        default:
            return false;
    }

    return true;
}
