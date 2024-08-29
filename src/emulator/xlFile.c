#include "emulator/xlFile.h"
#include "emulator/xlFileRVL.h"
#include "emulator/xlHeap.h"

bool xlFileGetSize(s32* pnSize, char* szFileName) {
    tXL_FILE* pFile;

    if (xlFileOpen(&pFile, XLFT_BINARY, szFileName)) {
        if (pnSize != NULL) {
            *pnSize = pFile->nSize;
        }

        if (!xlFileClose(&pFile)) {
            return false;
        }

        return true;
    }

    return false;
}

// xlFileLoad?
bool xlFileReadAll(char* szFileName, void** pTarget) {
    s32 pnSize;
    tXL_FILE* pFile;
    s32 temp_r3;

    if (xlFileGetSize(&pnSize, szFileName)) {
        if (!xlHeapTake(pTarget, pnSize | 0x30000000)) {
            return false;
        }

        if (!xlFileOpen(&pFile, XLFT_BINARY, szFileName)) {
            return false;
        }

        if (!xlFileGet(pFile, *pTarget, pnSize)) {
            return false;
        }

        temp_r3 = xlFileClose(&pFile);
        return temp_r3 != 0;
    }

    return false;
}
