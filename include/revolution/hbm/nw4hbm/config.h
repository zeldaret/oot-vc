#ifndef NW4HBM_CONFIG_H
#define NW4HBM_CONFIG_H

#include "revolution/hbm/nw4hbm/lyt/types.h"

#define NW4HBM_BYTEORDER_BIG 0xFEFF
#define NW4HBM_BYTEORDER_LITTLE 0xFFFE

#ifdef NW4HBM_LITTLE_ENDIAN
#define NW4HBM_BYTEORDER_NATIVE NW4HBM_BYTEORDER_LITTLE
#else
#define NW4HBM_BYTEORDER_NATIVE NW4HBM_BYTEORDER_BIG
#endif

#define NW4HBM_VERSION(major, minor) ((major & 0xFF) << 8 | minor & 0xFF)

#endif // NW4HBM_CONFIG_H
