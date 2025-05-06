#ifndef _METROTRK_TRK_GLUE_H
#define _METROTRK_TRK_GLUE_H

#include "metrotrk/UART.h"
#include "revolution/os/OSInterrupt.h"
#include "revolution/types.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HARDWARE_GDEV = 0,
    HARDWARE_NDEV = 1,
    HARDWARE_BBA = 2
} HardwareType;

typedef signed long OSInterrupt;

typedef int (*DBCommFunc)();
typedef int (*DBCommInitFunc)(void*, OSInterruptHandler);
typedef int (*DBCommReadFunc)(u8*, int);
typedef int (*DBCommWriteFunc)(const u8*, int);

typedef struct DBCommTable {
    DBCommInitFunc initialize_func;
    DBCommFunc initinterrupts_func;
    DBCommFunc shutdown_func;
    DBCommFunc peek_func;
    DBCommReadFunc read_func;
    DBCommWriteFunc write_func;
    DBCommFunc open_func;
    DBCommFunc close_func;
    DBCommFunc pre_continue_func;
    DBCommFunc post_stop_func;
} DBCommTable;

int InitMetroTRKCommTable(int);
void TRKUARTInterruptHandler();
UARTError TRK_InitializeIntDrivenUART(u32, u32, void*);
void EnableEXI2Interrupts();
int TRKPollUART();
UARTError TRKReadUARTN(void*, u32);
UARTError TRK_WriteUARTN(const void*, u32);
void ReserveEXI2Port(void);
void UnreserveEXI2Port(void);
void TRK_board_display(char*);
void InitializeProgramEndTrap();

#ifdef __cplusplus
}
#endif

#endif
