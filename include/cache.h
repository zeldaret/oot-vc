#ifndef _CACHE_H
#define _CACHE_H

#include "types.h"

void DCStoreRange(void *, size_t);
void ICInvalidateRange(void *, size_t);

#endif