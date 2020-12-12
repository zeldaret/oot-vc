#include "xlFile.h"
#include "xlHeap.h"
#include "xlObject.h"

void *func_800B0DF0(void*, size_t, s32);

s32 func_800FEFF0(u32 content, u8_hdr_t **arg1, unk_file_struct *arg2);         // contentInitHandleNAND
u32 func_800FF1B0(u8_archive_t*, char *fn, file_info_0x38_t*);                  // ARCGetFile
s32 func_800FF28C(file_info_0x38_t*);                                           // contentGetLengthNAND
s32 func_800FF2FC(file_info_0x38_t *arg0, char *data, u32 size, u32 pos);       // contentReadNAND
s32 func_800FF42C(u8_archive_t*);                                               // contentReleaseHandleNAND
s32 func_800FF424(void*);                                                       // contentCloseNAND

extern class_t gClassFile;

extern u8_archive_t lbl_801C9680;
extern unk_file_struct lbl_801C96A8;

extern u32 (*gpfOpen)(char*, file_info_0x38_t*);
extern void (*gpfRead)(list_type_t*, char*, u32, u32, u32);

s32 xlFileSetOpen(void) {
    return 1;
}

s32 xlFileSetRead(void) {
    return 1;
}

s32 xlFileOpen(file_class_t **file, s32 arg1, char *fn) {
    s32 ret;
    u32 uVar1;

    if (!xlObjectMake((void**)file, NULL, &gClassFile)) {
        return 0;
    }

    if (gpfOpen != NULL) {
        uVar1 = (*gpfOpen)(fn, &(*file)->unk_28);
    } else {
        uVar1 = !func_800FF1B0(&lbl_801C9680, fn, &(*file)->unk_28);
    }
    if (uVar1 != 0) {
        (*file)->unk_18 = arg1;
        (*file)->size = func_800FF28C(&(*file)->unk_28);
        (*file)->common.ref_list = (list_type_t*)&(*file)->unk_28;
        return 1;
    } else {
        xlObjectFree((void**)file);
        return 0;
    }
}

s32 xlFileClose(file_class_t **file) {
    return !!xlObjectFree((void**)file);
}

s32 xlFileRead(file_class_t *file, char *data, s32 size) {
    s32 iVar1;
    u32 size_00;
    u32 pos;
    s32 len;

    if (file->pos + size > file->size) {
        size = file->size - file->pos;
    }
    if (size == 0) {
        *data = -1;
        return 0;
    }
    while (size > 0) {
        if ((file->unk_24 != -1) && (iVar1 = file->pos - file->unk_24, iVar1 < 0x1000)) {
            len = 0x1000 - iVar1;
            if (len > size) {
                len = size;
            }
            if (!xlHeapCopy(data, (void *)&file->unk_8[iVar1], len)) {
                return 0;
            }
            data += len;
            size -= len;
            file->pos += len;
        }
        if (size > 0) {
            if ((((u32)data % 0x20 == 0) && (pos = file->pos, (pos % 4) == 0)) && ((u32)size % 0x20 == 0)) {
                if (gpfRead != NULL) {
                    (*gpfRead)((file->common).ref_list, data, size, pos, 0);
                } else {
                    func_800FF2FC((file_info_0x38_t *)file->common.ref_list, data, size, pos);
                }
                file->pos += size;
                size = 0;
            } else {
                size_00 = 0x1000;
                pos = file->pos & ~3;
                iVar1 = file->size - pos;
                file->unk_24 = pos;
                if (iVar1 <= 0x1000) {
                    size_00 = (iVar1 + 0x1F) & ~0x1F; // align 0x20
                }
                if (gpfRead != NULL) {
                    (*gpfRead)((file->common).ref_list, file->unk_8, size_00, pos, 0);
                } else {
                    func_800FF2FC((file_info_0x38_t *)file->common.ref_list, file->unk_8, size_00, pos);
                }
            }
        }
    }
    return 1;
}

s32 func_8008039C(file_class_t *arg0, s32 arg1) {
    if (arg1 >= 0 && arg1 < arg0->size) {
        arg0->pos = arg1;
        return 1;
    }
    return 0;
}

s32 fileEvent(file_t *file, event_t event, void *arg) {
    s32 bVar2;
    void *pvVar1;
    void *local_18;

    switch (event) {
        case 0:
            bVar2 = xlHeapTake(&local_18, 0x70020000);
            if (bVar2 == 0) {
                bVar2 = 0;
            } else {
                pvVar1 = func_800B0DF0(local_18, 0x20000, 0);
                if (pvVar1 == NULL) {
                    bVar2 = 0;
                } else {
                    func_800B165C(&lbl_801C96A8, pvVar1, 4);
                    bVar2 = 1;
                }
            }
            if (!bVar2) {
                return 0;
            }
            func_800FEFF0(5, &lbl_801C9680.hdr, &lbl_801C96A8);
            break;
        case 1:
            func_800FF42C(&lbl_801C9680);
            break;
        case 2:
            file->unk_10 = NULL;
            file->unk_14 = NULL;
            file->unk_0 = NULL;
            file->unk_4 = NULL;
            file->unk_24 = 0xffffffff;
            if (!xlHeapTake(&file->unk_8, 0x30001000)) {
                return 0;
            }
            break;
        case 3:
            if (file->unk_4 != NULL && !xlHeapFree(&file->unk_4)) {
                return 0;
            }
            func_800FF424(&file->unk_28);
            if (!xlHeapFree(&file->unk_8)) {
                return 0;
            }
            break;
        case 4:
            break;
        default:
            return 0;
    }
    return 1;
}
