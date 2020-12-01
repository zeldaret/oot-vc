#ifndef _XL_OBJECT_H
#define _XL_OBJECT_H

#include "class.h"

s32 xlObjectEvent(void *obj, s32 event, void *arg);
s32 xlObjectFree(void **obj);
s32 xlObjectMake(void **dst, void *arg, class_t *class);
s32 xlObjectReset(void);
s32 xlObjectSetup(void);
s32 xlObjectTest(void *obj, class_t *class);

#endif
