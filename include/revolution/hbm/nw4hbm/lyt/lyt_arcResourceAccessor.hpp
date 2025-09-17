#ifndef RVL_SDK_HBM_NW4HBM_LYT_ARC_RESOURCE_ACCESSOR_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_ARC_RESOURCE_ACCESSOR_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/arc/arc.h" // ARCHandle
#include "revolution/hbm/nw4hbm/lyt/lyt_resourceAccessor.hpp"
#include "revolution/hbm/nw4hbm/ut/LinkList.h"
#include "revolution/types.h" // u32

/*******************************************************************************
 * classes and functions
 */

// forward declarations
namespace nw4hbm {
namespace ut {
class Font;
}
} // namespace nw4hbm

namespace nw4hbm {
namespace lyt {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4885c3
class FontRefLink {
    // typedefs
  public:
    typedef ut::LinkList<FontRefLink, 0> LinkList;

    // methods
  public:
    // cdtors
    FontRefLink();

    // methods
    const char* GetFontName() const { return mFontName; }
    ut::Font* GetFont() const { return mpFont; }

    // members
  private:
    ut::LinkListNode mLink; // offset 0x00, size 0x08
    char mFontName[128]; // offset 0x08, size 0x80
    ut::Font* mpFont; // offset 0x88, size 0x04
}; // size 0x8c

/* [SPQE7T]/ISpyD.elf:.debug_info::0x119a3e
 * This is the struct info for nw4r::lyt::ArcResourceLink, but until shown
 * otherwise I am assuming it is similar enough to the nw4hbm version.
 */
class ArcResourceLink {
    // typedefs
  public:
    typedef ut::LinkList<ArcResourceLink, 0> LinkList;

    // methods
  public:
    // methods
    ARCHandle* GetArcHandle() { return &mArcHandle; }
    const char* GetResRootDir() const { return mResRootDir; }

    bool Set(void* archiveStart, const char* resourceRootDirectory);

    // members
  private:
    ut::LinkListNode mLink; // offset 0x00, size 0x08
    ARCHandle mArcHandle; // offset 0x08, size 0x1c
    char mResRootDir[128]; // offset 0x24, size 0x80
}; // size 0xa4

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x47f88a
class ArcResourceAccessor : public ResourceAccessor {
    // methods
  public:
    // cdtors
    ArcResourceAccessor();
    virtual ~ArcResourceAccessor() {}

    // virtual function ordering
    // vtable ResourceAccessor
    virtual void* GetResource(u32 resType, const char* name, u32* pSize);
    virtual ut::Font* GetFont(const char* name);

    // methods
    bool Attach(void* archiveStart, const char* resourceRootDirectory);

    bool IsAttached(void) { return this->mArcBuf != nullptr; }

    // members
  private:
    /* base ResourceAccessor */ // offset 0x00, size 0x04
    ARCHandle mArcHandle; // offset 0x04, size 0x1c
    void* mArcBuf; // offset 0x20, size 0x04
    FontRefLink::LinkList mFontList; // offset 0x24, size 0x0c
    char mResRootDir[128]; // offset 0x30, size 0x80
}; // size 0xb0

/* [SPQE7T]/ISpyD.elf:.debug_info::0x113e8b
 * This is the struct info for nw4r::lyt::MultiArcResourceAccessor, but
 * until shown otherwise I am assuming it is similar enough to the nw4hbm
 * version.
 */
class MultiArcResourceAccessor : public ResourceAccessor {
    // methods
  public:
    // cdtors
    MultiArcResourceAccessor();
    virtual ~MultiArcResourceAccessor();

    // virtual function ordering
    // vtable ResourceAccessor
    virtual void* GetResource(u32 resType, const char* name, u32* pSize);
    virtual ut::Font* GetFont(const char* name);
    ut::Font* dummy(const char* name);

    // methods
    void Attach(ArcResourceLink* pLink);

    // What
    void DetachAll() { reinterpret_cast<ut::detail::LinkListImpl*>(&mArcList)->Clear(); }

    // members
  private:
    /* base ResourceAccessor */ // offset 0x00, size 0x04
    ArcResourceLink::LinkList mArcList; // offset 0x04, size 0x08
    FontRefLink::LinkList mFontList; // offset 0x10, size 0x08
}; // size 0x1c

namespace detail {
ut::Font* FindFont(FontRefLink::LinkList* pFontRefList, const char* name);
} // namespace detail
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_ARC_RESOURCE_ACCESSOR_HPP
