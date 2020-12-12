#include "types.h"

s32 xlTextLen(char *str) {
    s32 iVar1 = 0;

    while (str[iVar1] != '\x00') {
        iVar1++;
    }
    return iVar1;
}

s32 xlTextCopy(char *str1, char *str2) {
    s32 iVar2 = 0;

    while (str2[iVar2] != '\x00') {
        str1[iVar2] = str2[iVar2];
        iVar2++;
    }
    str1[iVar2] = '\x00';
    return iVar2;
}

s32 xlTextConcat(char *str1, char *str2) {
    s32 iVar3;
    s32 iVar4 = 0;

    while(str1[iVar4] != '\x00') {
        iVar4++;
    }
    iVar3 = 0;
    while(str2[iVar3] != '\x00') {
        str1[iVar4++] = str2[iVar3++];
    }
    str1[iVar4] = '\x00';
    return iVar3;
}
