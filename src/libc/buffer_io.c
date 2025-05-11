#include "buffer_io.h"
#include "stdio.h"

void __prep_buffer(FILE* file) {
    file->buffer_ptr = file->buffer;
    file->buffer_len = file->buffer_size;
    file->buffer_len = file->buffer_len - (file->position & file->buffer_alignment);
    file->buffer_pos = file->position;
    return;
}

int __flush_buffer(FILE* file, size_t* length) {
    size_t bufferLen;
    int writeCode;

    bufferLen = file->buffer_ptr - file->buffer;
    if (bufferLen) {
        file->buffer_len = bufferLen;
        writeCode = file->write_proc(file->handle, file->buffer, &file->buffer_len, file->ref_con);
        if (length) {
            *length = file->buffer_len;
        }
        if (writeCode) {
            return writeCode;
        }
        file->position += file->buffer_len;
    }

    file->buffer_ptr = file->buffer;
    file->buffer_len = file->buffer_size;
    file->buffer_len = file->buffer_len - (file->position & file->buffer_alignment);
    file->buffer_pos = file->position;
    return 0;
}
