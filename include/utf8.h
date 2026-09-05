#ifndef UTF8_H
#define UTF8_H

#include <limits.h>
#include "system.h"

typedef struct string{
    char *data;         /* The string itself */
    char *byteCount;    /* The sizes in bytes of each character */
    char *tokenId;      /* Used for syntax coloring */
    int len;            /* The length of the string in bytes */
    int byteLen;        /* The ammount of codepoints currently in data */
    int maxSize;        /* The size of 'byteCount' array. The 'data' array is always 4 times larger than byte size */
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

extern char *token_escape[];

typedef struct character{
    char arr[4];
    int byteCount;
    char tokenId;
}character;

#define isSeparator(c) ((c) == ' ' || (c) == 0 || (c) == ':' || (c) == '.' || (c) == ';')
#define isParanthesis(c) ((c) == '(' || (c) == ')' || (c) == '{' || (c) == '}' || (c) == '[' || (c) == ']')
#define sepOrPar(c) (isSeparator(c) || isParanthesis(c))
#define isUppercase(c) ((c) >= 'A' && (c) <= 'Z')
#define isSpecial(c) ((c) == '_' || (c) == '$')
#define isDigit(c) ((c) >= '0' && (c) <= '9')
#define upOrSpecial(c) (isUppercase(c) || isSpecial(c))

string stringCreate(int size);
void doubleSize(string *str);
void stringAppendEnd(string *str, const char *c, int size);
void clearBuffer(string *str);
string *stringCreateHeap(int size);
string *bufferCreateFromString(const char *c, int size);
void stringFree(string *str);
void stringFreeHeap(string *str);

int stringCharToByteCount(string *str, int startOffset, int endOffset, int charCount, int *outStartOffset);
int stringByteToCharCount(string *str, int startOffsetInBytes, int endOffsetInBytes, int maxByteCount, int *outStartOffsetInChars);

character_input getCharAtPos(string *str, int index);
character getCharAtPosEx(string *str, int index);

// void shiftStringRight(char *str, int len);
// void shiftStringLeft(char *str, int len);
// void shiftStringUtf8Right(string *str, int pos, int shiftCountInChars);
void insertCharAtPossition(string *str, character_input ci, int pos, int insertMode);
void deleteCharFromPossition(string *str, int pos);

void terminateStringOnPos(string *str, int pos);

string *createCopy(string *src, int startCharOffset, int charCount);

#define PrintBuffer(buffer) writeOutput((buffer).data, (buffer).byteLen)
#define WriteToBuffer(buffer, str) stringAppendEnd((buffer), (str), strlen(str))

/* String list */

typedef struct list_node{
    string *str;
    struct list_node *next;
    struct list_node *prev;
} list_node;

typedef struct string_list{
    list_node *head;
    list_node *tail;
    int size;
} string_list;

#define createList() (string_list){0}
void listAppendEnd(string_list *list, string *str);
void listInsertAtPossition(string_list *list, string *str, int index);
string *getStringAtIndex(string_list *list, int index);
void printList(string_list *list);
void listDeleteRow(string_list *list, int index);
void swapStringsForIndexes(string_list *list, int ind1, int ind2);
string_list createListWithRows(int initRowCount);
void clearList(string_list *list);
void freeList(string_list *list);
void listForeachString(string_list *list, int (*funct)(string *, int flags));
void listForeachStringEx(string_list *list, int startIndex, int maxLen, int (*funct)(string *, int flags));

/*** Syntax coloring ***/
static inline int resetHighlight(string *str, int unused){
    memset(str->tokenId, TOKEN_NEUTRAL, str->len);
    return 0;
}

int syntaxHighlightString(string *str, int flags);
int syntaxHighlightStringKeyword(string *str, int flags);
static inline void syntaxForce(string *str, int offset, token_type tokenId, int len){
    memset(str->tokenId + offset, tokenId, len);
}

#define SYNTAX_MULTILINE_COMMENT 0x1
#define SYNTAX_WAS_EXTENDED 0x2

#endif