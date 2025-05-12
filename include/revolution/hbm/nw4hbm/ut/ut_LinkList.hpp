#ifndef RVL_SDK_HBM_NW4HBM_UT_LINK_LIST_HPP
#define RVL_SDK_HBM_NW4HBM_UT_LINK_LIST_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp" // NonCopyable
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace ut {
// forward declarations
namespace detail {
class LinkListImpl;
}

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x475f13
class LinkListNode : private NonCopyable {
    // methods
  public:
    // cdtors
    LinkListNode() : mNext(nullptr), mPrev(nullptr) {}

    // methods
    LinkListNode* GetNext() const { return mNext; }

    // members
  private:
    /* base NonCopyable */ // size 0x00, offset 0x00
    LinkListNode* mNext; // size 0x04, offset 0x00
    LinkListNode* mPrev; // size 0x04, offset 0x04

    // friends
  private:
    friend class detail::LinkListImpl;
}; // size 0x08

namespace detail {
namespace dummy {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x476198
class Iterator {}; // what does this base do?
} // namespace dummy

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x475f8e
class LinkListImpl : private NonCopyable {
    // nested types
  public:
    // [SGLEA4]/GormitiDebug.elf:.debug_info::0x476169
    class Iterator : public dummy::Iterator {
        // methods
      public:
        // cdtors
        Iterator(LinkListNode* pNode) : mPointer(pNode) {}

        // operators
        friend bool operator==(Iterator it1, Iterator it2) { return it1.mPointer == it2.mPointer; }

        Iterator& operator++() {
            mPointer = mPointer->GetNext();

            return *this;
        }

        LinkListNode* operator->() const { return mPointer; }

        // members
      private:
        /* base dummy::Iterator */ // size 0x00, offset 0x00
        LinkListNode* mPointer; // size 0x04, offset 0x00

        // friends
      private:
        friend class LinkListImpl;
    }; // size 0x04

    // methods
  public:
    // cdtors
    LinkListImpl() : mNode() { Initialize_(); }
    ~LinkListImpl();

    // methods
    Iterator GetBeginIter() { return mNode.GetNext(); }
    Iterator GetEndIter() { return &mNode; }

    Iterator Insert(Iterator it, LinkListNode* p);

    Iterator Erase(LinkListNode* p);
    Iterator Erase(Iterator it);
    Iterator Erase(Iterator itFirst, Iterator itLast);

    void Clear();
    bool IsEmpty() { return mSize != 0; };
    void SetPrev(LinkListNode* p, LinkListNode *pPrev);
    void SetNext(LinkListNode* p, LinkListNode *pNext);

  private:
    void Initialize_() {
        mSize = 0;
        mNode.mNext = &mNode;
        mNode.mPrev = &mNode;
    }

    // members
  private:
    u32 mSize; // size 0x04, offset 0x00
    LinkListNode mNode; // size 0x08, offset 0x04
}; // size 0x0c
} // namespace detail

namespace dummy {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x476159, 0x477fde...
template <typename, int> class Iterator {}; // same thing here idk
} // namespace dummy

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x475f72, 0x4763e9...
template <typename T, int I> class LinkList : private detail::LinkListImpl {
    // nested types
  public:
    // [SGLEA4]/GormitiDebug.elf:.debug_info::0x47612f, 0x477fb4...
    class Iterator : public dummy::Iterator<T, I> {
        // methods
      public:
        // cdtors
        Iterator(detail::LinkListImpl::Iterator it) : it_(it) {}

        // operators
        friend bool operator==(Iterator it1, Iterator it2) { return it1.it_ == it2.it_; }

        friend bool operator!=(Iterator it1, Iterator it2) { return !(it1 == it2); }

        Iterator& operator++() {
            ++it_;

            return *this;
        }

        Iterator operator++(int) {
            Iterator it = *this;
            ++*this;

            return it;
        }

        T& operator*() const {
            T* p = this->operator->();

            return *p;
        }

        T* operator->() const { return GetPointerFromNode(it_.operator->()); }

        // members
      private:
        /* base dummy::Iterator */ // size 0x00, offset 0x00
        detail::LinkListImpl::Iterator it_; // size 0x04, offset 0x00

        // friends
      private:
        friend class LinkList<T, I>;
    }; // size 0x04

    // methods
  public:
    // cdtors
    LinkList() {}

    // methods
    Iterator GetBeginIter() { return LinkListImpl::GetBeginIter(); }
    Iterator GetEndIter() { return LinkListImpl::GetEndIter(); }

    Iterator Insert(Iterator it, T* p) { return LinkListImpl::Insert(it.it_, GetNodeFromPointer(p)); }

    void PushBack(T* p) { Insert(GetEndIter(), p); }
    Iterator Erase(Iterator it) { return LinkListImpl::Erase(it.it_); }

    // static methods
    static LinkListNode* GetNodeFromPointer(T* ptr) {
        return reinterpret_cast<LinkListNode*>(reinterpret_cast<int>(ptr) + I);
    }

    static T* GetPointerFromNode(LinkListNode* ptr) { return reinterpret_cast<T*>(reinterpret_cast<int>(ptr) - I); }

    // members
  private:
    /* base LinkListImpl */ // size 0x0c, offset 0x00
}; // size 0x0c
} // namespace ut
} // namespace nw4hbm

/**
 * Declare typedef for linked-list specialization.
 */
#define NW4R_UT_LINKLIST_TYPEDEF_DECL(T) typedef nw4hbm::ut::LinkList<T, offsetof(T, node)> T##List;

/**
 * Declare typedef for linked-list specialization.
 *
 * Use the specified link node (name suffix) for classes with multiple nodes.
 */
#define NW4R_UT_LINKLIST_TYPEDEF_DECL_EX(T, SUFFIX) \
    typedef nw4hbm::ut::LinkList<T, offsetof(T, node##SUFFIX)> T##SUFFIX##List;

/**
 * Declare a member LinkListNode for use with the typedef.
 */
#define NW4R_UT_LINKLIST_NODE_DECL() nw4hbm::ut::LinkListNode node

/**
 * Declare a member LinkListNode for use with the typedef.
 *
 * Use the specified link node (name suffix) for classes with multiple nodes.
 */
#define NW4R_UT_LINKLIST_NODE_DECL_EX(SUFFIX) nw4hbm::ut::LinkListNode node##SUFFIX

/**
 * Explicitly instantiate a linked list specialization.
 * (RESERVED FOR MATCHING DECOMP HACKS)
 */
#ifndef __DECOMP_NON_MATCHING
#define NW4R_UT_LINKLIST_TYPEDEF_FORCE(T) template struct nw4hbm::ut::LinkList<T, offsetof(T, node)>
#else
#define NW4R_UT_LINKLIST_TYPEDEF_FORCE(T)
#endif

/**
 * Linked-list for-each macro.
 *
 * @param NAME Element name
 * @param LIST Reference to list
 * @param ... Statement(s) to execute
        NW4HBM_ASSERT_PTR_NULL(NAME, 573);                                     \
 */
#define NW4R_UT_LINKLIST_FOREACH(NAME, LIST, ...)                                          \
    {                                                                                      \
        typedef DECLTYPE((LIST).GetBeginIter()) IterType;                                  \
        for (IterType NAME = (LIST).GetBeginIter(); NAME != (LIST).GetEndIter(); ++NAME) { \
                                                                                           \
            __VA_ARGS__;                                                                   \
        }                                                                                  \
    }

/**
 * List for-each macro, with robust iteration.
 *
 * @param NAME Element name
 * @param LIST Reference to list
 * @param ... Statement(s) to execute
 */
#define NW4R_UT_LINKLIST_FOREACH_SAFE(NAME, LIST, ...)                                      \
    {                                                                                       \
        typedef DECLTYPE((LIST).GetBeginIter()) IterType;                                   \
                                                                                            \
        for (IterType __impl__ = (LIST).GetBeginIter(); __impl__ != (LIST).GetEndIter();) { \
                                                                                            \
            IterType NAME = __impl__++;                                                     \
            __VA_ARGS__;                                                                    \
        }                                                                                   \
    }

#endif // RVL_SDK_HBM_NW4HBM_UT_LINK_LIST_HPP
