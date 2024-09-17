#include "revolution/ax.h"

const char* __AXVersion = "<< RVL_SDK - AX \trelease build: Oct  1 2006 03:26:07 (0x4200_60422) >>";

static bool __init = false;

void AXInit(void) {
    OSRegisterVersion(__AXVersion);
    __AXAllocInit();
    __AXVPBInit();
    __AXSPBInit();
    __AXAuxInit();
    __AXClInit();
    __AXOutInit(0);
}

void AXQuit(void) {
    __AXAllocQuit();
    __AXVPBQuit();
    __AXSPBQuit();
    __AXAuxQuit();
    __AXClQuit();
    __AXOutQuit();
}
