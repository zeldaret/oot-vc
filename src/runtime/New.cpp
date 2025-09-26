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

typedef void (*new_handler)();
new_handler nhandler;

} // namespace std

// TODO: malloc/free calls below should not generate __unexpected() calls (wrong compiler?)

void* operator new(size_t size) throw(std::bad_alloc) {
    void* ptr;

    if (size == 0) {
        size = 4;
    }

    for (;;) {
        ptr = malloc(size);
        if (ptr != NULL) {
            break;
        }

        if (std::nhandler != NULL) {
            std::nhandler();
        } else {
            throw std::bad_alloc();
        }
    }

    return ptr;
}

void operator delete(void* ptr) throw() {
    if (ptr != NULL) {
        free(ptr);
    }
}
