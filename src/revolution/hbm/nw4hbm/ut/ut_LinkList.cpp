#include "revolution/hbm/nw4hbm/ut/LinkList.h"

/*******************************************************************************
 * headers
 */

#include "macros.h"
#include "revolution.h"

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace ut {
namespace detail {

LinkListImpl::~LinkListImpl() { Clear(); }

LinkListImpl::Iterator LinkListImpl::Erase(Iterator it) {
    // clang-format off
    NW4HBMAssert_Line(it.mPointer!=&mNode, 31);
    // clang-format on

    Iterator itNext(it);
    ++itNext;

    return Erase(it, itNext);
}

LinkListImpl::Iterator LinkListImpl::Erase(LinkListImpl::Iterator itFirst, LinkListImpl::Iterator itLast) {
    LinkListNode *p = itFirst.mPointer, *pItLast = itLast.mPointer, *pNext;

    while (p != pItLast) {
        pNext = p->mNext;
        Erase(p);
        p = pNext;
    }

    return itLast;
}

void LinkListImpl::Clear() { Erase(GetBeginIter(), GetEndIter()); }

LinkListImpl::Iterator LinkListImpl::Insert(Iterator it, LinkListNode* p) {
    NW4HBMAssertPointerNonnull_Line(p, 74);

    LinkListNode* pIt = it.mPointer;
    NW4HBMAssertPointerNonnull_Line(pIt, 76);

    LinkListNode* pItPrev = pIt->mPrev;
    NW4HBMAssertPointerNonnull_Line(pItPrev, 79);

    NW4HBMAssert_Line(p->mNext == NULL, 81);
    NW4HBMAssert_Line(p->mPrev == NULL, 82);
    p->mNext = pIt;
    p->mPrev = pItPrev;

    pIt->mPrev = p;
    pItPrev->mNext = p;

    mSize++;

    return p;
}

LinkListImpl::Iterator LinkListImpl::Erase(LinkListNode* p) {
    // clang-format off
    NW4HBMAssert2_Line(!IsEmpty(), 96);
    NW4HBMAssertPointerNonnull_Line(p, 97);
    NW4HBMAssert_Line(p!=&mNode, 98);
    // clang-format on

    LinkListNode* pNext = p->mNext;
    LinkListNode* pPrev = p->mPrev;

    SetPrev(pPrev, pNext);
    SetNext(pPrev, pNext);

    mSize--;

    p->mNext = nullptr;
    p->mPrev = nullptr;

    return pNext;
}

void LinkListImpl::SetPrev(LinkListNode* pPrev, LinkListNode* pNext) {
    NW4HBMAssertPointerNonnull_Line(pNext, 101);
    pNext->mPrev = pPrev;
}

void LinkListImpl::SetNext(LinkListNode* pPrev, LinkListNode* pNext) {
    NW4HBMAssertPointerNonnull_Line(pPrev, 103);
    pPrev->mNext = pNext;
}

} // namespace detail
} // namespace ut
} // namespace nw4hbm
