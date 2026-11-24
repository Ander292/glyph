#include "io.h"
#include <stdio.h>

static inline void universalCheck(char character, int *byteCountAdr){
    if(((character >= 'a' && character <= 'z') || 
        (character >= 'A' && character <= 'Z')) && *byteCountAdr < 2) *byteCountAdr = 2; 
    
    if(character == 'h' || character == 'f' || character == 'H' || character == 'F' ||
       character == 'o' || character == 'O') *byteCountAdr = 3;

    if((character <= '9' && character >= '0' 
        || character == 'O') && *byteCountAdr < 4) *byteCountAdr = 4;
    if(character == ';') *byteCountAdr = 6;
}

#ifdef WINDOWS
#include "win32.c"
#elif defined UNIX
#include "unix32.c"
#endif

read_result io_ReadConsoleInput(){
    read_result result = {0};

    result.bytes[0] = readByte();
    if(result.bytes[0] == ESC_SEQ[0]){
        result.type = TYPE_ESCAPE;
        result.size = 3;

        for(int i = 1; i < result.size; i++){
                result.bytes[i] = readByte();
                universalCheck(result.bytes[i], &result.size);
            }
    }else{
        result.size = charGetByteCount(result.bytes[0]);
        if(result.size == 1) result.type = TYPE_ANSI;
        else result.type = TYPE_UTF32;

        for(int i = 1; i < result.size; i++){
            result.bytes[i] = readByte();
        }
    }

    return result;
}

static inline uint32 getInputOutputErrorCode(){
#if defined WINDOWS
    return GetLastError();
#elif defined UNIX
    return errno;
#endif
}

static inline char *getInputOutputErrorString(){
#if defined WINDOWS
    return WIN32_formatError((DWORD)getInputOutputErrorCode());
#elif defined UNIX
    return strerror(errno);
#endif
}

char *io_GetInputOutputErrorString(){
    return getInputOutputErrorString();
}

void Die(const char *format, ...){
    char str[256];

    va_list args;
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);

    uint32 errCode = getInputOutputErrorCode();
    char *errStr = getInputOutputErrorString();

    printf(ESC_CLEAR_SCREEN "%s : (%u) %s\n", str, errCode, errStr);

    readByte();
    exit(1);
}