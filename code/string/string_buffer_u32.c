#include "string_buffer.h"
#include "system/io.h"
#include "main.h"
#include "res/res.h"

#include <stdlib.h>

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
    u32 wcharlen = sizeof(*str->data);
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
    str->data[pos] = 0;

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

    u32 byteCount = charGetByteCount(targetChar);

    for(i32 i = startPos; i < str->len; i++){
        wchar32 stringChar = GetCharAtPosU32(str, i);
        u32 stringByteCount = charGetByteCount(stringChar);

        if(stringByteCount == byteCount){
            // for(u32 j = 0; j < byteCount; j++){
            //     if(targetChar.bytes[j] != stringChar.bytes[j]) goto CHECK_SUCCESS;
            // }
            if(targetChar != stringChar) goto CHECK_SUCCESS;
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

    u32 byteCount = charGetByteCount(targetChar);

    for(i32 i = startPos - 1; i >= 0; i--){
        wchar32 stringChar = GetCharAtPosU32(str, i);
        u32 stringByteCount = charGetByteCount(stringChar);
        
        if(byteCount == stringByteCount){
            // for(u32 j = 0; j < stringByteCount; j++){
            //     if(targetChar.bytes[j] != stringChar.bytes[j]) goto CHECK_SUCCESS;            
            // }
            if(targetChar != stringChar) goto CHECK_SUCCESS;
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
    
    u32 byteCount = charGetByteCount(targetChar);

    for(i32 i = startPos - 1; i >= 0; i--){
        wchar32 stringChar = GetCharAtPosU32(str, i);
        u32 stringByteCount = charGetByteCount(stringChar);
        
        if(byteCount == stringByteCount){
            // for(u32 j = 0; j < stringByteCount; j++){
            //     if(targetChar.bytes[j] != stringChar.bytes[j]) goto CHECK_FAILED;
            // }
            if(targetChar != stringChar) goto CHECK_FAILED;
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

    u32 byteCount = charGetByteCount(targetChar);

    for(i32 i = startPos; i < str->len; i++){
        wchar32 stringChar = GetCharAtPosU32(str, i);
        u32 stringByteCount = charGetByteCount(stringChar);
        
        if(byteCount == stringByteCount){
            // for(u32 j = 0; j < stringByteCount; j++){
            //     if(targetChar.bytes[j] != stringChar.bytes[j]) goto CHECK_FAILED;
            // }
            if(targetChar != stringChar) goto CHECK_FAILED;
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

i32 ResetHighlightU32(string *strr, i32 unused){
    memset(strr->tokenId, TOKEN_NEUTRAL, strr->len);
    return 0;
}

i32 SyntaxHighlightStringU32(string *strr, i32 flags){
    //resetHighlight(str, flags);
    string_u32 *str = (string_u32*)strr;
    // TODO: Merge all the int8s into one variable.
    i8 inComment = flags & SYNTAX_MULTILINE_COMMENT;
    i8 singleLineComment = flags & SYNTAX_WAS_EXTENDED;
    i8 sep = 0;
    i8 prevSep = 1;
    i8 inString = 0, inHashtag = 0, inScream = 0, isNum = 0;
    i32 screamStartIndex = 0;

    i8 oldInComment = 0, oldInString = 0, oldInHashtag = 0, oldSingleLineComment = 0, oldInScream = 0, wasNum = 0;

    i8 commentFirstCond = 0;
    i8 commentBreakCond = 0;
    i8 octCond = 0, isHex = 0;

    for(i32 i = 0; i < str->len; i++){
        wchar32 ci = GetCharAtPosU32(str, i);
        /* The first check to see if its a comment */
        isNum = 0;
        switch(ci){
            case U'"':
            case U'\'':
                inString ^= 1;
                break;
            case U'#':
                inHashtag = 1;
                break;
            case U'/':
                if(commentFirstCond){
                    inComment = 1;
                    singleLineComment = 1;
                }else if(commentBreakCond){
                    inComment = 0;
                }else{
                    commentFirstCond = 1;
                }
                break;
            case U'*':
                if(commentFirstCond){
                    inComment = 1;
                    singleLineComment = 0;
                }else{
                    commentBreakCond = 1;
                }
                break;
            case U'x':
                if(octCond){
                    isHex = 1;
                    isNum = 1;
                }
                break;
            case U'0':
                octCond = 1;
                ATR_FALLTROUGHT;
            case U'1':
            case U'2':
            case U'3':
            case U'4':
            case U'5':
            case U'6':
            case U'7':
            case U'8':
            case U'9':
                isNum = 1;
                break;
            case U'a':
            case U'b':
            case U'c':
            case U'd':
            case U'e':
            case U'f':
                if(isHex)
                    isNum = 1;
                else goto DEFAULT;
                break;
            case U' ':
            case U':':
            case U'.':
                sep = 1;
                inScream = 0;
                break;
            default:
            DEFAULT:
                //isNum = isDigit(ci.arr[0]);
                if(!inComment && !inString){
                    if(oldInScream && (upOrSpecial(ci) || isDigit(ci))){
                        inScream = 1;
                    }else if(!oldInScream){
                        if(prevSep && upOrSpecial(ci)){
                            inScream = 1;
                            screamStartIndex = i;
                        }
                    }else{
                        inScream = 0;
                        if(!isParanthesis(ci))
                            memset(str->tokenId + screamStartIndex, TOKEN_NEUTRAL, str->len - screamStartIndex);
                    }
                }
                commentFirstCond = 0;
                commentBreakCond = 0;
                break;
        }
        if(!isNum) {
            octCond = 0;
            isHex = 0;
        }
    

        if(str->tokenId[i] != TOKEN_SEARCH_HIGHTLIGHT){
            if(inString || oldInString){
                str->tokenId[i] = TOKEN_STRING;
            }else if(inComment || oldInComment){
                str->tokenId[i] = TOKEN_COMMENT;
            }else if(isParanthesis(ci)){
                str->tokenId[i] = TOKEN_PARENTHESES;
            }else if(inScream){
                str->tokenId[i] = TOKEN_SCREAM;
            }else if(charGetByteCount(ci) == 1 && isNum){
                str->tokenId[i] = TOKEN_NUMBER;
            }else{
                str->tokenId[i] = TOKEN_NEUTRAL;
            }
    
            if(inComment && !oldInComment && i > 0) str->tokenId[i-1] = TOKEN_COMMENT;
        }
        
        oldInComment = inComment;
        oldInHashtag = inHashtag;
        oldInString = inString;
        prevSep = sep;
        oldInScream = inScream;
        wasNum = isNum;
    }

    // Checking for trailing whitespaces
    for(int i = str->len - 1; i >= 0; i--){
        wchar32 ci = GetCharAtPosU32(str, i);
        if(ci == U' ') str->tokenId[i] = TOKEN_TRAILING_WHITE;
        else break;
    }

    oldSingleLineComment = singleLineComment;
    if((str->len > 0) && str->data[str->len - 1] == U'\\') singleLineComment = 0;
    return (inComment && !singleLineComment) | ((oldSingleLineComment && inComment) << 1);
}

i32 SyntaxHighlightStringKeywordU32(string *strr, i32 flags){
    string_u32 *str = (string_u32*)strr;
    NullTerminateStringU32(str);

    /* Keyword highlighting */
    for(u32 j = 0; j < keyword1_u32_count; j++){
        i32 keyLen = StrlenU32(keyword1_u32[j]);

        char32_t *haystack = (char32_t*)str->data;
        char32_t *needle;
        do{
            needle = StrstrU32(haystack, keyword1_u32[j]);
            if(needle == NULL) break;

            if( ((needle == (char32_t*)str->data) || sepOrPar(needle[-1])) && sepOrPar(needle[keyLen]) ){
                u32 tokenStartOffset = needle - (char32_t*)str->data;
                memset(str->tokenId + tokenStartOffset, TOKEN_KEYWORD_1, keyLen);
            }

            haystack = needle + keyLen;
        }while(true);
    }

    for(u32 j = 0; j < keyword2_u32_count; j++){
        i32 keyLen = StrlenU32(keyword2_u32[j]);

        char32_t *haystack = (char32_t*)str->data;
        char32_t *needle;
        do{
            needle = StrstrU32(haystack, keyword2_u32[j]);
            if(needle == NULL) break;

            if( ((needle == (char32_t*)str->data) || sepOrPar(needle[-1])) && sepOrPar(needle[keyLen]) ){
                u32 tokenStartOffset = needle - (char32_t*)str->data;
                memset(str->tokenId + tokenStartOffset, TOKEN_KEYWORD_2, keyLen);
            }

            haystack = needle + keyLen;
        }while(true);
    }
    for(uint32 j = 0; j < preprocesorKeyword_u32_count; j++){
        i32 keyLen = StrlenU32(preprocesorKeyword_u32[j]);

        char32_t *haystack = (char32_t*)str->data;
        char32_t *needle;
        do{
            needle = StrstrU32(haystack, preprocesorKeyword_u32[j]);
            if(needle == NULL) break;

            if( ((needle == (char32_t*)str->data) || sepOrPar(needle[-1])) && sepOrPar(needle[keyLen]) ){
                u32 tokenStartOffset = needle - (char32_t*)str->data;
                memset(str->tokenId + tokenStartOffset, TOKEN_PREPROCESSOR, keyLen);
            }

            haystack = needle + keyLen;
        }while(true);
    }
    
    return flags;
}

i32 SyntaxForceU32(string *strr, i32 offset, token_type tokenId, i32 len){
    memset(strr->tokenId + offset, tokenId, len);
}


size_t StrlenU32(const char32_t *s){
  size_t len = 0;

    while(s[len] != L'\0'){
        if(s[++len] == L'\0')
            return len;
        if(s[++len] == L'\0')
            return len;
        if(s[++len] == L'\0')
            return len;
        ++len;
    }

  return len;
}

char32_t *StrstrU32(const char32_t *haystack, const char32_t *needle){
    register char32_t b, c;

    if((b = *needle) != U'\0'){
        haystack--;
        do{
	        if((c = *++haystack) == U'\0') goto ret0;
        }while(c != b);

        if(!(c = *++needle)) goto foundneedle;
        ++needle;
        goto jin;

        for(;;){
            register char32_t a;
            register const char32_t *rhaystack, *rneedle;

	        do{
	            if(!(a = *++haystack)) goto ret0;
	            if(a == b) break;
	            if((a = *++haystack) == U'\0') goto ret0;
                shloop:
	        }while(a != b);

            jin:	  
            if(!(a = *++haystack))
            goto ret0;

            if(a != c) goto shloop;

	        if(*(rhaystack = haystack-- + 1) == (a = *(rneedle = needle)))
	        do{
		        if(a == U'\0') goto foundneedle;
		        if(*++rhaystack != (a = *++needle)) break;
		        if(a == U'\0') goto foundneedle;
	        }while(*++rhaystack == (a = *++needle));

	        needle = rneedle;

	        if(a == U'\0')break;
	    }
    }

foundneedle:
    return (char32_t*)haystack;
ret0:
    return NULL;
}