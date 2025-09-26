#ifndef _RVL_SDK_ESP_H
#define _RVL_SDK_ESP_H

#include "revolution/arc.h"
#include "revolution/ipc.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ES_IOCTLV_LAUNCH_TITLE = 8,
    ES_IOCTLV_OPEN_CONTENT_FILE = 9,
    ES_IOCTLV_READ_CONTENT_FILE = 10,
    ES_IOCTLV_CLOSE_CONTENT_FILE = 11,
    ES_IOCTLV_GET_NUM_TICKET_VIEWS = 18,
    ES_IOCTLV_GET_TICKET_VIEWS = 19,
    ES_IOCTLV_GET_DATA_DIR = 29,
    ES_IOCTLV_GET_TITLE_ID = 32,
    ES_IOCTLV_SEEK_CONTENT_FILE = 35,
} ESIoctl;

typedef struct ESPTicket {
    u8 dummy[0x2A4];
} ESPTicket;

typedef struct ESPTmd {
    u8 dummy[0x49E4];
} ESPTmd;

s32 ESP_ReadContentFile(s32 fd, void* dst, u32 len);
s32 ESP_SeekContentFile(s32 fd, s32 offset, s32 origin);

extern s32 __esFd;

#ifdef __cplusplus
}
#endif

#endif
