#ifndef _VERSIONS_H
#define _VERSIONS_H

// The VERSION macro will be set to one of these version numbers.
#define SM64_J 1
#define SM64_U 2
#define SM64_E 3
#define MK64_J 4
#define MK64_U 5
#define MK64_E 6
#define OOT_J 7
#define OOT_U 8
#define OOT_E 9

#define REGION_JP (VERSION == SM64_J || VERSION == MK64_J || VERSION == OOT_J)
#define REGION_US (VERSION == SM64_U || VERSION == MK64_U || VERSION == OOT_U)
#define REGION_EU (VERSION == SM64_E || VERSION == MK64_E || VERSION == OOT_E)

#if VERSION == SM64_J || VERSION == SM64_U
#define HBM_REVISION 1
#else
#define HBM_REVISION 2
#endif

#endif
