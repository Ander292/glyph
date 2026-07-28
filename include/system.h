#ifndef SYSTEM_H
#define SYSTEM_H

#include "escape.h"
#include "types.h"
#include <time.h>
#include <string.h>

typedef struct console_info{
    int Rows, Cols;
    time_t CurrentTime;
} console_info;

typedef struct character_input{
    char arr[4];
    int byteCount;
} character_input;

/* */
void disableRawMode();
void enableRawMode();
console_info getConsoleSystemInfo();
void prepareConsole();
void die(const char *string, ...);

/* Input-Output */
character_input pollInput();
uint32 writeOutput(char *src, uint32 size);

#define TIMEOUT_MS 10
#define CTRL_KEY(k) ((k) & 0x1f)

#define WINDOWS

#if defined WINDOWS
#include <windows.h>
#define Print(str) writeOutput(str, strlen(str))
#elif defined LINUX
#include <termios.h>
#include <unistd.h>

#endif

static inline uint32 _BitScanReverse32(uint32 x){
    uint32 Result = 0;
    __asm__ (
        "bsr %1, %0\n\t"
        : "=r" (Result)
        : "r" (x)
        : "cc"
    );
    
    return Result;
}

static inline uint16 _BitScanReverse16(uint16 x){
    uint16 Result = 0;
    __asm__ (
        "bsr %1, %0\n\t"
        : "=r" (Result)
        : "r" (x)
        : "cc"
    );
    
    return Result;
}

static inline int charGetByteCount(char c){
    if(c == 0) return 0;
    int Result = 15 - _BitScanReverse16(~(c << 8));
    if(Result > 4 || Result <= 0) Result = 1;
    return Result;
}

#endif /* SYSTEM_H */