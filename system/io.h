#ifndef SYS_IO_H
#define SYS_IO_H

#define WINDOWS

#include "types.h"
#include "escape.h"

typedef enum read_result_type{
    TYPE_ANSI,
    TYPE_UTF32,
    TYPE_ESCAPE
} read_result_type;

typedef struct read_result{
    read_result_type type;
    u32 size;

    u8 bytes[8];
} read_result;

read_result io_ReadConsoleInput();

#endif /* SYS_IO_H */
