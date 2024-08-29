#ifndef _RVL_SDK_MEM_LIST_H
#define _RVL_SDK_MEM_LIST_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MEMList {
    /* 0x0 */ void* head;
    /* 0x4 */ void* tail;
    /* 0x8 */ u16 length;
    /* 0xA */ u16 offset;
} MEMList;

typedef struct MEMLink {
    /* 0x0 */ void* prev;
    /* 0x4 */ void* next;
} MEMLink;

void MEMInitList(MEMList* list, u16 offset);
void MEMAppendListObject(MEMList* list, void* object);
void MEMRemoveListObject(MEMList* list, void* object);
void* MEMGetNextListObject(MEMList* list, void* object);

#ifdef __cplusplus
}
#endif

#endif
