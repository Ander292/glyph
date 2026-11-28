#include "string_buffer.h"

#include <stdlib.h>

void ClearBuffer(void *str){
    string *n = str;
    n->len = 0;
    *(byte*)n->unknown = 0;
    *n->tokenId = 0;
}

void StringFree(void *str){
    string *n = str;
    free(n->unknown);
    free(n->tokenId);
    n->unknown = NULL;
    n->tokenId = NULL;
    n->maxSize = 0;
    n->len = 0;
}

void StringFreeHeap(void *str){
    string *n = str;
    free(n->unknown);
    free(n->tokenId);
    free(n);
}