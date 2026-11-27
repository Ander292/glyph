#include "string_buffer.h"
#include "system/io.h"
#include "main.h"

#include <stdlib.h>

static const char32_t *keyword1[] = {
    U"while",
    U"for",
    U"if",
    U"else",
    U"switch",
    U"case",
    U"default",
    U"return",
    U"break",
    U"continue"
};

static const char32_t *keyword2[] = {
    U"int",
    U"short",
    U"char",
    U"long",
    U"unsigned",
    U"signed",
    U"static",
    U"void",
    U"float",
    U"double",
    U"inline",
    U"auto",
    U"const",

    U"int8",
    U"int16",
    U"int32",
    U"int64",
    U"uint8",
    U"uint16",
    U"uint32",
    U"uint64",

    U"int8_t",
    U"int16_t",
    U"int32_t",
    U"int64_t",
    U"uint8_t",
    U"uint16_t",
    U"uint32_t",
    U"uint64_t",
    U"size_t",

    U"i8",
    U"i16",
    U"i32",
    U"i64",
    U"u8",
    U"u16",
    U"u32",
    U"u64"
};

static const char32_t *preprocesorKeyword[] = {
    U"#define",
    U"#include",
    U"#undef",
    U"#if",
    U"#ifdef",
    U"#ifndef",
    U"#else",
    U"#elif",
    U"#region",
    U"#endregion",
    U"#pragma",
    U"#endif",
    U"#warning",
    U"#error"
};

string_u32 StringCreateU32(u32 size){
    size = powerOfTwoRoundUp(size);
    string_u32 result = {0};
    result.maxSize = size;
    result.len = 0;

    result.data = malloc(size * sizeof(*result.data));
    if(result.data == NULL) {
        Die("Error allocating string_u32 data array");
    }
    result.tokenId = malloc(size * sizeof(*result.tokenId));
    if(result.tokenId == NULL) {
        Die("Error allocating string_u32 tokenId array");
    }

    return result;
}

static void doubleSizeU32(string_u32 *str){
    str->maxSize *= 2;
    str->data = realloc(str->data, str->maxSize * sizeof(*str->data));
    if(str->data == NULL){
        Die("Error doubling size of string_u32 data array");
    }

    str->tokenId = realloc(str->tokenId, str->maxSize * sizeof(*str->tokenId));
    if(str->tokenId == NULL){
        Die("Error doubling size of string_u32 tokenId array");
    }
}

void StringAppendEndU32(string_u32 *str, const char *c, u32 size){
    while(str->len + size >= str->maxSize) doubleSizeU32(str); // NOTE: Overkill as it uses multibyte string len

    
    char *tokenStart = str->tokenId + str->len;
    
    //str->len += size;
    //memcpy(dataStart, c, size);
    for(u32 i = 0; i < size;){
        u32 byteCount = charGetByteCount(c[i]);
        char *writePtr = (char*)(str->data + str->len);
        memcpy(writePtr, c + i, byteCount);
        
        if(4 - byteCount > 0){
            memset(writePtr + byteCount, 0, 4 - byteCount);
        }

        i += byteCount;
        str->len++;

        debug_check(i > size){ // If we copied more than what was the size then we had an incomplete utf32 char
            Die("StringAppendEndU32 found an incomplete utf8 character. c:%x bc:%u", (u32*)writePtr, byteCount);
        }
    }
    memset(tokenStart, 0, size);
}

void OverrunBufferWithU32(string_u32 *str, const char *c, u32 size){
    //while(size >= str->maxSize) doubleSizeU32(str); // NOTE: Overkill as it uses multibyte string len
    //str->len = size;
    //memcpy(str->data, c, size);
    //memset(str->tokenId, 0, size);

    str->len = 0;
    StringAppendEndU32(str, c, size);
}

#if 0
void ClearBufferU32(string_u32 *str){
    str->len = 0;
    *str->data = (wchar32){0};
    *str->tokenId = 0;
}
#endif

string_u32 *StringCreateHeapU32(u32 size){
    size = powerOfTwoRoundUp(size);

    string_u32 *result = malloc(sizeof(*result));
    if(result == NULL){
        Die("Error allocating string_u32 on heap");
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

string_u32 *BufferCreateFromStringU32(const char *c, u32 size){
    string_u32 *result = StringCreateHeapU32(powerOfTwoRoundUp(size));
    StringAppendEndU32(result, c, size);

    return result;
}

#if 0
void StringFreeU32(string_u32 *str){
    free(str->data);
    free(str->tokenId);
    str->data = NULL;
    str->tokenId = NULL;
    str->maxSize = 0;
    str->len = 0;
}
void StringFreeHeapU32(string_u32 *str){
    free(str->data);
    free(str->tokenId);
    free(str);
}
#endif

bool32 InsertCharAtPosU32(string_u32 *str, wchar32 c, u32 index, u32 insertionMode){
    u32 wcharlen = sizeof(str->data->full);
    if(index == str->len){
        if(str->len == str->maxSize) doubleSizeU32(str);

        str->tokenId[str->len] = TOKEN_NEUTRAL;
        str->data[str->len++] = c;
        return true;

    }else if(index < str->len){
        if(!TEST_FLAG(insertionMode, INS_MODE_OVERWRITE)){
            char *dataInsPos = (char*)(str->data + index);
            char *tokenInsPos = str->tokenId + index;

            u32 shiftLen = (str->len - index) * wcharlen;
            u32 tokenShiftLen = (str->len - index);
            memmove(dataInsPos + wcharlen, dataInsPos, shiftLen);
            memmove(tokenInsPos + 1, tokenInsPos, tokenShiftLen);
            str->len++;
        }
        str->tokenId[index] = TOKEN_NEUTRAL;
        str->data[index] = c;
        return true;

    }else{ // index > str->len
        if(TEST_FLAG(insertionMode, INS_MODE_INCREASE_SIZE)){
            while(index >= str->maxSize) doubleSizeU32(str);

            char *firstFreeChar = (char *)(str->data + str->len);
            char *firstFreeToken = str->tokenId + str->len;
            u32 emptySegmentLen = index - str->len;

            memset(firstFreeChar, ' ', emptySegmentLen*4);
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

bool32 DeleteCharFromPosU32(string_u32 *str, u32 pos){
    if(pos > str->len) return false;
    else{
        char *destPos = (char*)(str->data + pos);
        char *destPosToken = str->tokenId + pos;

        u32 moveLen = str->len - pos - 1;
        memmove(destPos, destPos + sizeof(wchar32), moveLen * sizeof(wchar32));
        memmove(destPosToken, destPosToken + 1, moveLen);
        str->len--;

        return true;
    }
}

bool32 TerminateStringOnPosU32(string_u32 *str, u32 pos){
    if(str->len == str->maxSize) doubleSizeU32(str);
    if(pos > str->len) return false;

    str->len = pos;
    str->data[pos] = WCHAR32(0);

    return true;
}

string_u32 *CreateCopyU32(string_u32 *str, u32 startOffset, u32 endOffset){
    if(startOffset >= str->len || endOffset >= str->len) return NULL;

    string_u32 *result = StringCreateHeapU32(str->maxSize);

    char *startPosData = (char*)(str->data + startOffset);
    char *startPosToken = str->tokenId + startOffset;
    u32 len = str->len - startOffset - endOffset;

    memcpy(result->data, startPosData, len);
    memcpy(result->tokenId, startPosToken, len);
    result->len = len;
    
    return result;
}

void TranslateToUtf8(string_a *dest, string_u32 *src){
    // TODO: Handle tokens
    dest->len = 0;
    for(u32 i = 0; i < src->len; i++){
        const char *currentChar = (char*)(src->data + i);
        StringAppendEndA(dest, currentChar, charGetByteCount(*currentChar));
    }
}
void PrintBufferU32(string_u32 *str){
    string_a *Temp = StringCreateHeapA(str->maxSize);
    TranslateToUtf8(Temp, str);
    PrintBufferA(*Temp);

    StringFreeA(Temp);
}

/* Counting functions */

i32 CountToNotChar32(string_u32 *str, i32 startPos, wchar32 targetChar){
    i32 Result = 0;

    u32 byteCount = charGetByteCount(*targetChar.bytes);

    for(i32 i = startPos; i < str->len; i++){
        wchar32 stringChar = GetCharAtPosU32(str, i);
        u32 stringByteCount = charGetByteCount(*stringChar.bytes);

        if(stringByteCount == byteCount){
            for(u32 j = 0; j < byteCount; j++){
                if(targetChar.bytes[j] != stringChar.bytes[j]) goto CHECK_SUCCESS;
            }
            goto CHECK_FAILED;
        }
        CHECK_FAILED:
        Result++;
    }

    CHECK_SUCCESS:
    return Result;
}

i32 CountBackToNotChar32(string_u32 *str, i32 startPos, wchar32 targetChar){
    i32 Result = 0;

    u32 byteCount = charGetByteCount(*targetChar.bytes);

    for(i32 i = startPos - 1; i >= 0; i--){
        wchar32 stringChar = GetCharAtPosU32(str, i);
        u32 stringByteCount = charGetByteCount(*stringChar.bytes);
        
        if(byteCount == stringByteCount){
            for(u32 j = 0; j < stringByteCount; j++){
                if(targetChar.bytes[j] != stringChar.bytes[j]) goto CHECK_SUCCESS;            
            }
            goto CHECK_FAILED;
        }
        CHECK_FAILED:
        Result++;
    }

    CHECK_SUCCESS:
    return Result;
}

i32 CountBackToChar32(string_u32 *str, u32 startPos, wchar32 targetChar){
    i32 Result = 0;
    
    u32 byteCount = charGetByteCount(*targetChar.bytes);

    for(i32 i = startPos - 1; i >= 0; i--){
        wchar32 stringChar = GetCharAtPosU32(str, i);
        u32 stringByteCount = charGetByteCount(*stringChar.bytes);
        
        if(byteCount == stringByteCount){
            for(u32 j = 0; j < stringByteCount; j++){
                if(targetChar.bytes[j] != stringChar.bytes[j]) goto CHECK_FAILED;
            }
            goto CHECK_SUCCESS;
        }
        CHECK_FAILED:
        Result++;
    }

    CHECK_SUCCESS:
    return Result;
}

i32 CountToChar32(string_u32 *str, i32 startPos, wchar32 targetChar){
    i32 Result = 0;

    u32 byteCount = charGetByteCount(*targetChar.bytes);

    for(i32 i = startPos; i < str->len; i++){
        wchar32 stringChar = GetCharAtPosU32(str, i);
        u32 stringByteCount = charGetByteCount(*stringChar.bytes);
        
        if(byteCount == stringByteCount){
            for(u32 j = 0; j < stringByteCount; j++){
                if(targetChar.bytes[j] != stringChar.bytes[j]) goto CHECK_FAILED;
            }
            goto CHECK_SUCCESS;
        }
        CHECK_FAILED:
        Result++;
    }

    CHECK_SUCCESS:
    return Result;
}

void ConcatStringsU32(string_u32 *dest, const string_u32 *src){
    wchar32 *start = dest->data + dest->len;
    
    wchar32 *from = src->data;
    u32 len = src->len;

    while(dest->len + src->len >= dest->maxSize) doubleSizeU32(dest);
    
    memcpy(start, from, len * sizeof(u32));
    dest->len += len;
}

void ConcatStringsExU32(string_u32 *dest, const string_u32 *src, u32 startOffset, u32 endOffset){
    wchar32 *start = dest->data + dest->len;
    char *startToken = dest->tokenId + dest->len;
    
    wchar32 *from = src->data + startOffset;
    char *fromToken = src->tokenId + startOffset;

    u32 len = src->len - startOffset - endOffset;

    while(dest->len + src->len >= dest->maxSize) doubleSizeU32(dest);
    
    memcpy(start, from, len * sizeof(u32));
    memcpy(startToken, fromToken, len * sizeof(char));
    dest->len += len;
}