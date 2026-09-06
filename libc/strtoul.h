#ifndef _STRTOUL_H
#define _STRTOUL_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned long __strtoul(int base, int max_width, int (*ReadProc)(void*, int, int), void* ReadProcArg,
                        int* chars_scanned, int* negative, int* overflow);
unsigned long long __strtoull(int base, int max_width, int (*ReadProc)(void*, int, int), void* ReadProcArg,
                              int* chars_scanned, int* negative, int* overflow);
unsigned long strtoul(const char* str, char** end, int base);
long strtol(const char* str, char** end, int base);
int atoi(const char* str);
long atol(const char* str);
char* itoa(int value, char* str, int base);

#ifdef __cplusplus
}
#endif

#endif
