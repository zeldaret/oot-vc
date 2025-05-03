#ifndef _MSL_RAND_H
#define _MSL_RAND_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int rand();
void srand(u32 seed);

#ifdef __cplusplus
};
#endif

#endif
