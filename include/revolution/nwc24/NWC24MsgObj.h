#ifndef _RVL_SDK_NWC24_MSG_OBJ_H
#define _RVL_SDK_NWC24_MSG_OBJ_H

#include "revolution/nwc24/NWC24Types.h"
#include "revolution/nwc24/NWC24Utils.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NWC24_MSG_RECIPIENT_MAX 8
#define NWC24_MSG_ATTACHMENT_MAX 2

// Forward declarations
typedef struct RFLCharData;

typedef enum {
    NWC24_MSGTYPE_RVL_MENU_SHARED,

    NWC24_MSGTYPE_RVL,
    NWC24_MSGTYPE_RVL_APP = NWC24_MSGTYPE_RVL,

    NWC24_MSGTYPE_RVL_MENU,

    NWC24_MSGTYPE_RVL_HIDDEN,
    NWC24_MSGTYPE_RVL_APP_HIDDEN = NWC24_MSGTYPE_RVL_HIDDEN,

    NWC24_MSGTYPE_PUBLIC
} NWC24MsgType;

typedef struct NWC24MsgObj {
    /* 0x0 */ u32 id;
    /* 0x4 */ u32 flags;
    /* 0x8 */ u32 length;
    /* 0xC */ u32 appId;
    char UNK_0x10[0x4];
    /* 0x14 */ u32 tag;
    /* 0x18 */ u32 ledPattern;
    /* 0x20 */ u64 fromId;
    u32 WORD_0x28;
    u32 WORD_0x2C;
    NWC24Data DATA_0x30;
    NWC24Data DATA_0x38;
    /* 0x40 */ NWC24Data subject;
    /* 0x48 */ NWC24Data text;
    NWC24Data DATA_0x50;
    NWC24Data DATA_0x58;
    /* 0x60 */ NWC24Charset charset;
    /* 0x64 */ NWC24Encoding encoding;
    /* 0x68 */ NWC24Data attached[NWC24_MSG_ATTACHMENT_MAX];
    /* 0x78 */ u32 attachedSize[NWC24_MSG_ATTACHMENT_MAX];
    /* 0x80 */ NWC24MIMEType attachedType[NWC24_MSG_ATTACHMENT_MAX];
    /* 0x88 */ union {
        u64 toIds[NWC24_MSG_RECIPIENT_MAX];
        NWC24Data toAddrs[NWC24_MSG_RECIPIENT_MAX];
    };
    /* 0xC8 */ u8 numTo;
    /* 0xC9 */ u8 numAttached;
    /* 0xCA */ u16 groupId;
    /* 0xCC */ union {
        struct {
            u32 noreply : 1;
            u32 unknown : 7;
            u32 delay : 8;
            u32 regdate : 16;
        };

        u32 raw;
    } mb;
    NWC24Data DATA_0xD0;
    /* 0xD8 */ NWC24Data face;
    /* 0xE0 */ NWC24Data alt;
    char UNK_0xE8[0x100 - 0xE8];
} NWC24MsgObj;

NWC24Err NWC24InitMsgObj(NWC24MsgObj* msg, NWC24MsgType type);
NWC24Err NWC24SetMsgToId(NWC24MsgObj* msg, u64 id);
NWC24Err NWC24SetMsgText(NWC24MsgObj* msg, const char* text, u32 len, NWC24Charset charset, NWC24Encoding encoding);
NWC24Err NWC24SetMsgFaceData(NWC24MsgObj* msg, const struct RFLCharData* data);
NWC24Err NWC24SetMsgAltName(NWC24MsgObj* msg, const wchar_t* name, u32 len);
NWC24Err NWC24SetMsgMBNoReply(NWC24MsgObj* msg, bool enable);
NWC24Err NWC24SetMsgMBRegDate(NWC24MsgObj* msg, u16 year, u8 month, u8 day);

#ifdef __cplusplus
}
#endif

#endif
