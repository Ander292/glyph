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
int64 fileWrite(char *destPath, char *string, uint32 size);
int64 fileRead(char *filePath, char *destBuffer, uint32 maxBufferSize);
int64 getFileSize(char *filePath);
void die(const char *string, ...);
char *getInputOutputErrorString();

/* Input-Output */
character_input pollInput();
uint32 writeOutput(char *src, uint32 size);

#define TIMEOUT_MS 10
#define CTRL_KEY(k) ((k) & 0x1f)
#define UNCTRL_KEY(k) ((k) | 0x60)

#define Print(str) writeOutput(str, strlen(str))
//#define LINUX
//#define arm

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
#ifdef x86
static inline uint32 _BitScanReverse32(uint32 x){
    uint32 Result = 0;
    __asm__ (
        "bsr %0, %1\n\t"
        : "=r" (Result)
        : "r" (x)
        : "cc"
    );

    return Result;
}

static inline uint16 _BitScanReverse16(uint16 x){
    uint16 Result = 0;
    __asm__ (
        "bsr %0, %1\n\t"
        : "=r" (Result)
        : "r" (x)
        : "cc"
    );

    return Result;
}
#elif defined arm
static inline uint32 _CountLeadingZeros32(uint32 x){
    uint32 Result = 0;
    __asm__ (
        "clz %0, %1\n\t"
        : "=r" (Result)
        : "r" (x)
        : "cc"
    );

    return Result;
}
#endif

static inline int charGetByteCount(char c){
    if(c == 0) return 0;
#ifdef x86
    int Result = 15 - _BitScanReverse16(~(c << 8));
#elif defined arm
    int Result = _CountLeadingZeros32(~((uint32)c << 24));
#else
    int Result = 1;
#endif
    if(Result > 4 || Result <= 0) Result = 1;
    return Result;
}


#endif /* SYSTEM_H */
