#ifndef _FILE_IO_H
#define _FILE_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "revolution/types.h"
#include "stdio.h"

int fclose(FILE* file);
int fflush(FILE* file);

#ifdef __cplusplus
}
#endif

#endif
