#include "xlFile.h"
#include "xlObject.h"

s32 xlFileSetOpen(void) {
    return 1;
}

s32 xlFileSetRead(void) {
    return 1;
}

extern file_class_t lbl_80175280; // gClassFile

typedef u32 callback(char*, file_info_0x38_t*);

extern callback lbl_8025D1B0;

extern u8_archive_t lbl_801C9680;

s32 func_800FF28C(file_info_0x38_t*);                               // contentGetLengthNAND
u32 func_800FF1B0(u8_archive_t*, char *fn, file_info_0x38_t*);      // ARCGetFile

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlFile/func_800800D4.s")
/* s32 xlFileOpen(file_class_t **file, s32 arg1, char *fn) {
    s32 ret;
    u32 uVar1;

    if (xlObjectMake((void**)file, NULL, (class_t*)&lbl_80175280) == 0) {
        ret = 0;
    } else {
        if (lbl_8025D1B0 == NULL) {
            uVar1 = func_800FF1B0(&lbl_801C9680,fn,&(*file)->unk_28) >> 5;
        }
        else {
            uVar1 = (*lbl_8025D1B0)(fn, &(*file)->unk_28);
        }
        if (uVar1 == 0) {
            xlObjectFree((void**)file);
            ret = 0;
        }
        else {
            (*file)->unk_18 = arg1;
            *(u8_archive_t **)&(*file)->size = func_800FF28C(&(*file)->unk_28);
            ret = 1;
            *(file_info_0x38_t **)&((*file)->common).ref_list = &(*file)->unk_28;
        }
    }
    return ret;
} */

// xlFileClose
s32 func_800801BC(file_class_t** file) {
    return !!xlObjectFree((void**)file);
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlFile/func_800801E8.s")

s32 func_8008039C(file_class_t* arg0, s32 arg1) {
    if (arg1 >= 0 && arg1 < arg0->size) {
        arg0->pos = arg1;
        return 1;
    }
    return 0;
}

#define OBJECT_INIT 2
#define OBJECT_INSERTED 0

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlFile/func_800803C4.s")
// fileEvent
/* s32 func_800803C4(void *file, event_t event, void *arg) {
    s32 bVar2;
    void *pvVar1;
    void *local_18;

    if (event == OBJECT_INIT) {
        *(undefined4 *)((int)file + 0x10) = 0;
        *(undefined4 *)((int)file + 0x14) = 0;
        *(undefined4 *)file = 0;
        *(undefined4 *)((int)file + 4) = 0;
        *(undefined4 *)((int)file + 0x24) = 0xffffffff;
        bVar2 = xlHeapTake((void **)((int)file + 8),0x30001000);
        if (bVar2 == false) {
            return false;
        }
    } else if (event < 2) {
        if (event == OBJECT_INSERTED) {
            bVar2 = xlHeapTake(&local_18,0x70020000);
            if (bVar2 == false) {
                bVar2 = false;
            } else {
                pvVar1 = FUN_800b0df0(local_18,0x20000,0);
                if (pvVar1 == (void *)0x0) {
                    bVar2 = false;
                } else {
                    FUN_800b165c(&__unk_file_struct_801c96a8,pvVar1,4);
                    bVar2 = true;
                }
            }
            if (!bVar2) {
                return false;
            }
            contentInitHandleNAND(5, (u8_hdr_t **)&u8_archive_t_801c9680, &__unk_file_struct_801c96a8);
        } else {
            if (event < 0) {
                return false;
            }
            contentReleaseHandleNAND(&u8_archive_t_801c9680);
        }
    } else if (event != 4) {
        if (3 < event) {
            return false;
        }
        if ((*(int *)((int)file + 4) != 0) &&
            (bVar2 = xlHeapFree((void **)((int)file + 4)), bVar2 == false)) {
            return false;
        }
        contentCloseNAND((int)file + 0x28);
        bVar2 = xlHeapFree((void **)((int)file + 8));
        if (bVar2 == false) {
            return false;
        }
    }
    return true;
} */

s32 func_80080534(char *str) {
    s32 iVar1 = 0;
    while (str[iVar1] != '\x00') {
        iVar1++;
    }
    return iVar1;
}

s32 func_80080558(char *str1, char *str2) {
    s32 iVar2 = 0;

    while (str2[iVar2] != '\x00') {
        str1[iVar2] = str2[iVar2];
        iVar2++;
    }
    str1[iVar2] = '\x00';
    return iVar2;
}

s32 func_80080590(char *str1, char *str2) {
    s32 iVar3;
    s32 iVar4;

    iVar4 = 0;
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
