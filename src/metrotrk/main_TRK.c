#include "metrotrk/main_TRK.h"
#include "metrotrk/mainloop.h"

static int TRK_mainError;

int TRK_main(void) {
    TRK_mainError = TRK_InitializeNub();

    if (!TRK_mainError) {
        TRK_NubWelcome();
        TRK_NubMainLoop();
    }

    TRK_mainError = TRK_TerminateNub();

    return TRK_mainError;
}
