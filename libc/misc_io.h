#ifndef _MISC_IO_H
#define _MISC_IO_H

#ifdef __cplusplus
extern "C" {
#endif

extern void (*__stdio_exit)(void);

void __stdio_atexit();

#ifdef __cplusplus
}
#endif

#endif
