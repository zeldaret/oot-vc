#ifndef RVL_SDK_HBM_NW4HBM_UT_LIST_HPP
#define RVL_SDK_HBM_NW4HBM_UT_LIST_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace ut {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4779de
struct List {
    void* headObject; // size 0x04, offset 0x00
    void* tailObject; // size 0x04, offset 0x04
    u16 numObjects; // size 0x02, offset 0x08
    u16 offset; // size 0x02, offset 0x0a
}; // size 0x0c

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x477aea
struct Link {
    void* prevObject; // size 0x04, offset 0x00
    void* nextObject; // size 0x04, offset 0x04
}; // size 0x08

void List_Init(List* list, u16 offset);
void List_Append(List* list, void* object);
void List_Remove(List* list, void* object);
void* List_GetNext(const List* list, const void* object);
void* List_GetNth(const List* list, u16 index);

inline void* List_GetFirst(const List* list) { return List_GetNext(list, nullptr); }

inline u16 List_GetSize(const List* list) { return list->numObjects; }

inline const void* List_GetNthConst(const List* list, const u16 index) { return List_GetNth(list, index); }
} // namespace ut
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_UT_LIST_HPP
