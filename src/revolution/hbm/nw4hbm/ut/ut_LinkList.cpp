#include "revolution/hbm/nw4hbm/ut/ut_LinkList.hpp"

/*******************************************************************************
 * headers
 */

#include "macros.h"
#include "revolution/types.h"

//! TODO: remove once matched
extern "C" void fn_8010CB20(char*, int, ...);
extern "C" void fn_8010CBAC(char*, int, ...);

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace ut {
namespace detail {

LinkListImpl::~LinkListImpl() { Clear(); }

LinkListImpl::Iterator LinkListImpl::Erase(Iterator it) {
    NW4HBM_ASSERT(it.mPointer != &mNode, 0);
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
    NW4HBM_ASSERT_PTR_NULL(p, 0);
    LinkListNode* pIt = it.mPointer;
    NW4HBM_ASSERT_PTR_NULL(pIt, 0);
    LinkListNode* pItPrev = pIt->mPrev;
    NW4HBM_ASSERT_PTR_NULL(pItPrev, 0);

    NW4HBM_PANIC(p->mPrev == nullptr, 0, "");
    NW4HBM_PANIC(p->mNext == nullptr, 0, "");
    p->mNext = pIt;
    p->mPrev = pItPrev;

    pIt->mPrev = p;
    pItPrev->mNext = p;

    mSize++;

    return p;
}

void LinkListImpl::SetPrev(LinkListNode* p, LinkListNode* pPrev) {
    NW4HBM_ASSERT_PTR_NULL(pPrev, 101);
    LinkListNode* pNode = p->mPrev;
    pNode->mPrev = pPrev;
}

void LinkListImpl::SetNext(LinkListNode* p, LinkListNode* pNext) {
    NW4HBM_ASSERT_PTR_NULL(pNext, 103);
    LinkListNode* pNode = p->mNext;
    pNode->mNext = pNext;
}

LinkListImpl::Iterator LinkListImpl::Erase(LinkListNode* p) {
    // clang-format off
    NW4HBM_ASSERT(!IsEmpty(), 96);
    NW4HBM_ASSERT_PTR_NULL(p, 97);
    NW4HBM_ASSERT2(p!=&mNode, 98);
    // clang-format on

    LinkListNode *pNext = p->mNext, *pPrev = p->mPrev;

    SetNext(p, pNext);
    SetPrev(p, pPrev);

    mSize--;

    p->mNext = nullptr;
    p->mPrev = nullptr;

    return pNext;
}

} // namespace detail
} // namespace ut
} // namespace nw4hbm
