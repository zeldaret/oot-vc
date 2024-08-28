#ifndef _XL_OBJECT_H
#define _XL_OBJECT_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XL_OBJECTTYPE _XL_OBJECTTYPE;

typedef bool (*EventFunc)(void* pObject, s32 nEvent, void* pArgument);

struct _XL_OBJECTTYPE {
    /* 0x0 */ char* szName;
    /* 0x4 */ s32 nSizeObject;
    /* 0x8 */ _XL_OBJECTTYPE* pClassBase;
    /* 0xC */ EventFunc pfEvent;
}; // size = 0x10

typedef struct __anon_0x5062 {
    /* 0x0 */ struct tXL_LIST* pList;
    /* 0x4 */ _XL_OBJECTTYPE* pType;
} __anon_0x5062; // size = 0x8

bool xlObjectMake(void** ppObject, void* pArgument, _XL_OBJECTTYPE* pType);
bool xlObjectFree(void** ppObject);
bool xlObjectTest(void* pObject, _XL_OBJECTTYPE* pType);
bool xlObjectEvent(void* pObject, s32 nEvent, void* pArgument);
bool xlObjectSetup(void);
bool xlObjectReset(void);

#ifdef __cplusplus
}
#endif

#endif
