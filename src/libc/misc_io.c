#include "misc_io.h"
#include "ansi_files.h"

void __stdio_atexit(void) { __stdio_exit = __close_all; }
