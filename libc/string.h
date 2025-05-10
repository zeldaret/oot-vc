#ifndef _STRING_H
#define _STRING_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int stricmp(const char*, const char*);

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t num);

char* strcat(char*, const char*);
char* strncat(char* dest, const char* src, size_t n);

int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

char* strchr(const char*, int);
char* strstr(const char* str, const char* substr);

size_t strlen(const char* str);

void* __memrchr(const void* src, int val, size_t n);

void* memmove(void*, const void*, size_t);
int memcmp(const void*, const void*, size_t);
void* memchr(const void*, int, size_t);

void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, int val, size_t count);

#ifdef __cplusplus
}
#endif

#endif
