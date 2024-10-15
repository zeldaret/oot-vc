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

bool xlFileLoad(char* szFileName, void** pTarget) {
    s32 pnSize;
    tXL_FILE* pFile;

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

        if (!xlFileClose(&pFile)) {
            return false;
        }

        return true;
    }

    return false;
}
