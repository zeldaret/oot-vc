#include "macros.h"

typedef struct PTMF {
    long this_delta;
    long vtbl_offset;
    union {
        void* func_addr;
        long ventry_offset;
    } func_data;
} PTMF;

ASM long __ptmf_cmpr(register PTMF* ptmf1, register PTMF* ptmf2){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    lwz	    r5,PTMF.this_delta(r3)
    lwz	    r6,PTMF.this_delta(r4)
    lwz	    r7,PTMF.vtbl_offset(r3)
    lwz	    r8,PTMF.vtbl_offset(r4)
    lwz	    r9,PTMF.func_data(r3)
    lwz	    r10,PTMF.func_data(r4)
    li	    r3,1
    cmpw    cr0,r5,r6
    cmpw    cr6,r7,r8
    cmpw    cr7,r9,r10
    bnelr    cr0
    bnelr    cr6
    bnelr    cr7
    li	    r3,0
    blr
#endif // clang-format on
}

ASM void __ptmf_scall(...) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    lwz	    r0,PTMF.this_delta(r12)
    lwz	    r11,PTMF.vtbl_offset(r12)
    lwz	    r12,PTMF.func_data(r12)
    add	    r3,r3,r0
    cmpwi    cr0,r11,0
    blt	    cr0,@1
    lwzx    r12,r3,r12
    lwzx    r12,r12,r11

@1  mtctr    r12
    bctr
#endif // clang-format on
}
