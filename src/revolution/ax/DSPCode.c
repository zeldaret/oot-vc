#include "macros.h"
#include "revolution/ax.h"

#include "axDspSlave.inc"

u16 axDspInitVector = 0x0010;
u16 axDspResumeVector = 0x0031;
u16 axDspSlaveLength = ARRAY_COUNT(axDspSlave);
