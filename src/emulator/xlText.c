#include "emulator/xlText.h"

s32 xlTextGetLength(const char* szTextSource) {
    s32 nCount = 0;

    while (szTextSource[nCount] != '\0') {
        nCount++;
    }

    return nCount;
}

s32 xlTextCopy(char* acTextTarget, const char* szTextSource) {
    s32 iCharacter;

    for (iCharacter = 0; szTextSource[iCharacter] != '\0'; iCharacter++) {
        acTextTarget[iCharacter] = szTextSource[iCharacter];
    }

    acTextTarget[iCharacter] = '\0';
    return iCharacter;
}

s32 xlTextAppend(char* acTextTarget, const char* szTextSource) {
    s32 iSource;
    s32 iTarget;

    for (iTarget = 0; acTextTarget[iTarget] != '\0'; iTarget++) {}

    for (iSource = 0; szTextSource[iSource] != '\0';) {
        acTextTarget[iTarget++] = szTextSource[iSource++];
    }

    acTextTarget[iTarget] = '\0';
    return iSource;
}
