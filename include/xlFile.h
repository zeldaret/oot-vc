#ifndef _XL_FILE_H
#define _XL_FILE_H

#include "class.h"

typedef struct {
    /* 0x000 */ u32 tag;
    /* 0x004 */ u32 rootnode_offset;
    /* 0x008 */ u32 header_size;
    /* 0x00C */ u32 data_offset;
    /* 0x010 */ char pad[0x8];
} u8_hdr_t; // size = 0x18

typedef struct {
    /* 0x000 */ u16 type;
    /* 0x002 */ u16 name_offset;
    /* 0x004 */ u32 data_offset;
    /* 0x008 */ size_t size;
} u8_node_t; // size = 0xC

typedef struct {
    /* 0x000 */ u8_hdr_t *hdr;
    /* 0x004 */ u8_node_t *nodes;
    /* 0x008 */ void *unk_8;
    /* 0x00C */ s32 node_cnt;
    /* 0x010 */ char *string_table;
    /* 0x014 */ s32 unk_20;
    /* 0x018 */ s32 data_offset;
    /* 0x01C */ s32 unk_28;
} u8_archive_t;

typedef struct {
    /* 0x000 */ u8_archive_t* archive;
    /* 0x004 */ u32 data_offset;
    /* 0x008 */ u32 size;
    /* 0x00C */ char unk_C[0x4];
} file_info_0x38_t; // size = 0x10

typedef struct {
    /* 0x000 */ class_ref_t common;
    /* 0x008 */ char unk_8[0x8];
    /* 0x010 */ s32 size;
    /* 0x014 */ s32 pos;
    /* 0x018 */ s32 unk_18;
    /* 0x01C */ char unk_1C[0xC];
    /* 0x028 */ file_info_0x38_t unk_28;
} file_class_t; // size = 0x38


#endif
