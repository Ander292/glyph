#ifndef STRING_LIST_H
#define STRING_LIST_H

#include "string_buffer.h"

typedef enum list_type{
    LIST_TYPE_ASCII,
    LIST_TYPE_UTF32
} list_type;

typedef struct list_node{
    string *str;
    struct list_node *next;
    struct list_node *prev;
} list_node;

typedef struct string_list{
    list_node *head;
    list_node *tail;
    i32 size;
    list_type type;
} string_list;

typedef i32 list_lambda(string *, i32);

#define createList(t) (string_list){.type = (t)}
string_list CreateListWithRows(u32 initRowCount, list_type type);

void ListAppendEnd(string_list *list, string *string);
void ListInsertAtPossition(string_list *list, string *str, u32 index);
string *GetStringAtIndex(string_list *list, u32 index);

void PrintList(string_list *list);
void ListDeleteRow(string_list *list, u32 index);
void SwapStringsForIndexes(string_list *list, u32 ind1, u32 ind2);

void ClearList(string_list *list);
void FreeList(string_list *list);
void ListForeachString(string_list *list, list_lambda *funct);
void ListForeachStringEx(string_list *list, u32 startIndex, u32 maxLen, list_lambda *funct);

#endif /* STRING_LIST_H */