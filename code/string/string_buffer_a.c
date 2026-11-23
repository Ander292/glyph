#include "string_buffer.h"
#include "../system/io.h"
#include "main.h"

#include <stdlib.h>

static const char *keyword1[] = {
    "while",
    "for",
    "if",
    "else",
    "switch",
    "case",
    "default",
    "return",
    "break",
    "continue"
};

static const char *keyword2[] = {
    "int",
    "short",
    "char",
    "long",
    "unsigned",
    "signed",
    "static",
    "void",
    "float",
    "double",
    "inline",
    "auto",
    "const",

    "int8",
    "int16",
    "int32",
    "int64",
    "uint8",
    "uint16",
    "uint32",
    "uint64",

    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "size_t"
};

static const char *preprocesorKeyword[] = {
    "#define",
    "#include",
    "#undef",
    "#if",
    "#ifdef",
    "#ifndef",
    "#else",
    "#elif",
    "#region",
    "#endregion",
    "#pragma",
    "#endif",
    "#warning",
    "#error"
};

string_a StringCreateA(u32 size){
    size = powerOfTwoRoundUp(size);
    string_a result = {0};
    result.maxSize = size;
    result.len = 0;

    result.data = malloc(size * sizeof(*result.data));
    if(result.data == NULL) {
        Die("Error allocating string_a data array");
    }
    result.tokenId = malloc(size * sizeof(*result.tokenId));
    if(result.tokenId == NULL) {
        Die("Error allocating string_a tokenId array");
    }

    return result;
}

static void doubleSizeA(string_a *str){
    str->maxSize *= 2;
    str->data = realloc(str->data, str->maxSize * sizeof(*str->data));
    if(str->data == NULL){
        Die("Error doubling size of string_a data array");
    }

    str->tokenId = realloc(str->tokenId, str->maxSize * sizeof(*str->tokenId));
    if(str->tokenId == NULL){
        Die("Error doubling size of string_a tokenId array");
    }
}

void StringAppendEndA(string_a *str, const char *c, u32 size){
    while(str->len + size >= str->maxSize) doubleSizeA(str);

    char *dataStart = str->data + str->len;
    char *tokenStart = str->tokenId + str->len;
    
    str->len += size;
    memcpy(dataStart, c, size);
    memset(tokenStart, 0, size);
}

void OverrunBufferWithA(string_a *str, const char *c, u32 size){
    while(size >= str->maxSize) doubleSizeA(str);

    str->len = size;
    memcpy(str->data, c, size);
    memset(str->tokenId, 0, size);
}

#if 0
void ClearBufferA(string_a *str){
    str->len = 0;
    *str->data = 0;
    *str->tokenId = 0;
}
#endif

string_a *StringCreateHeapA(u32 size){
    size = powerOfTwoRoundUp(size);

    string_a *result = malloc(sizeof(*result));
    if(result == NULL){
        Die("Error allocating string_a on heap");
    }

    result->maxSize = size;
    result->len = 0;

    result->data = malloc(size * sizeof(*result->data));
    if(result->data == NULL) {
        Die("Error allocating stringA data array");
    }
    result->tokenId = malloc(size * sizeof(*result->tokenId));
    if(result->tokenId == NULL) {
        Die("Error allocating stringA tokenId array");
    }

    return result;    
}

string_a *BufferCreateFromStringA(const char *c, u32 size){
    string_a *result = StringCreateHeapA(powerOfTwoRoundUp(size));
    StringAppendEndA(result, c, size);

    return result;
}

#if 0
void StringFreeA(string_a *str){
    free(str->data);
    free(str->tokenId);
    str->data = NULL;
    str->tokenId = NULL;
    str->maxSize = 0;
    str->len = 0;
}
void StringFreeHeapA(string_a *str){
    free(str->data);
    free(str->tokenId);
    free(str);
}
#endif

bool32 InsertCharAtPosA(string_a *str, char c, u32 index, u32 insertionMode){
    if(index == str->len){
        if(str->len == str->maxSize) doubleSizeA(str);

        str->tokenId[str->len] = TOKEN_NEUTRAL;
        str->data[str->len++] = c;
        return true;

    }else if(index < str->len){
        if(!TEST_FLAG(insertionMode, INS_MODE_OVERWRITE)){
            char *dataInsPos = str->data + index;
            char *tokenInsPos = str->tokenId + index;

            u32 shiftLen = str->len - index;
            memmove(dataInsPos + 1, dataInsPos, shiftLen);
            memmove(tokenInsPos + 1, tokenInsPos, shiftLen);
            str->len++;
        }
        str->tokenId[index] = TOKEN_NEUTRAL;
        str->data[index] = c;
        return true;

    }else{ // index > str->len
        if(TEST_FLAG(insertionMode, INS_MODE_INCREASE_SIZE)){
            while(index >= str->maxSize) doubleSizeA(str);

            char *firstFreeChar = str->data + str->len;
            char *firstFreeToken = str->tokenId + str->len;
            u32 emptySegmentLen = index - str->len;

            memset(firstFreeChar, ' ', emptySegmentLen);
            memset(firstFreeToken, TOKEN_NEUTRAL, emptySegmentLen);
            str->len += (emptySegmentLen + 1);

            str->data[index] = c;
            str->tokenId[index] = TOKEN_NEUTRAL;
            return true;
        }else{
            return false;
        }
    }
}

bool32 DeleteCharFromPosA(string_a *str, u32 pos){
    if(pos > str->len) return false;
    else{
        char *destPos = str->data + pos;
        char *destPosToken = str->tokenId + pos;

        u32 moveLen = str->len - pos - 1;
        memmove(destPos, destPos + 1, moveLen);
        memmove(destPosToken, destPosToken + 1, moveLen);
        str->len--;

        return true;
    }
}

bool32 TerminateStringOnPosA(string_a *str, u32 pos){
    if(pos > str->len) return false;
    else if(pos < str->len){
        str->len = pos;
    }else{ // pos == str->len
        return true;
    }
}

string_a *CreateCopyA(string_a *str, u32 startOffset, u32 endOffset){
    if(startOffset >= str->len || endOffset >= str->len) return NULL;

    string_a *result = StringCreateHeapA(str->maxSize);

    char *startPosData = str->data + startOffset;
    char *startPosToken = str->tokenId + startOffset;
    u32 len = str->len - startOffset - endOffset;

    memcpy(result->data, startPosData, len);
    memcpy(result->tokenId, startPosToken, len);
    result->len = len;
    
    return result;
}