#ifndef _GLOBALDESTRUCTORCHAIN
#define _GLOBALDESTRUCTORCHAIN

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DestructorChain {
    struct DestructorChain* next;
    void* destructor;
    void* object;
} DestructorChain;

extern void* __register_global_object(void* object, void* destructor, void* registration);
extern void __destroy_global_chain(void);

#ifdef __cplusplus
}
#endif

#endif
