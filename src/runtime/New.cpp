#include "new.hpp"
#include "alloc.h"
#include "macros.h"
#include "runtime/exception.hpp"
#include "stddef.h"

namespace std {
class bad_alloc : public exception {
  public:
    virtual ~bad_alloc() {}
    virtual const char* what() const { return "bad_alloc"; }
};

struct nothrow_t {
    explicit nothrow_t() {}
};
extern const nothrow_t nothrow;

typedef void (*new_handler)();
new_handler set_new_handler(new_handler new_p) throw();
} // namespace std

typedef void (*Test)();
extern "C" void* malloc(size_t);
extern "C" void fn_80154E68(u32*, void*, void*);
extern "C" Test lbl_8025DCE0;
extern "C" u32* lbl_801A0060;
extern "C" u32 lbl_8016E39C;

void* operator new(size_t size, void* ptr) {
    void* new_ptr;

    try {
        while (new_ptr == NULL) {
            new_ptr = malloc(size);
        }
    } catch (std::bad_alloc e) { new_ptr = NULL; }

    return new_ptr;
}

void* operator new[](size_t count, void* p) {
    if (count > 0 && p != NULL) {
        delete p;
    }

    return p;
}

#pragma exceptions off

void operator delete(void* p) throw() {
    if (p != 0) {
        free(p);
    }
}

#pragma exceptions on

void operator delete[](void* p, size_t count) throw(std::bad_alloc) {}
