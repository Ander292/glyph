#include "string_list.h"
#include "system/io.h"
#include <stdlib.h>

static inline list_node *nodeCreate(){
    list_node *Result = malloc(sizeof(list_node));
    if(Result == NULL){
        Die("Error allocating list_node");
    }
    memset(Result, 0, sizeof(list_node));
    return Result;
}

void ListAppendEnd(string_list *list, string *str){
    list->size++;

    list_node *p = nodeCreate();
    p->str = str;
    p->next = NULL;
    p->prev = NULL;

    if(list->head == NULL || list->tail == NULL) {
        list->head = p;
        list->tail = p;
    }else{
        list_node *q = list->tail;
        q->next = p;
        p->prev = q;
        list->tail = p;
    }
}

// TODO: Optimize to go from the closest end
void ListInsertAtPossition(string_list *list, string *str, u32 index){
    if(index < 0) return;
    u32 oldSize = list->size;
    list_node *p = nodeCreate();
    p->str = str;

    if(index >= oldSize){
        u32 loopCount = index - oldSize;
        for(u32 i = 0; i < loopCount; i++){
            switch(list->type){
                case LIST_TYPE_ASCII: 
                    ListAppendEnd(list, (string*)StringCreateHeapA(64));
                    break;
                case LIST_TYPE_UTF32: 
                    ListAppendEnd(list, (string*)StringCreateHeapU32(64));
                    break;
                default: Die("Invalid list type");
            }
        }
        ListAppendEnd(list, str);
    }
    else{
        list->size++;
        list_node *q = list->head;
        for(u32 i = 0;(i < index); q = q->next, i++);

        list_node *temp = q->prev; // Inserting before
        if(temp == NULL){
            p->next = list->head;
            list->head->prev = p;
            list->head = p;
        }else{
            temp->next = p;
            p->prev = temp;

            p->next = q;
            q->prev = p;
        }
    }
}

// TODO: Optimize to go from the closest end
string *GetStringAtIndex(string_list *list, u32 index){
    if(index < 0 || index >= list->size) return NULL;
    list_node *q = list->head;
    for(u32 i = 0;(i < index) && (q != NULL); q = q->next, i++);

    if(q == NULL) Die("(getStringAtIndex) q was NULL");
    return q->str;
}

static list_node *getNodeAtIndex(string_list *list, u32 index){
    if(index < 0 || index >= list->size) return NULL;
    list_node *q = list->head;
    for(u32 i = 0;(i < index) && (q != NULL); q = q->next, i++);

    if(q == NULL) Die("(getNodeAtIndex) q was NULL");
    return q;
}

void SwapStringsForIndexes(string_list *list, u32 ind1, u32 ind2){
    list_node *first = getNodeAtIndex(list, ind1);
    list_node *second = getNodeAtIndex(list, ind2);

    string *temp = first->str;
    first->str = second->str;
    second->str = temp;
}

// TODO: Actually implement this function
void PrintList(string_list *list){
    Die("printList() is not implemented yet");
    for(list_node *p = list->head; p != NULL; p = p->next){
        //printf("%s\n", p->str->unknown);
    }
}

void FreeList(string_list *list){
    list_node *p = list->head;

    while(p != NULL){
        list_node *q = p->next;
        StringFreeHeap(p->str);
        free(p);
        p = q;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = -1;
}

void ClearList(string_list *list){
    for(list_node *p = list->head; p != NULL; p = p->next){
        ClearBuffer(p->str);
    }
}

void ListCreateLineAtIndex(string_list *list, u32 index, const char *str, u32 size){
    string *line;
    switch(list->type){
        case LIST_TYPE_ASCII: 
            line = (string*)BufferCreateFromStringA(str, size);
            break;
        case LIST_TYPE_UTF32: 
            line = (string*)BufferCreateFromStringU32(str, size);
            break;
        default: Die("Invalid list type");
    }
    ListInsertAtPossition(list, line, index);
}

void ListDeleteRow(string_list *list, u32 index){
    if(index < 0 || index >= list->size) return;
    list_node *q = list->head;
    for(int i = 0;(i < index) && (q != NULL); q = q->next, i++);

    list_node *prevN = q->prev;
    list_node *nextN = q->next;

    if(prevN == NULL) list->head = nextN;
    else prevN->next = nextN;
    if(nextN == NULL) list->tail = prevN;
    else nextN->prev = prevN;

    list->size--;

    StringFreeHeap(q->str);
    free(q);
}

string_list CreateListWithRows(u32 initRowCount, list_type type){
    string_list Result = createList(type);
    
    for(u32 i = 0; i < initRowCount; i++){
        switch(type){
            case LIST_TYPE_ASCII: 
                ListAppendEnd(&Result, (string*)StringCreateHeapA(64));
                break;
            case LIST_TYPE_UTF32: 
                ListAppendEnd(&Result, (string*)StringCreateHeapU32(64));
                break;
            default: Die("Invalid list type");
        }
        
    }

    return Result;
}

void ListForeachString(string_list *list, list_lambda *funct){
    i32 retVal = 0;
    for(list_node *p = list->head; p != NULL; p = p->next){
        retVal = funct(p->str, retVal);
    }
}

void ListForeachStringEx(string_list *list, u32 startIndex, u32 maxLen, list_lambda *funct){
    i32 retVal = 0;
    list_node *p = getNodeAtIndex(list, startIndex);
    for(int i = 0; i < maxLen && p != NULL; p = p->next){
        retVal = funct(p->str, retVal);
    }
}

static void listForeachNode(string_list *list, void (*funct)(list_node *)){
    for(list_node *p = list->head; p != NULL; p = p->next){
        funct(p);
    }
}