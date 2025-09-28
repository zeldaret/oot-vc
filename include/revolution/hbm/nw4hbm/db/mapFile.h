#ifndef NW4R_DB_MAP_FILE_H
#define NW4R_DB_MAP_FILE_H

#include "revolution/types.h"

#include "revolution/os/OSLink.h" // OSModuleInfo

namespace nw4hbm {
namespace db {

struct MapFile {
    byte_t* mapBuf; // size 0x04, offset 0x00
    OSModuleInfo* moduleInfo; // size 0x04, offset 0x04
    s32 fileEntry; // size 0x04, offset 0x08
    MapFile* next; // size 0x04, offset 0x0c
}; // size 0x10
} // namespace db
} // namespace nw4hbm

namespace nw4hbm {
namespace db {
bool MapFile_Exists();
bool MapFile_QuerySymbol(u32 address, u8* strBuf, u32 strBufSize);
} // namespace db
} // namespace nw4hbm

#endif // NW4R_DB_MAP_FILE_H
