/**
 * @file banner.c
 *
 * This file implements methods to create, write and delete a banner to the NAND.
 *
 * The banner has a title (usually the emulated game's name), a subtitle (usually an empty string) and an icon.
 */

#include "emulator/banner.h"
#include "emulator/errordisplay.h"
#include "emulator/xlFile.h"
#include "emulator/xlHeap.h"
#include "macros.h"
#include "mem_funcs.h"
#include "revolution/tpl.h"
#include "versions.h"

static u8 lbl_8025C888[] = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};
static u8 lbl_8025C890[] = {0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x04, 0x04};

static u8 sBannerBuffer[BANNER_SIZE];
static NANDBanner* sBanner;

static s32 fn_80063F30(const char* szFileName, u32 nLength);
static s32 fn_800640BC(const char* szFileName, u32 arg1, u8 arg2);
static NANDResult bannerNANDOpen(const char* szFileName, NANDFileInfo* info, u8 access, void* buffer, s32 len);
static bool bannerWrite(void) NO_INLINE;
static s32 bannerDelete(void) NO_INLINE;
static s32 bannerGetBufferSize(void) NO_INLINE;
static s32 fn_8006496C(void) NO_INLINE;

#pragma ppc_iro_level 0

static inline bool fn_80063F30_Inline(NANDResult result) {
    struct_80174988* var_r4;

    if (result == NAND_RESULT_OK) {
        return true;
    }

    for (var_r4 = lbl_80174988; var_r4->result != NAND_RESULT_OK; var_r4++) {
        if (var_r4->result == result) {
            if (var_r4->eStringIndex == ERROR_NULL) {
                return false;
            }
            errorDisplayShow(var_r4->eStringIndex);
            break;
        }
    }

    return true;
}

static s32 fn_80063F30(const char* szFileName, u32 nLength) {
    NANDFileInfo nandFileInfo;
    u32 length;
    s32 openResult;

    length = 0;
    openResult = NANDSafeOpen(szFileName, &nandFileInfo, 1, sBannerBuffer, sizeof(sBannerBuffer));

    switch (openResult) {
        case NAND_RESULT_OK:
            break;
        case NAND_RESULT_AUTHENTICATION:
        case NAND_RESULT_ECC_CRIT:
            return 1;
        case NAND_RESULT_NOEXISTS:
            return 2;
        default:
            fn_80063F30_Inline(openResult);
            break;
    }

    fn_80063F30_Inline(NANDGetLength(&nandFileInfo, &length));
    fn_80063F30_Inline(NANDSafeClose(&nandFileInfo));

    if (length != nLength) {
        return 1;
    }

    return 0;
}

#pragma ppc_iro_level reset

static s32 fn_800640BC(const char* szFileName, u32 arg1, u8 arg2) {
    NANDFileInfo nandFileInfo;
    u8 buffer[32] ATTRIBUTE_ALIGN(32);
    s32 var_r31;
    s32 var_r30;
    NANDResult result;

    result = NANDCreate(szFileName, 0x34, 0);
    if (result != NAND_RESULT_OK && result != NAND_RESULT_EXISTS) {
        return 0;
    }

    result = NANDSafeOpen(szFileName, &nandFileInfo, 3, sBannerBuffer, sizeof(sBannerBuffer));
    if (result != NAND_RESULT_OK) {
        return 0;
    }

    var_r30 = arg1 >> 5;
    memset(buffer, arg2, sizeof(buffer));
    DCFlushRange(buffer, sizeof(buffer));

    for (var_r31 = 0; var_r31 < var_r30; var_r31++) {
        result = NANDWrite(&nandFileInfo, &buffer, sizeof(buffer));
        if (result < NAND_RESULT_OK) {
            break;
        }
    }

    NANDSafeClose(&nandFileInfo);
    if (result < NAND_RESULT_OK) {
        return 0;
    }

    return 1;
}

#pragma ppc_iro_level 0

bool fn_800641CC(NANDFileInfo* pNandFileInfo, const char* szFileName, s32 arg2, s32 arg3, s32 access) {
    u32 spC;
    u32 sp8;
    s32 var_r3;
    s32 var_r4;

    if (lbl_8025D130 == 0) {
        char buffer[64] = {0};

        if (NANDGetHomeDir(buffer) != 0) {
            errorDisplayShow(ERROR_MAX_FILES);
        }

        if (fn_800B48C4(buffer) != 0) {
            errorDisplayShow(ERROR_MAX_FILES);
        }

        lbl_8025D130 = 1;
    }

    arg2 = ((arg2 + 0x3FFF) / 0x4000) << 0xE;

    while (true) {
        lbl_8025D12C = 0;
        sp8 = 0;

        lbl_8025D12C |= fn_8006496C() << 4;
        lbl_8025D12C |= fn_80063F30(szFileName, arg2);

        if (lbl_8025D12C & 0x11) {
            if (!errorDisplayShow(ERROR_SYS_CORRUPT)) {
                return false;
            }

            if (lbl_8025D12C & 0x10) {
                bannerDelete();
            }

            if (lbl_8025D12C & 1) {
                if (NANDDelete(szFileName)) {
                    (void)0;
                } else {
                    (void)0;
                }
            }
        } else if (lbl_8025D12C & 0x22) {
            var_r3 = 0;
            var_r4 = 0;

            if (lbl_8025D12C & 0x20) {
                var_r3 = (((bannerGetBufferSize() + 0x3FFF) / 0x4000) << 0xE) / 0x4000;
                var_r4 = 1;
            }

            if (lbl_8025D12C & 2) {
                var_r3 += arg2 / 0x4000;
                var_r4++;
            }

            sStringDraw[1].unk38 = var_r4;
            sStringDraw[0].unk38 = ((var_r3 << 0xE) + 0x1FFFF) / 0x20000;
            fn_80063F30_Inline(NANDCheck(var_r3, var_r4, &spC));

            if (spC & 5) {
                if (!errorDisplayShow(ERROR_INS_SPACE)) {
                    return false;
                }
            } else if (spC & 0xA) {
                if (!errorDisplayShow(ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU)) {
                    return false;
                }
            } else if ((!(lbl_8025D12C & 0x20) || (bannerWrite() != 0)) && (lbl_8025D12C & 2)) {
                fn_800640BC(szFileName, arg2, arg3);
            }
        } else {
            if (bannerNANDOpen(szFileName, pNandFileInfo, access, &sBannerBuffer, 0x1000) == NAND_RESULT_OK) {
                return true;
            }
        }
    }
}

#pragma ppc_iro_level reset

/**
 * @brief Open the NAND access.
 * @param szFileName Name of the file to open.
 * @param info Pointer to `NANDFileInfo`.
 * @param access Access type, either safe or unsafe.
 * @param buffer Memory buffer to store the opened file.
 * @param len File size.
 * @return `NANDResult` – Default value is `NAND_RESULT_OK`.
 */
static NANDResult bannerNANDOpen(const char* szFileName, NANDFileInfo* info, u8 access, void* buffer, s32 len) {
    if (access & 2) {
        return NANDOpen(szFileName, info, access);
    } else {
        return NANDSafeOpen(szFileName, info, access, buffer, len);
    }

    return NAND_RESULT_OK;
}

/**
 * @brief Close the NAND access.
 * @param info Pointer to `NANDFileInfo`.
 * @param access Access type, either safe or unsafe.
 * @return `bool` – Always `true`.
 */
bool bannerNANDClose(NANDFileInfo* info, u8 access) {
    if (access & 2) {
        NANDClose(info);
    } else {
        NANDSafeClose(info);
    }

    return true;
}

static inline void bannerSetString(char* src, wchar_t* dest, s32 max) {
    s32 i;
    s32 nSize;

    nSize = 0;

#if VERSION < OOT_J
    while (src[0] != 0x00 && nSize < max) {
        ((char*)dest)[1] = *src++;
        ((char*)dest)[0] = *src++;

        dest++;
        nSize++;
    }
#else
    for (i = 0; i < max; i++) {
        if (src[0] == 0x00 && src[1] == (char)0xBB) {
            break;
        }

        ((char*)dest)[1] = *src++;
        ((char*)dest)[0] = *src++;

        dest++;
        nSize++;
    }
#endif

    if (nSize == 0) {
        *dest++ = ' ';
    }

    *dest = '\0';
}

/**
 * @brief Creates a new banner.
 * @param szGameName The title of the banner, usually the emulated game's name.
 * @param szEmpty The subtitle of the banner, usually an empty string.
 * @return `bool` – `true` on success, `false` on failure.
 */
bool bannerCreate(char* szGameName, char* szEmpty) {
    wchar_t comment[2][NAND_BANNER_TITLE_MAX];
    TPLPalette* tplPal;
    u32 i;
    NANDBanner* temp_r26;

    xlHeapTake((void**)&sBanner, 0x10000 | 0x70000000);

    temp_r26 = sBanner;

    if (!xlFileLoad("save_banner.tpl", (void**)&tplPal)) {
        return false;
    }

    TPLBind(tplPal);

    memset(temp_r26, 0, sizeof(NANDBanner));
    memset(comment, 0, sizeof(comment));

    if (szGameName != NULL) {
        bannerSetString(szGameName, comment[0], NAND_BANNER_TITLE_MAX - 1);
    }

    if (szEmpty != NULL) {
        bannerSetString(szEmpty, comment[1], NAND_BANNER_TITLE_MAX - 1);
    }

    NANDInitBanner(temp_r26, 0x10, comment[0], comment[1]);
    memcpy(temp_r26->bannerTexture, tplPal->descriptors[0].texHeader->data, sizeof(temp_r26->bannerTexture));

    for (i = 0; i < 8; i++) {
        memcpy(temp_r26->iconTexture[i], tplPal->descriptors[lbl_8025C890[i]].texHeader->data,
               sizeof(temp_r26->iconTexture[0]));
        temp_r26->iconSpeed = (temp_r26->iconSpeed & ~(3 << (i * 2))) | (lbl_8025C888[i] << (i * 2));
    }

    if (i < 8) {
        temp_r26->iconSpeed = (temp_r26->iconSpeed & ~(3 << (i * 2)));
    }

    xlHeapFree((void**)&tplPal);
    return true;
}

/**
 * @brief Writes the banner file to the NAND.
 * @return `bool` – `true` on success, `false` on failure.
 */
static bool bannerWrite(void) {
    NANDFileInfo info;
    void* pBuffer;
    s32 nResult;

    pBuffer = sBanner;

    nResult = NANDCreate("banner.bin", 0x34, 0);
    if (nResult != NAND_RESULT_EXISTS && nResult != NAND_RESULT_OK) {
        return false;
    }

    if (NANDSafeOpen("banner.bin", &info, 3, sBannerBuffer, sizeof(sBannerBuffer))) {
        return false;
    }

    DCFlushRange(pBuffer, sizeof(NANDBanner));
    nResult = NANDWrite(&info, pBuffer, sizeof(NANDBanner));
    NANDSafeClose(&info);

    return nResult >= NAND_RESULT_OK;
}

/**
 * @brief Deletes the banner file from the NAND.
 * @return `bool` – `true` on success, `false` on failure.
 */
static s32 bannerDelete(void) { return NANDDelete("banner.bin") == NAND_RESULT_OK; }

/**
 * @brief Returns the buffer size.
 * @return `s32` – The size of the buffer.
 */
static s32 bannerGetBufferSize(void) { return sizeof(NANDBanner); }

#pragma dont_inline on

static s32 fn_8006496C(void) { return fn_80063F30("banner.bin", sizeof(NANDBanner)); }

#pragma dont_inline off
