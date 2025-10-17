#include "signal.h"

void (*__stdio_exit)(void);
bool __aborting;

void abort(void) {
    raise(1);
    __aborting = true;
    exit(1);
}
