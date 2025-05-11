#include "ansi_files.h"
#include "buffer_io.h"
#include "ctype.h"
#include "locale.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int fclose(FILE* file) {
    int flush_result;
    int close_result;

    if (file == NULL) {
        return -1;
    }

    if (file->mode.file_kind == __closed_file) {
        return 0;
    }

    flush_result = fflush(file);

    close_result = (*file->close_proc)(file->handle);

    file->mode.file_kind = __closed_file;
    file->handle = 0;

    if (file->state.free_buffer) {
        free(file->buffer);
    }

    return ((flush_result || close_result) ? -1 : 0);
}

int fflush(FILE* file) {
    int pos;

    if (file == NULL) {
        return __flush_all();
    }

    if (file->state.error != 0 || file->mode.file_kind == __closed_file) {
        return -1;
    }

    if (file->mode.io_mode == 1) {
        return 0;
    }

    if (file->state.io_state >= 3) {
        file->state.io_state = 2;
    }

    if (file->state.io_state == 2) {
        file->buffer_len = 0;
    }

    if (file->state.io_state != 1) {
        file->state.io_state = 0;
        return 0;
    }

    if (file->mode.file_kind != __disk_file) {
        pos = 0;
    } else {
        pos = ftell(file);
    }

    if (__flush_buffer(file, 0) != 0) {
        file->state.error = 1;
        file->buffer_len = 0;
        return -1;
    }

    file->state.io_state = 0;
    file->position = pos;
    file->buffer_len = 0;
    return 0;
}

int __msl_strnicmp(const char* pStr1, const char* pStr2, int n) {
    int i;
    char c1, c2;

    for (i = 0; i < n; i++) {
        c1 = tolower(*pStr1++);
        c2 = tolower(*pStr2++);
        if (c1 < c2) {
            return -1;
        }

        if (c1 > c2) {
            return 1;
        }

        if (!c1) {
            return 0;
        }
    }

    return 0;
}

char* __msl_itoa(int value, char* str, unsigned int base) {
    int var_r7;
    int iStr;
    char c;
    int start;
    int end;

    var_r7 = 0;
    iStr = 0;

    if (value < 0) {
        value = -value;
        var_r7 = 1;
    }

    do {
        int temp_r9 = value % base;

        if (temp_r9 > 9) {
            str[iStr++] = temp_r9 + 0x37;
        } else {
            str[iStr++] = temp_r9 + 0x30;
        }

        value /= base;
    } while (value != 0);

    if (var_r7 != 0) {
        str[iStr++] = '-';
    }

    str[iStr++] = '\0';

    start = 0;
    end = strlen(str) - 1;

    while (start < end) {
        c = str[start];
        str[start++] = str[end];
        str[end--] = c;
    }

    return str;
}
