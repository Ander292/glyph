#ifndef SYSTEM_H
#define SYSTEM_H

#include "escape.h"
#include "types.h"
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

typedef struct console_info{
    int Rows, Cols;
    time_t CurrentTime;
} console_info;

typedef struct character_input{
    char arr[8];
    int byteCount;
} character_input;

/* */
void disableRawMode();
void enableRawMode();
console_info getConsoleSystemInfo();
void prepareConsole();
uint32 fileWrite(char *destPath, char *string, uint32 size);
uint32 fileRead(char *filePath, char *destBuffer, uint32 maxBufferSize);
int64 getFileSize(char *filePath);
void die(const char *string, ...);

/* Input-Output */
character_input pollInput();
uint32 writeOutput(char *src, uint32 size);

#define TIMEOUT_MS 10
#define CTRL_KEY(k) ((k) & 0x1f)

#define Print(str) writeOutput(str, strlen(str))
#define WINDOWS

#if defined WINDOWS
#include <windows.h>
#elif defined LINUX
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

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