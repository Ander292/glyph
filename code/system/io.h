#ifndef SYS_IO_H
#define SYS_IO_H

#define WINDOWS

#include "types.h"
#include "res/escape.h"
#include "errno.h"
#include <string.h>
#include <time.h>

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

typedef struct console_info{
    int Rows, Cols;
    time_t CurrentTime;
} console_info;

//void io_SetCursorPossition(u32 x, u32 y);
read_result io_ReadConsoleInput();

// void term_DisableRawMode();
// void term_EnableRawMode();
void term_PrepareConsole();
console_info term_GetConsoleSystemInfo();

char *io_GetInputOutputErrorString();
void Die(const char *format, ...);

u32 writeOutput(const char *src, u32 size);
#define PrintA(str) writeOutput(str, strlen(str))

static inline i32 charGetByteCount(char c){
    if(c == 0) return 0;
    if((c & 0x80) == 0) return 1;
    if((c & 0xE0) == 0xC0) return 2;
    if((c & 0xF0) == 0xE0) return 3;
    if((c & 0xF8) == 0xF0) return 4;
}

i64 io_GetFileSize(const char *filePath);
i64 io_FileRead(const char *filePath, char *destBuffer, u32 maxBufferSize);
i64 io_FileWrite(const char *destPath, char *string, u32 size);

#endif /* SYS_IO_H */
