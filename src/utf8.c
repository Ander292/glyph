#include "utf8.h"
#include "system.h"
#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Allocates the memory for the string */
string stringCreate(int size){
    string Result;
    Result.len = 0;
    Result.byteLen = 0;
    Result.maxSize = size;

    Result.byteCount = malloc(size);
    Result.data = malloc(size * 4);

    if(!Result.byteCount){
        die("Error allocating %zu bytes", size);
    }
    if(!Result.data){
        die("Error allocating %zu bytes", size * 4);
    }

    *Result.byteCount = 0;
    *Result.data = 0;

    return Result;
}

void doubleSize(string *str){
    str->maxSize <<= 2;
    str->byteCount = realloc(str->byteCount, str->maxSize);
    if(!str->byteCount){
        die("Error reallocating %zu bytes", str->maxSize);
    }
    str->data = realloc(str->data, str->maxSize * 4);
    if(!str->byteCount){
        die("Error reallocating %zu bytes", str->maxSize * 4);
    }
}

void stringAppendEnd(string *str, const char *c, int size){
    while(str->len + size >= str->maxSize) doubleSize(str);

    int pos = 0;
    while(pos < size){
        int byteCount = charGetByteCount(c[pos]);
        str->byteCount[str->len]= byteCount;
        
        
        int j = 0;
        for(; j < byteCount; j++){
            str->data[str->byteLen++] = c[pos + j];
        }
        str->data[str->byteLen] = 0;
        pos += byteCount;
        str->len += 1;
    }
    str->byteCount[str->len] = 0;

    if(pos > size * 4) die("Fatal error. Pos: %d > Size: %d", pos, size);
}

void clearBuffer(string *str){
    *str->byteCount = 0;
    *str->data = 0;
    str->len = 0;
    str->byteLen = 0;    
}