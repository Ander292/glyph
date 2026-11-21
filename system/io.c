#include "io.h"

static inline void universalCheck(char character, int *byteCountAdr){
    if(((character >= 'a' && character <= 'z') || 
        (character >= 'A' && character <= 'Z')) && *byteCountAdr < 2) *byteCountAdr = 2; 
    
    if(character == 'h' || character == 'f' || character == 'H' || character == 'F' ||
       character == 'o' || character == 'O') *byteCountAdr = 3;

    if((character <= '9' && character >= '0' 
        || character == 'O') && *byteCountAdr < 4) *byteCountAdr = 4;
    if(character == ';') *byteCountAdr = 6;
}

static inline i32 charGetByteCount(char c){
    if(c == 0) return 0;
    if((c & 0x80) == 0) return 1;
    if((c & 0xE0) == 0xC0) return 2;
    if((c & 0xF0) == 0xE0) return 3;
    if((c & 0xF8) == 0xF0) return 4;
}

#ifdef WINDOWS
#include "win32.c"
#else
#endif

read_result io_ReadConsoleInput(){
    read_result result = {0};

    result.bytes[0] = readByteBlock();
    if(result.bytes[0] == ESC_SEQ[0]){
        result.size = 3;

        for(int i = 1; i < result.size; i++){
                result.bytes[i] = readByteBlock();
                universalCheck(result.bytes[i], &result.size);
            }
    }else{
        result.size = charGetByteCount(result.bytes[0]);
        
        for(int i = 1; i < result.size; i++){
            result.bytes[i] = readByteBlock();
        }
    }

    return result;
}