#include "stdio.h"

#include <sys/sys.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define __FILE_BUFF_SIZE 64

#define __FILE_FLAG_READ 1
#define __FILE_FLAG_WRITE 2

#define __FILE_BUFF_TYPE_UNBUFF 0
#define __FILE_BUFF_TYPE_LINEBUFF 1
#define __FILE_BUFF_TYPE_FULLBUFF 2

struct __FILE_INTERNAL {
    int file_fd;

    int flags;

    int buff_type;
    char* buff;
    unsigned buff_size;
    unsigned buff_clen;

    int status;
};

FILE __stdin = {
    0,
    __FILE_FLAG_READ,
    __FILE_BUFF_TYPE_UNBUFF,
    NULL, 0, 0, 0
};
FILE __stdout = {
    1,
    __FILE_FLAG_WRITE,
    __FILE_BUFF_TYPE_LINEBUFF,
    NULL, 0, 0, 0
};
FILE __stderr = {
    2,
    __FILE_FLAG_WRITE,
    __FILE_BUFF_TYPE_UNBUFF,
    NULL, 0, 0, 0
};

FILE* stdin = &__stdin;
FILE* stdout = &__stdout;
FILE* stderr = &__stderr;

void __stdio_init() {
    __stdout.buff_size = __FILE_BUFF_SIZE;
    __stdout.buff = malloc(__FILE_BUFF_SIZE);

    // open streams (should be in kernel?)
    __stdin.file_fd = sys_open("/dev/tty");
    __stdout.file_fd = sys_open("/dev/tty");
    __stderr.file_fd = sys_open("/dev/tty");
}


void __stdio_fini() {
    fclose(&__stdin);
    fclose(&__stdout);
    fclose(&__stderr);
}

FILE* fopen(const char* path, const char* mode) {
    (void)mode;
    // TODO: parse mode and stat file to get buffering type
    int fd = sys_open(path);
    if (fd < 0) {
        // TODO: set lib errno
        return NULL;
    }

    FILE* file = malloc(sizeof(FILE));
    file->file_fd = fd;
    file->flags = 0;
    file->buff_type = __FILE_BUFF_TYPE_UNBUFF;
    file->status = 0;

    return file;
}

int fclose(FILE* file) {
    if (file->file_fd == -1)
        return EOF;
    
    int __err = 0;
    if(fflush(file))
        __err = 1;

    if(sys_close(file->file_fd))
        __err = 1;

    if (file->buff_type != __FILE_BUFF_TYPE_UNBUFF)
        free(file->buff);

    if (file == &__stdin || file == &__stdout || file == &__stderr)
        file->file_fd = -1;
    else
        free(file);

    return __err * EOF;
}

size_t fwrite(const void *buffer, size_t _size, size_t nmemb, FILE *stream) {
    size_t size = _size*nmemb; 

    int write_to_buff = (
        stream->buff_type != __FILE_BUFF_TYPE_UNBUFF
        && stream->buff_clen + size <= stream->buff_size
    );

    if (stream->buff_type == __FILE_BUFF_TYPE_LINEBUFF) {
        const unsigned char* __cb = buffer;
        for (unsigned i=0; i<size; i++) {
            if (__cb[i] == (unsigned char) '\n') {
                write_to_buff = 0;
                break;
            }
        }
    }

    if (write_to_buff) {
        // fits in buffer
        memcpy(stream->buff+stream->buff_clen, buffer, size);
        stream->buff_clen += size;
        return size;
    }

    if(fflush(stream))
        return 0;
    
    int sys_res = sys_write(stream->file_fd, buffer, size);
    if (sys_res < 0) {
        stream->status = sys_res * -1;
        return 0;
    }
    stream->status = 0;
    return sys_res;
}

size_t fread(void *buffer, size_t _size, size_t nmemb, FILE *stream) {
    int sys_res = sys_read(stream->file_fd, buffer, _size*nmemb);

    if (sys_res < 0) {
        stream->status = sys_res * -1;
        return 0;
    }

    if (sys_res == 0)
        stream->status = EOF;

    stream->status = 0;
    return sys_res;
}

int fflush(FILE* file) {
    if (file->buff_type == __FILE_BUFF_TYPE_UNBUFF || !file->buff_clen)
        return 0;

    int sys_res = sys_write(file->file_fd, file->buff, file->buff_clen);
    file->buff_clen = 0;
    if (sys_res < 0) {
        file->status = sys_res * -1;
        return EOF;
    }
    return 0;
}

int fputc(int ch, FILE *stream) {
    unsigned char _c = (unsigned char) ch;
    if(fwrite(&_c, 1, 1, stream)) {
        return ch;
    }
    return EOF;
}

int fputs(const char* str, FILE *stream) {
    return fwrite(str, strlen(str), 1, stream) ?: EOF;
}

int fcntl(FILE* file, unsigned mode, unsigned flags) {
    if (mode == F_GETFL) {
        return sys_fcntl(file->file_fd, (unsigned) -1);
    } else if (mode == F_SETFL) {
        int res = sys_fcntl(file->file_fd, flags);
        if (res < 0)
            return res;
        return 0;
    }
    return 0;
}
