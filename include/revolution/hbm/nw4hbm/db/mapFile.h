#ifndef NW4R_DB_MAP_FILE_H
#define NW4R_DB_MAP_FILE_H

#include "revolution/types.h"

#include "revolution/os/OSLink.h"

namespace nw4hbm {
namespace db {

struct MapFile {
    /* 0x00 */ byte_t* mapBuf;
    /* 0x04 */ OSModuleInfo* moduleInfo;
    /* 0x08 */ s32 fileEntry;
    /* 0x0C */ MapFile* next;
}; // size = 0x10

} // namespace db
} // namespace nw4hbm

namespace nw4hbm {
namespace db {
bool MapFile_Exists();
bool MapFile_QuerySymbol(u32 address, u8* strBuf, u32 strBufSize);
} // namespace db
} // namespace nw4hbm

#endif
