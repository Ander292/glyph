#ifndef STRING_LIST_H
#define STRING_LIST_H

#include "string_buffer.h"

typedef struct string{
    byte d[MAX_VAL(sizeof(string_u32), sizeof(string_a))];
} string;

typedef struct list_node{
    string *str;
    struct list_node *next;
    struct list_node *prev;
} list_node;

typedef struct string_list{
    list_node *head;
    list_node *tail;
    u32 size;
};

#endif /* STRING_LIST_H */