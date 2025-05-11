#ifndef _CONSOLE_IO_H
#define _CONSOLE_IO_H

#include "revolution/types.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

int __read_console(__file_handle handle, u8* buffer, size_t* count, __ref_con ref_con);
int __write_console(__file_handle handle, u8* buffer, size_t* count, __ref_con ref_con);
int __close_console(__file_handle handle);

#ifdef __cplusplus
}
#endif

#endif
