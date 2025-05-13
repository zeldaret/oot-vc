#ifndef _RVL_SDK_OS_LINK_H
#define _RVL_SDK_OS_LINK_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef u32 OSModuleID;
typedef struct OSModuleLink OSModuleLink;
typedef struct OSModuleInfo OSModuleInfo;
typedef struct OSSectionInfo OSSectionInfo;

struct OSModuleLink {
    OSModuleInfo *next;
    OSModuleInfo *prev;
};

struct OSModuleInfo {
    OSModuleID id;         // unique identifier for the module
    OSModuleLink link;     // doubly linked list of modules
    u32 numSections;       // # of sections
    u32 sectionInfoOffset; // offset to section info table
    u32 nameOffset;        // offset to module name
    u32 nameSize;          // size of module name
    u32 version;           // version number
};

struct OSSectionInfo {
    u32 offset;
    u32 size;
};

#define OSGetSectionInfo(module) ((OSSectionInfo *)(((OSModuleInfo *)(module))->sectionInfoOffset))

bool OSLink(OSModuleInfo *newModule, void *bss);
bool OSLinkFixed(OSModuleInfo *newModule, void *bss);
bool OSUnlink(OSModuleInfo *module);
void OSSetStringTable(void *string_table);
void __OSModuleInit(void);
OSModuleInfo *OSSearchModule(void *ptr, u32 *section, u32 *offset);

#ifdef __cplusplus
}
#endif

#endif
