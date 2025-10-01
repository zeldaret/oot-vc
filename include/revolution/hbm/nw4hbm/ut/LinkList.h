#ifndef NW4HBM_UT_LINK_LIST_H
#define NW4HBM_UT_LINK_LIST_H

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/ut/inlines.h"

#include "revolution/hbm/HBMAssert.hpp"
#include "stddef.h"

namespace nw4hbm {
namespace ut {
namespace detail {
class LinkListImpl;
}

/******************************************************************************
 *
 * Linked list node
 *
 ******************************************************************************/
class LinkListNode : private NonCopyable {
    friend class detail::LinkListImpl;

  public:
    LinkListNode() : mNext(nullptr), mPrev(nullptr) {}

    LinkListNode* GetNext() const { return mNext; }
    LinkListNode* GetPrev() const { return mPrev; }

  private:
    /* 0x00 */ LinkListNode* mNext;
    /* 0x04 */ LinkListNode* mPrev;
};

namespace detail {

/******************************************************************************
 *
 * Linked list implementation
 *
 ******************************************************************************/
class LinkListImpl : private NonCopyable {
  public:
    class ConstIterator;

    /******************************************************************************
     * Iterator implementation
     ******************************************************************************/
    class Iterator {
        friend class LinkListImpl;
        friend class ConstIterator;

      public:
        Iterator() : mPointer(nullptr) {}
        explicit Iterator(LinkListNode* pNode) : mPointer(pNode) {}

        Iterator& operator++() {
            mPointer = mPointer->GetNext();
            return *this;
        }

        Iterator& operator--() {
            mPointer = mPointer->GetPrev();
            return *this;
        }

        LinkListNode* operator->() const { return mPointer; }

        friend bool operator==(LinkListImpl::Iterator lhs, LinkListImpl::Iterator rhs) {
            return lhs.mPointer == rhs.mPointer;
        }

      private:
        /* 0x00 */ LinkListNode* mPointer;
    };

    /******************************************************************************
     * Iterator implementation (const-view)
     ******************************************************************************/
    class ConstIterator {
        friend class LinkListImpl;

      public:
        explicit ConstIterator(Iterator it) : mNode(it.mPointer) {}

        ConstIterator& operator++() {
            mNode = mNode->GetNext();
            return *this;
        }

        ConstIterator& operator--() {
            mNode = mNode->GetPrev();
            return *this;
        }

        const LinkListNode* operator->() const { return mNode; }

        friend bool operator==(LinkListImpl::ConstIterator lhs, LinkListImpl::ConstIterator rhs) {
            return lhs.mNode == rhs.mNode;
        }

      private:
        /* 0x00 */ LinkListNode* mNode;
    };

  protected:
    static Iterator GetIteratorFromPointer(LinkListNode* pNode) { return Iterator(pNode); }

    LinkListImpl() { Initialize_(); }
    ~LinkListImpl();

    Iterator GetBeginIter() { return Iterator(mNode.GetNext()); }
    Iterator GetEndIter() { return Iterator(&mNode); }

    Iterator Insert(Iterator it, LinkListNode* pNode);

    Iterator Erase(Iterator it);
    Iterator Erase(LinkListNode* pNode);
    Iterator Erase(Iterator begin, Iterator end);

  public:
    u32 GetSize() const { return mSize; }
    bool IsEmpty() const { return mSize == 0; }

    void PopFront() { Erase(GetBeginIter()); }
    void PopBack() { Erase(--GetEndIter()); }

    void Clear();
    void SetPrev(LinkListNode* p, LinkListNode* pPrev);
    void SetNext(LinkListNode* p, LinkListNode* pNext);

  private:
    void Initialize_() {
        mSize = 0;
        mNode.mNext = &mNode;
        mNode.mPrev = &mNode;
    }

  private:
    /* 0x00 */ u32 mSize;
    /* 0x04 */ LinkListNode mNode;
};

/******************************************************************************
 *
 * Reverse iterator
 *
 ******************************************************************************/
template <typename TIter> class ReverseIterator {
  public:
    explicit ReverseIterator(TIter it) : mCurrent(it) {}

    TIter GetBase() const { return mCurrent; }

    ReverseIterator& operator++() {
        --mCurrent;
        return *this;
    }

    const typename TIter::TElem* operator->() const { return &this->operator*(); }

    typename TIter::TElem& operator*() const {
        TIter it = mCurrent;
        return *--it;
    }

    friend bool operator==(const ReverseIterator& rLhs, const ReverseIterator& rRhs) {
        return rLhs.mCurrent == rRhs.mCurrent;
    }

    friend bool operator!=(const ReverseIterator& rLhs, const ReverseIterator& rRhs) {
        return !(rLhs.mCurrent == rRhs.mCurrent);
    }

  private:
    /* 0x00 */ TIter mCurrent;
};

} // namespace detail

/******************************************************************************
 *
 * Templated linked list
 *
 ******************************************************************************/
template <typename T, int Ofs> class LinkList : public detail::LinkListImpl {
  public:
    class ConstIterator;

    /******************************************************************************
     * Templated iterator
     ******************************************************************************/
    class Iterator {
        friend class LinkList;
        friend class ConstIterator;

      public:
        // Element type must be visible to ReverseIterator
        typedef T TElem;

      public:
        Iterator() : mIterator(nullptr) {}
        explicit Iterator(LinkListImpl::Iterator it) : mIterator(it) {}

        Iterator& operator++() {
            ++mIterator;
            return *this;
        }

        Iterator& operator--() {
            --mIterator;
            return *this;
        }

        Iterator operator++(int) {
            Iterator ret = *this;
            ++*this;
            return ret;
        }

        T* operator->() const { return GetPointerFromNode(mIterator.operator->()); }

        T& operator*() const {
            T* p = this->operator->();
            NW4HBMAssertPointerNonnull_Line(p, 403);
            return *p;
        }

        friend bool operator==(Iterator lhs, Iterator rhs) { return lhs.mIterator == rhs.mIterator; }

        friend bool operator!=(Iterator lhs, Iterator rhs) { return !(lhs == rhs); }

      private:
        /* 0x00 */ LinkListImpl::Iterator mIterator;
    };

    /******************************************************************************
     * Templated iterator (const-view)
     ******************************************************************************/
    class ConstIterator {
        friend class LinkList;

      public:
        // Element type must be visible to ReverseIterator
        typedef T TElem;

      public:
        explicit ConstIterator(LinkListImpl::Iterator it) : mIterator(it) {}
        explicit ConstIterator(Iterator it) : mIterator(it.mIterator) {}

        ConstIterator& operator++() {
            ++mIterator;
            return *this;
        }

        ConstIterator& operator--() {
            --mIterator;
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator ret = *this;
            ++*this;
            return ret;
        }

        const T* operator->() const { return GetPointerFromNode(mIterator.operator->()); }

        const T& operator*() const {
            const T* p = this->operator->();
            NW4HBMAssertPointerNonnull_Line(p, 447);
            return *p;
        }

        friend bool operator==(ConstIterator lhs, ConstIterator rhs) { return lhs.mIterator == rhs.mIterator; }

        friend bool operator!=(ConstIterator lhs, ConstIterator rhs) { return !(lhs == rhs); }

      private:
        /* 0x00 */ LinkListImpl::ConstIterator mIterator;
    };

  public:
    // Shorthand names for reverse iterator types
    typedef detail::ReverseIterator<Iterator> RevIterator;
    typedef detail::ReverseIterator<ConstIterator> ConstRevIterator;

  public:
    LinkList() {}

    Iterator GetBeginIter() { return Iterator(LinkListImpl::GetBeginIter()); }
    ConstIterator GetBeginIter() const { return ConstIterator(const_cast<LinkList*>(this)->GetBeginIter()); }
    RevIterator GetBeginReverseIter() { return RevIterator(GetBeginIter()); }
    ConstRevIterator GetBeginReverseIter() const { return ConstRevIterator(GetBeginIter()); }

    Iterator GetEndIter() { return Iterator(LinkListImpl::GetEndIter()); }
    ConstIterator GetEndIter() const { return ConstIterator(const_cast<LinkList*>(this)->GetEndIter()); }
    RevIterator GetEndReverseIter() { return RevIterator(GetEndIter()); }
    ConstRevIterator GetEndReverseIter() const { return ConstRevIterator(GetEndIter()); }

    Iterator Insert(Iterator it, T* pElem) {
        return Iterator(LinkListImpl::Insert(it.mIterator, GetNodeFromPointer(pElem)));
    }

    Iterator Erase(T* pElem) { return Iterator(LinkListImpl::Erase(GetNodeFromPointer(pElem))); }
    Iterator Erase(Iterator it) { return Iterator(LinkListImpl::Erase(it.mIterator)); }

    void PushBack(T* pElem) { Insert(GetEndIter(), pElem); }

    T& GetFront() {
        NW4HBMAssert_Line(!IsEmpty(), 497);
        return *GetBeginIter();
    }
    const T& GetFront() const {
        NW4HBMAssert_Line(!IsEmpty(), 502);
        return *GetBeginIter();
    }

    T& GetBack() {
        NW4HBMAssert_Line(!IsEmpty(), 507);
        return *--GetEndIter();
    }
    const T& GetBack() const {
        NW4HBMAssert_Line(!IsEmpty(), 512); // assumed line number
        return *--GetEndIter();
    }

    static Iterator GetIteratorFromPointer(T* pElem) { return GetIteratorFromPointer(GetNodeFromPointer(pElem)); }

    static Iterator GetIteratorFromPointer(LinkListNode* pNode) {
        return Iterator(LinkListImpl::GetIteratorFromPointer(pNode));
    }

    static LinkListNode* GetNodeFromPointer(T* p) {
        NW4HBMAssertPointerNonnull_Line(p, 563);
        return reinterpret_cast<LinkListNode*>(reinterpret_cast<char*>(p) + Ofs);
    }

    static T* GetPointerFromNode(LinkListNode* p) {
        NW4HBMAssertPointerNonnull_Line(p, 573);
        return reinterpret_cast<T*>(reinterpret_cast<char*>(p) - Ofs);
    }

    static const T* GetPointerFromNode(const LinkListNode* p) {
        NW4HBMAssertPointerNonnull_Line(p, 578);
        return reinterpret_cast<const T*>(reinterpret_cast<const char*>(p) - Ofs);
    }
};
} // namespace ut
} // namespace nw4hbm

#endif
