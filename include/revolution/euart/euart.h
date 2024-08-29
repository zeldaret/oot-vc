#ifndef _RVL_SDK_EUART_EUART_H
#define _RVL_SDK_EUART_EUART_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EUART_ERROR_OK,
    EUART_ERROR_FATAL,
    EUART_ERROR_INVALID,
    EUART_ERROR_BUSY,
    EUART_ERROR_4,
    EUART_ERROR_EXI,
} EUARTError;

bool EUARTInit(void);
EUARTError InitializeUART(void);
EUARTError WriteUARTN(const char* msg, u32 n);

#ifdef __cplusplus
}
#endif

#endif
