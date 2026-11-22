#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H

#include "types.h"
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

extern char *token_escape[];

#define isSeparator(c) ((c) == ' ' || (c) == 0 || (c) == ':' || (c) == '.' || (c) == ';')
#define isParanthesis(c) ((c) == '(' || (c) == ')' || (c) == '{' || (c) == '}' || (c) == '[' || (c) == ']')
#define sepOrPar(c) (isSeparator(c) || isParanthesis(c))
#define isUppercase(c) ((c) >= 'A' && (c) <= 'Z')
#define isSpecial(c) ((c) == '_' || (c) == '$')
#define isDigit(c) ((c) >= '0' && (c) <= '9')
#define upOrSpecial(c) (isUppercase(c) || isSpecial(c))

string_a StringCreateA(u32 size);
//void doubleSizeA(string_a *str);
void StringAppendEndA(string_a *str, const char *c, u32 size);
void OverrunBufferWithA(string_a *str, const char *c, u32 size);
void ClearBufferA(string_a *str);
string_a *StringCreateHeapA(u32 size);
string_a *BufferCreateFromStringA(const char *c, u32 size);
void StringFreeA(string_a *str);
void StringFreeHeapA(string_a *str);

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

typedef enum insertion_flags{
    INS_MODE_OVERWRITE = 1,     // if set will overwrite chars instead of shifting
    INS_MODE_INCREASE_SIZE = 2  // will allow characters to be inserted beyond the [0, len] range
} insertion_flags;

bool32 InsertCharAtPosA(string_a *str, char c, u32 index, u32 insertionMode);
bool32 DeleteCharFromPosA(string_a *str, u32 pos);
bool32 TerminateStringOnPos(string_a *str, u32 pos);

string_a *CreateCopyA(string_a *str, u32 startOffest, u32 endOffset);

#define PrintBufferA(buffer) writeOutput((buffer).data, (buffer).len);
#define WriteToBufferA(buffer, str) StringAppendEndA((buffer), (str), strlen(str))

#endif /* STRING_BUFFER_H */
