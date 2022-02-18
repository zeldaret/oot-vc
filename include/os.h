#ifndef _OS_H
#define _OS_H

#include "types.h"

u32 OSGetTick(void);
void OSSetArena1Lo(void* ptr);
void OSSetArena1Hi(void* ptr);
void OSSetArena2Lo(void* ptr);
void OSSetArena2Hi(void* ptr);
void* OSGetArena1Lo(void);
void* OSGetArena1Hi(void);
void* OSGetArena2Lo(void);
void* OSGetArena2Hi(void);

#endif
