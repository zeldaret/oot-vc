#ifndef _XL_OBJECT_H
#define _XL_OBJECT_H

#include "class.h"

s32 xlObjectFree(void **obj);
s32 xlObjectMake(void **dst, void *arg, class_t *class);

#endif
