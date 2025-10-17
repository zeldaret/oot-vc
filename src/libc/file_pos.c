#include "file_pos.h"
#include "buffer_io.h"
#include "errno.h"
#include "revolution/types.h"

int _ftell(FILE* file) {
    int charsInUndoBuffer = 0;
    int position;

    u8 tmp_kind = file->mode.file_kind;
    if (!(tmp_kind == __disk_file || tmp_kind == __console_file) || file->state.error) {
        errno = EFPOS;
        return -1;
    }

    if (file->state.io_state == __neutral) {
        return (file->position);
    }

    position = file->buffer_pos + (file->buffer_ptr - file->buffer);

    if (file->state.io_state >= __rereading) {
        charsInUndoBuffer = file->state.io_state - __rereading + 1;
        position -= charsInUndoBuffer;
    }

    if (!file->mode.binary_io) {
        int n = file->buffer_ptr - file->buffer - charsInUndoBuffer;
        u8* p = (u8*)file->buffer;

        while (n--) {
            if (*p++ == '\n') {
                position++;
            }
        }
    }

    return (position);
}

int ftell(FILE* stream) {
    int retval;

    retval = (long)_ftell(stream);
    return retval;
}

int _fseek(FILE* file, u32 offset, int whence) {
    int bufferCode;
    int pos;
    int adjust;
    u32 state;
    int buffLen;

    char* ptr;

    if (file->mode.file_kind != 1 || file->state.error != 0) {
        errno = EFPOS;
        return -1;
    }

    if (file->state.io_state == 1) {
        if (__flush_buffer(file, NULL) != 0) {
            file->state.error = 1;
            file->buffer_len = 0;
            errno = EFPOS;
            return -1;
        }
    }

    if (whence == SEEK_CUR) {
        whence = SEEK_SET;
        offset += _ftell(file);
    }

    if ((whence != SEEK_END) && (file->mode.io_mode != 3) && (file->state.io_state == 2 || file->state.io_state == 3)) {
        if ((offset >= file->position) || !(offset >= file->buffer_pos)) {
            file->state.io_state = 0;
        } else {
            file->buffer_ptr = file->buffer + (offset - file->buffer_pos);
            file->buffer_len = file->position - offset;
            file->state.io_state = 2;
        }
    } else {
        file->state.io_state = 0;
    }

    if (file->state.io_state == 0) {
        if (file->position_proc != NULL && (int)file->position_proc(file->handle, &offset, whence, file->ref_con)) {
            file->state.error = 1;
            file->buffer_len = 0;
            errno = EFPOS;
            return -1;
        } else {
            file->state.eof = 0;
            file->position = offset;
            file->buffer_len = 0;
        }
    }

    return 0;
}
