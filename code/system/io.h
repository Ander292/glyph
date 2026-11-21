#ifndef SYS_IO_H
#define SYS_IO_H

#define WINDOWS

#include "../include/types.h"
#include "../include/escape.h"
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

read_result io_ReadConsoleInput();

// void term_DisableRawMode();
// void term_EnableRawMode();
void term_PrepareConsole();
console_info term_GetConsoleSystemInfo();

void Die(const char *format, ...);

u32 writeOutput(char *src, u32 size);
#define PrintA(str) writeOutput(str, strlen(str))


#endif /* SYS_IO_H */
