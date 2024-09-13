/**
 * @file stringtable.c
 *
 * This file implements tools to read a string table (following the "ST10" format, see below for more details)
 * and get a pointer to a string (from the string's unique identifier) from this table.
 *
 * "ST10" is the name of a custom string table file format.
 *
 * The header of the file starts with the version of the format (in this case it's "ST10"), then
 * it's followed by the table's identifier, then there's the number of entries, the name of the encoding,
 * the region's name and the size of one entry. The header ends with 0xC0, the purpose of this value is unknown.
 *
 * Each entry contains an identifier (so the emulator knows which string to choose),
 * the offset to the string, and its size.
 *
 * Finally, the file ends with the list of the strings.
 * Each string is zero-terminated and aligned to 0x4 (using 0xBB as a padding value).
 */
#include "emulator/stringtable.h"

/**
 * @brief Get the string table entry corresponding to the string ID.
 * @param pStringTable Pointer to the string table.
 * @param eStringID The ID of the string table entry to get.
 * @return `STEntry*` – pointer of the corresponding string table entry if found, else NULL.
 */
STEntry* tableGetEntry(StringTable* pStringTable, StringID eStringID) {
    u32 nStringID;
    s32 lo;
    s32 hi;
    s32 mid;
    u8* pEntries = (u8*)(&pStringTable->header.entries);

    if (pStringTable == NULL || pStringTable->header.nEntries == 0) {
        return NULL;
    }

    hi = pStringTable->header.nEntries - 1;
    lo = 0;
    mid = hi / 2;

    while (hi >= lo) {
        STEntry* entry = (STEntry*)(pEntries + mid * pStringTable->header.nSizeEntry);

        if (entry->nStringID == eStringID) {
            return entry;
        }

        if (entry->nStringID < eStringID) {
            lo = mid + 1;
        } else if (entry->nStringID > eStringID) {
            hi = mid - 1;
        }

        mid = lo + ((hi - lo) / 2);
    }

    return NULL;
}

/**
 * @brief Get the string from the string table corresponding to the string ID.
 * @param pSTBuffer Pointer to the string table buffer.
 * @param eStringID The ID of the string to get.
 * @return `char*` – pointer of the corresponding string if found, else NULL.
 */
char* tableGetString(void* pSTBuffer, StringID eStringID) {
    STEntry* pEntry;

    if (pSTBuffer == NULL) {
        return NULL;
    }

    pEntry = tableGetEntry(pSTBuffer, eStringID);

    if (pEntry != NULL) {
        return (char*)pSTBuffer + pEntry->nTextOffset1;
    }

    return NULL;
}
