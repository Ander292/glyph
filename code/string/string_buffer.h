#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H

#include "types.h"
#include "escape.h"
#include <limits.h>
#include <uchar.h>


typedef struct string_a{
    char *data;
    i8 *tokenId;
    u32 len;
    u32 maxSize;
} string_a;

typedef struct string_u32{
    wchar32 *data;
    i8 *tokenId;
    u32 len;
    u32 maxSize;
} string_u32;

typedef union string{
    byte d[MAX_VAL(sizeof(string_u32), sizeof(string_a))];
    struct{
        void *unknown;
        i8 *tokenId;
        u32 len;
        u32 maxSize;
    };
} string;

typedef enum token_type{
    TOKEN_NEUTRAL,
    TOKEN_NUMBER,
    TOKEN_PREPROCESSOR,
    TOKEN_STRING,
    TOKEN_COMMENT,
    TOKEN_KEYWORD_1,
    TOKEN_KEYWORD_2,
    TOKEN_PARENTHESES,
    TOKEN_TRAILING_WHITE,
    TOKEN_SCREAM,
    TOKEN_SEARCH_HIGHTLIGHT
} token_type;

typedef enum insertion_flags{
    INS_MODE_OVERWRITE = 1,     // if set will overwrite chars instead of shifting
    INS_MODE_INCREASE_SIZE = 2  // will allow characters to be inserted beyond the [0, len] range
} insertion_flags;

extern char *token_escape[];

#define isSeparator(c) ((c) == ' ' || (c) == 0 || (c) == ':' || (c) == '.' || (c) == ';')
#define isParanthesis(c) ((c) == '(' || (c) == ')' || (c) == '{' || (c) == '}' || (c) == '[' || (c) == ']')
#define sepOrPar(c) (isSeparator(c) || isParanthesis(c))
#define isUppercase(c) ((c) >= 'A' && (c) <= 'Z')
#define isSpecial(c) ((c) == '_' || (c) == '$')
#define isDigit(c) ((c) >= '0' && (c) <= '9')
#define upOrSpecial(c) (isUppercase(c) || isSpecial(c))

/* universal functions */
void ClearBuffer(void *str);
void StringFree(void *str);
void StringFreeHeap(void *str);

/* string_buffer_a */
string_a StringCreateA(u32 size);
void StringAppendEndA(string_a *str, const char *c, u32 size);
void OverrunBufferWithA(string_a *str, const char *c, u32 size);
// void ClearBufferA(string_a *str);
string_a *StringCreateHeapA(u32 size);
string_a *BufferCreateFromStringA(const char *c, u32 size);
// void StringFreeA(string_a *str);
// void StringFreeHeapA(string_a *str);

#define ClearBufferA(str) ClearBuffer(str);
#define StringFreeA(str) StringFree(str);
#define StringFreeHeapA(str) StringFreeHeap(str);

static inline char GetCharAtPosA(string_a *str, u32 index){
    if(index < str->len)
        return str->data[index];
    else
        return 0;
}

static inline i8 GetTokenAtPosA(string_a *str, u32 index){
    if(index < str->len)
        return str->tokenId[index];
    else
        return -1;
}

bool32 InsertCharAtPosA(string_a *str, char c, u32 index, u32 insertionMode);
bool32 DeleteCharFromPosA(string_a *str, u32 pos);
bool32 TerminateStringOnPosA(string_a *str, u32 pos);

string_a *CreateCopyA(string_a *str, u32 startOffest, u32 endOffset);

#define PrintBufferA(buffer) writeOutput((buffer).data, (buffer).len);
#define WriteToBufferA(buffer, str) StringAppendEndA((buffer), (str), strlen(str))

/* string_buffer_u32 */
string_u32 StringCreateU32(u32 size);

void StringAppendEndU32(string_u32 *str, const char *c, u32 size);
void OverrunBufferWithU32(string_u32 *str, const char *c, u32 size);

//void ClearBufferU32(string_u32 *str);
string_u32 *StringCreateHeapU32(u32 size);
string_u32 *BufferCreateFromStringU32(const char *c, u32 size);

//void StringFreeU32(string_u32 *str);
//void StringFreeHeapU32(string_u32 *str);

#define ClearBufferA(str) ClearBuffer(str);
#define StringFreeA(str) StringFree(str);
#define StringFreeHeapA(str) StringFreeHeap(str);

static inline wchar32 GetCharAtPosU32(string_u32 *str, u32 index){
    if(index < str->len)
        return str->data[index];
    else
        return (wchar32){0};
}

static inline i8 GetTokenAtPosU32(string_u32 *str, u32 index){
    if(index < str->len)
        return str->tokenId[index];
    else
        return -1;
}

bool32 InsertCharAtPosU32(string_u32 *str, wchar32 c, u32 index, u32 insertionMode);
bool32 DeleteCharFromPosU32(string_u32 *str, u32 pos);
bool32 TerminateStringOnPosU32(string_u32 *str, u32 pos);

string_u32 *CreateCopyU32(string_u32 *str, u32 startOffest, u32 endOffset);

void TranslateToUtf8(string_a *dest, string_u32 *src);
void PrintBufferU32(string_u32 *str);
#define WriteToBufferU32(buffer, str) StringAppendEndU32((buffer), (str), strlen(str))

i32 ResetHighlight(string *str, i32 unused);
i32 SyntaxHighlightString(string *str, i32 flags);
i32 SyntaxHighlightStringKeyword(string *str, i32 flags);
i32 SyntaxForce(string *str, i32 offset, token_type tokenId, i32 len);

#define SYNTAX_MULTILINE_COMMENT 0x1
#define SYNTAX_WAS_EXTENDED 0x2

#endif /* STRING_BUFFER_H */
