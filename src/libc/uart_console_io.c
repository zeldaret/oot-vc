#include "file_struc.h"
#include "revolution/os.h"
#include "revolution/types.h"
#include "stdio.h"
#include "uart.h"

static bool initialized;

int __TRK_write_console(__file_handle, u8*, size_t*, __ref_con);

bool __write_console(__file_handle handle, u8* buffer, size_t* count, __ref_con ref_con) {

    if ((OSGetConsoleType() & 0x20000000) == 0) {
        int r3_cond = 0;
        if (initialized == false) {
            r3_cond = InitializeUART(0xE100);
            ;
            if (r3_cond == 0) {
                initialized = true;
            }
        }
        if (r3_cond != 0) {
            return true;
        }
        if (WriteUARTN(buffer, *count) != 0) {
            *count = 0;
            return true;
        }
    }
    __TRK_write_console(handle, buffer, count, ref_con);
    return false;
}

int __close_console(__file_handle handle) { return 0; }
