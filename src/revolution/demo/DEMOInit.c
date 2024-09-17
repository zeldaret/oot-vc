#include "macros.h"
#include "revolution/demo.h"
#include "revolution/mem.h"

u32 DemoUseMEMHeap = 0;
void* DemoFrameBuffer1;
void* DemoFrameBuffer2;
void* DemoCurrentBuffer;

MEMAllocator DemoAllocator1;
MEMAllocator DemoAllocator2;
u32 lbl_801E3380[12];
