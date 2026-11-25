#include "string_buffer.h"

#include <stdlib.h>

char *token_escape[] = {
    ESC_RESET_TEXT_ATTRIBUTES,
    ESC_TEXT_COLOR_BLUE,
    ESC_TEXT_COLOR_MAGENTA,
    ESC_TEXT_COLOR_YELLOW_BRIGHT,
    ESC_TEXT_COLOR_GREEN_BRIGHT,
    ESC_TEXT_COLOR_GREEN,
    ESC_TEXT_COLOR_CYAN,
    ESC_TEXT_COLOR_MAGENTA_BRIGHT,
    ESC_BACKGROUND_COLOR_GREEN_BRIGHT,
    ESC_TEXT_COLOR_RED_BRIGHT,
    ESC_BACKGROUND_COLOR_WHITE_BRIGHT,
};

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