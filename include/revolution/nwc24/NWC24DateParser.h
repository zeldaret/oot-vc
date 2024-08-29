#ifndef _RVL_SDK_NWC24_DATE_PARSER_H
#define _RVL_SDK_NWC24_DATE_PARSER_H

#include "revolution/nwc24/NWC24Types.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

NWC24Err NWC24iIsValidDate(u16 year, u8 month, u8 day);

#ifdef __cplusplus
}
#endif

#endif
