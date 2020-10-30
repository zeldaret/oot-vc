#include "types.h"
#include "xlHeap.h"
#include "xlList.h"

inline s32 listAlloc(void **item, list_type_t *list) {
    list_item_t *new_item;
    list_item_t *item_p;
    if(!xlHeapTake((void**)&new_item, list->item_size + 4)) {
        return 0;
    }

    new_item->next = NULL;
    *item = (void*)new_item->data;
    item_p = (list_item_t*)&list->first;
    while(item_p != NULL) {
        if(item_p->next == NULL) {
            item_p->next = new_item;
            list->item_cnt++;
            return 1;
        }
        item_p = item_p->next;
    }
    return 0;
}

s32 xlListMake(list_type_t **list, s32 item_size) {
    item_size = (item_size + 3) & ~3;

    if(listAlloc((void**)list, &gListList)){
        (*list)->item_cnt = 0;
        (*list)->item_size = item_size;
        (*list)->last = NULL;
        (*list)->first = NULL;
        return 1;
    }
    return 0;
}

inline s32 listFree(list_type_t **list) {
    list_item_t *item_p;
    list_type_t *list_p = *list;
    list_item_t *tmp;

    tmp = item_p = list_p->first;
    while(item_p != NULL) {
        item_p = item_p->next;
        if(!xlHeapFree((void**)&tmp)){
            return 0;
        }

        tmp = item_p;
    }

    list_p->item_cnt = 0;
    list_p->last = NULL;
    list_p->first = NULL;
    return 1;
}

s32 rspFreeList(list_type_t **list) {
    if(!listFree(list)) {
        return 0;
    }
    return !!xlListFreeItem(&gListList, (void**)list);
}

s32 xlListMakeItem(list_type_t *list, void **item) {
    return listAlloc(item, list);
}

s32 xlListFreeItem(list_type_t *list, void **item) {
    list_item_t *item_p;
    list_item_t *tmp;

    if(list->first == NULL) {
        return 0;
    }

    item_p = (list_item_t*)&list->first;
    while(item_p != NULL) {
        tmp = item_p->next;
        if(*item == (void*)tmp->data) {
            item_p->next = tmp->next;
            *item = NULL;
            if(!xlHeapFree((void**)&tmp)) {
                return 0;
            }

            list->item_cnt--;
            return 1;
        }
        item_p = tmp;
    }
    return 0;
}

inline s32 findList(list_type_t *list) {
    list_item_t *list_p;
    if(list == &gListList){
        return 1;
    }
    list_p = gListList.first;
    while(list_p != NULL) {
        if(list == (list_type_t*)list_p->data) {
            return 1;
        }
        list_p = list_p->next;
    }
    return 0;
}

s32 xlListTestItem(list_type_t *list, void *item) {
    list_item_t *item_p;
    if(!findList(list) || item == NULL) {
        return 0;
    }

    item_p = list->first;
    while(item_p != NULL) {
        if(item == item_p->data) {
            return 1;
        }
        item_p = item_p->next;
    }
    return 0;
}

s32 xlListSetup(void) {
    gListList.item_cnt = 0;
    gListList.item_size = sizeof(list_type_t);
    gListList.last = NULL;
    gListList.first = NULL;
    return 1;
}

s32 xlListReset(void) {
    return 1;
}
