#ifndef _XL_LIST_H
#define _XL_LIST_H

#include "types.h"

typedef struct list_item_s list_item_t;

struct list_item_s{
    list_item_t *next;
    char data[];
};

typedef struct {
    size_t item_size;
    s32 item_cnt;
    list_item_t *first;
    list_item_t *last;
} list_type_t;

s32 xlListMake(list_type_t **list, s32 item_size);
s32 rspFreeList(list_type_t **list);
s32 xlListMakeItem(list_type_t *list, void **item);
s32 xlListFreeItem(list_type_t *list, void **item);
s32 xlListTestItem(list_type_t *list, void *item);
s32 xlListSetup(void);
s32 xlListReset(void);

extern list_type_t gListList;

#endif