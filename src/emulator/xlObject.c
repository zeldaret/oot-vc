#include "xlObject.h"
#include "xlList.h"

extern list_type_t* lbl_8025D1F8;

typedef struct {
    list_type_t* class_list;
    class_t* class;
} class_ent_t;

typedef struct class_item_s class_item_t;

struct class_item_s {
    class_item_t* next;
    class_ent_t ent;
};

#ifdef NON_MATCHING
// r3 being used instead of r4 in findClass (item_p)
inline s32 findClass(class_ent_t** ent, class_t* class) {
    class_item_t* item_p;
    for (item_p = (class_item_t*)lbl_8025D1F8->first; item_p != NULL; item_p = item_p->next) {
        *ent = &item_p->ent;
        if (item_p->ent.class == class) {
            return 1;
        }
    }
    return 0;
}

inline s32 newClass(class_ent_t** ent, class_t* class) {
    if (!xlListMakeItem(lbl_8025D1F8, (void**)ent)) {
        return 0;
    }

    (*ent)->class = class;

    return !!xlListMake((list_type_t**)*ent, class->size + 4);
}

s32 xlObjectMake(void** dst, void* arg, class_t* class) {
    s32 new_class;
    list_item_t* list_item;
    class_ent_t* ent;
    if (!findClass(&ent, class)) {
        if (!newClass(&ent, class)) {
            return 0;
        }
        new_class = 1;
    } else {
        new_class = 0;
    }

    if (!xlListMakeItem(ent->class_list, dst)) {
        return 0;
    }

    list_item = (list_item_t*)*dst;
    list_item->next = (list_item_t*)ent;
    *dst = list_item->data;
    memset(*dst, 0, class->size);

    if (new_class) {
        class->callback(*dst, 0, NULL);
    }

    return class->callback(*dst, 2, arg);
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/xlObject/xlObjectMake.s")
#endif

s32 xlObjectFree(void** obj) {
    if (obj != NULL && *obj != NULL) {
        class_ent_t* ent = *(class_ent_t**)((u8*)*obj - 4);

        ent->class->callback(*obj, 3, NULL);

        *obj = (void*)((u8*)*obj - 4);

        if (!xlListFreeItem(ent->class_list, obj)) {
            return 0;
        }

        *obj = NULL;
        return 1;
    }

    return 0;
}

s32 xlObjectTest(void* obj, class_t* class) {
    class_ent_t* ent;
    if (obj != NULL) {
        ent = *(class_ent_t**)((u8*)obj - 4);
        if (xlListTestItem(lbl_8025D1F8, ent) && ent->class == class) {
            return 1;
        }
    }
    return 0;
}

inline s32 testClass(void* obj, class_ent_t* ent2) {
    class_ent_t* ent;
    class_t* class2 = ent2->class;
    if (obj != NULL) {
        ent = *(class_ent_t**)((u8*)obj - 4);
        if (xlListTestItem(lbl_8025D1F8, ent)) {
            if (ent->class == class2) {
                return 1;
            }
        }
    }

    return 0;
}

s32 xlObjectEvent(void* obj, s32 event, void* arg) {
    if (obj != NULL) {
        class_ent_t* ent = *(class_ent_t**)((u8*)obj - 4);
        if (xlListTestItem(lbl_8025D1F8, ent)) {
            if (testClass(obj, ent)) {
                return ent->class->callback(obj, event, arg);
            }
        }
    }
    return 0;
}

s32 xlObjectSetup(void) {
    return !!xlListMake(&lbl_8025D1F8, 8);
}

s32 xlObjectReset(void) {
    list_item_t* item_p;
    for (item_p = lbl_8025D1F8->first; item_p != NULL; item_p = item_p->next) {
        if (!xlListFree((list_type_t**)item_p->data)) {
            return 0;
        }
    }

    return !!xlListFree(&lbl_8025D1F8);
}
