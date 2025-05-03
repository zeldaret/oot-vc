#ifndef _METROTRK_TRK_GLUE_H
#define _METROTRK_TRK_GLUE_H

#include "metrotrk/UART.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

UARTError TRKWriteUARTN(const void* src, size_t n);

#ifdef __cplusplus
}
#endif

#endif
