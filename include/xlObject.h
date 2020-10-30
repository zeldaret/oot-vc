#ifndef _XL_OBJECT_H
#define _XL_OBJECT_H

#include "types.h"

typedef s32 event_t;

typedef s32 (*event_cb_t)(void *obj, event_t event, void *arg);

typedef struct {
    /* 0x0000 */ const char *name;
    /* 0x0004 */ size_t size;
    /* 0x0008 */ u32 unk_0x08;
    /* 0x000C */ event_cb_t callback;
} class_t; // size = 0x10
