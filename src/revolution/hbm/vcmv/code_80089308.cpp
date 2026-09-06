
#include "macros.h"
#include "revolution.h"
#include "revolution/hbm/vcmv/vcmv.h"
#include "runtime/Gecko_setjmp.h"
#include "runtime/MWCPlusLib.h"

#include <bsearch.h>
#include <locale.h>
#include <math.h>
#include <printf.h>
#include <stdlib.h>
#include <string.h>
#include <strtoul.h>
#include <time.h>

extern "C" UNKWORD fn_801004A8(void*);
extern "C" UnkStruct_80100070* fn_80100070(void*, const char*);

extern "C" unsigned long __cvt_fp2unsigned(register double d);
extern "C" void __cvt_sll_dbl(void);
extern "C" void __div2i();
extern "C" void __mod2i();
extern "C" void __ptmf_scall(...);
extern "C" void __shl2i();
extern "C" void abort(void);
extern "C" int getenv(void);

const UnkStruct_80176360 lbl_8016B800[] = {
    {"ARCClose", (void*)ARCClose},
    {"ARCConvertPathToEntrynum", (void*)ARCConvertPathToEntrynum},
    {"ARCEntrynumIsDir", (void*)ARCEntrynumIsDir},
    {"ARCFastOpen", (void*)ARCFastOpen},
    {"ARCGetLength", (void*)ARCGetLength},
    {"ARCGetStartAddrInMem", (void*)ARCGetStartAddrInMem},
    {"ARCGetStartOffset", (void*)ARCGetStartOffset},
    {"ARCInitHandle", (void*)ARCInitHandle},
    {"ARCOpen", (void*)ARCOpen},
    {"ARCReadDir", (void*)ARCReadDir},
    {"DVDClose", (void*)DVDClose},
    {"DVDCloseDir", (void*)DVDCloseDir},
    {"DVDConvertPathToEntrynum", (void*)DVDConvertPathToEntrynum},
    {"DVDEntrynumIsDir", (void*)DVDEntrynumIsDir},
    {"DVDFastOpen", (void*)DVDFastOpen},
    {"DVDOpenDir", (void*)DVDOpenDir},
    {"DVDReadDir", (void*)DVDReadDir},
    {"DVDReadPrio", (void*)DVDReadPrio},
    {"NANDClose", (void*)NANDClose},
    {"NANDCreate", (void*)NANDCreate},
    {"NANDCreateDir", (void*)NANDCreateDir},
    {"NANDDelete", (void*)NANDDelete},
    {"NANDGetCurrentDir", (void*)NANDGetCurrentDir},
    {"NANDGetLength", (void*)NANDGetLength},
    {"NANDGetStatus", (void*)NANDGetStatus},
    {"NANDGetType", (void*)NANDGetType},
    {"NANDOpen", (void*)NANDOpen},
    {"NANDRead", (void*)NANDRead},
    {"NANDReadDir", (void*)NANDReadDir},
    {"NANDSafeClose", (void*)NANDSafeClose},
    {"NANDSafeOpen", (void*)NANDSafeOpen},
    {"NANDSeek", (void*)NANDSeek},
    {"NANDWrite", (void*)NANDWrite},
    {"NETGetUniversalCalendar", (void*)NETGetUniversalCalendar},
    {"OSCalendarTimeToTicks", (void*)OSCalendarTimeToTicks},
    {"OSGetTime", (void*)OSGetTime},
    {"OSPanic", (void*)OSPanic},
    {"OSRegisterVersion", (void*)OSRegisterVersion},
    {"OSReport", (void*)OSReport},
    {"OSYieldThread", (void*)OSYieldThread},
    {"SCGetLanguage", (void*)SCGetLanguage},
    {"WWW_FONT_FILE_DATA_TABLE__", (void*)WWW_FONT_FILE_DATA_TABLE__},
    {"contentCloseNAND", (void*)contentCloseNAND},
    {"contentConvertPathToEntrynumNAND", (void*)contentConvertPathToEntrynumNAND},
    {"contentFastOpenNAND", (void*)contentFastOpenNAND},
    {"contentGetLengthNAND", (void*)contentGetLengthNAND},
    {"contentInitHandleNAND", (void*)contentInitHandleNAND},
    {"contentOpenDirNAND", (void*)contentOpenDirNAND},
    {"contentReadNAND", (void*)contentReadNAND},
    {"contentReleaseHandleNAND", (void*)contentReleaseHandleNAND},
    {"contentSeekNAND", (void*)contentSeekNAND},
    {"__OSGetSystemTime", (void*)__OSGetSystemTime},
    {"_SDA2_BASE_", (void*)_SDA2_BASE_},
    {"_SDA_BASE_", (void*)_SDA_BASE_},
    {"__construct_array", (void*)__construct_array},
    {"__construct_new_array", (void*)__construct_new_array},
    {"__cvt_fp2unsigned", (void*)__cvt_fp2unsigned},
    {"__cvt_sll_dbl", (void*)__cvt_sll_dbl},
    {"__destroy_arr", (void*)__destroy_arr},
    {"__div2i", (void*)__div2i},
    {"__double_huge", (void*)__double_huge},
    {"__files", (void*)__files},
    {"__mod2i", (void*)__mod2i},
    {"__ptmf_scall", (void*)__ptmf_scall},
    {"__setjmp", (void*)__setjmp},
    {"__shl2i", (void*)__shl2i},
    {"__va_arg", (void*)__va_arg},
    {"_current_locale", (void*)&_current_locale},
    {"abort", (void*)abort},
    {"abs", (void*)abs},
    {"acos", (void*)acos},
    {"asin", (void*)asin},
    {"atan", (void*)atan},
    {"atan2", (void*)atan2},
    {"atoi", (void*)atoi},
    {"atol", (void*)atol},
    {"bsearch", (void*)bsearch},
    {"ceil", (void*)ceil},
    {"clock", (void*)clock},
    {"cos", (void*)cos},
    {"exit", (void*)exit},
    {"exp", (void*)exp},
    {"floor", (void*)floor},
    {"fmod", (void*)fmod},
    {"fprintf", (void*)fprintf},
    {"getenv", (void*)getenv},
    {"itoa", (void*)itoa},
    {"labs", (void*)labs},
    {"localtime", (void*)localtime},
    {"log", (void*)log},
    {"longjmp", (void*)longjmp},
    {"memchr", (void*)memchr},
    {"memcmp", (void*)memcmp},
    {"memcpy", (void*)memcpy},
    {"memmove", (void*)memmove},
    {"memset", (void*)memset},
    {"mktime", (void*)mktime},
    {"pow", (void*)pow},
    {"printf", (void*)printf},
    {"qsort", (void*)qsort},
    {"rand", (void*)rand},
    {"sin", (void*)sin},
    {"snprintf", (void*)snprintf},
    {"sprintf", (void*)sprintf},
    {"sqrt", (void*)sqrt},
    {"srand", (void*)srand},
    {"sscanf", (void*)sscanf},
    {"strcat", (void*)strcat},
    {"strchr", (void*)strchr},
    {"strcmp", (void*)strcmp},
    {"strcpy", (void*)strcpy},
    {"strcspn", (void*)strcspn},
    {"strftime", (void*)strftime},
    {"stricmp", (void*)stricmp},
    {"strlen", (void*)strlen},
    {"strncat", (void*)strncat},
    {"strncmp", (void*)strncmp},
    {"strncpy", (void*)strncpy},
    {"strnicmp", (void*)strnicmp},
    {"strpbrk", (void*)strpbrk},
    {"strrchr", (void*)strrchr},
    {"strspn", (void*)strspn},
    {"strstr", (void*)strstr},
    {"strtol", (void*)strtol},
    {"strtoul", (void*)strtoul},
    {"tan", (void*)tan},
    {"vprintf", (void*)vprintf},
    {"vsnprintf", (void*)vsnprintf},
    {"vsprintf", (void*)vsprintf},
    {"", nullptr},
};

FORCE_ACTIVE(code_80089308, "!!!!!Prolog!!!!!\n");
FORCE_ACTIVE(code_80089308, "RSO Module : call constructor (%08x)\n");
FORCE_ACTIVE(code_80089308, "!!!!!Epilog!!!!!\n");
FORCE_ACTIVE(code_80089308, "RSO Module : call destructor (%08x)\n");
FORCE_ACTIVE(code_80089308, "\n[Error]: Unlinked function was called.\n");

void VCMV_80089308(UnkStruct_80089308* param1) {
    UnkStruct_80089308* new_var;

    if (fn_801004A8(new_var = param1) == 0) {
        OSReport("RSOLinkInit ERROR!\n");
    }

    for (const UnkStruct_80176360* ptr = lbl_8016B800; ptr->unk_04 != 0; ptr++) {
        UnkStruct_80100070* temp_r3 = fn_80100070(new_var, ptr->unk_00);

        if (temp_r3 != 0) {
            temp_r3->unk_04 = ((uintptr_t)ptr->unk_04) - new_var->unk_0C[temp_r3->unk_08].unk_00;
        }
    }
}
