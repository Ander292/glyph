#ifndef UTF8_H
#define UTF8_H

#include "system.h"

typedef struct string{
    char *data;         /* The string itself */
    char *byteCount;    /* The sizes in bytes of each character */
    int len;            /* The length of the string in bytes */
    int byteLen;        /* The ammount of codepoints currently in data */
    int maxSize;        /* The size of 'byteCount' array. The 'data' array is always 4 times larger than byte size */
} string;

string stringCreate(int size);
void doubleSize(string *str);
void stringAppendEnd(string *str, const char *c, int size);
void clearBuffer(string *str);
string *stringCreateHeap(int size);

int stringCharToByteCount(string *str, int startOffset, int endOffset, int charCount, int *outStartOffset);

void shiftStringRight(char *str, int len);
void insertCharAtPossition(string *str, character_input ci, int pos, int insertMode);

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

#endif