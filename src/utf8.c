#include "utf8.h"
//#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Allocates the memory for the string. Size must be > 0 or undefined behavior!!!*/
string stringCreate(int size){
    string Result;
    Result.len = 0;
    Result.byteLen = 0;
    Result.maxSize = size;

    Result.byteCount = malloc(size);
    Result.data = malloc(size * 4);
    Result.tokenId = malloc(size);
    memset(Result.tokenId, 0, size);

    if(!Result.byteCount){
        die("Error allocating %zu bytes", size);
    }
    if(!Result.data){
        die("Error allocating %zu bytes", size * 4);
    }

    *Result.byteCount = 0;
    *Result.data = 0;

    return Result;
}

void stringFree(string *str){
    free(str->byteCount);
    free(str->data);
    free(str->tokenId);
    str->byteCount = NULL;
    str->data = NULL;

    str->len = 0;
    str->maxSize = -1;
    str->byteLen = -1;
}

string *stringCreateHeap(int size){
    string *Result = malloc(sizeof(string));
    if(!Result){
        die("Error allocating %zu bytes", sizeof(string));
    }
    Result->len = 0;
    Result->byteLen = 0;
    Result->maxSize = size;

    Result->byteCount = malloc(size);
    Result->data = malloc(size * 4);
    Result->tokenId = malloc(size);
    memset(Result->tokenId, TOKEN_NEUTRAL, size);

    if(!Result->byteCount){
        die("Error allocating %zu bytes", size);
    }
    if(!Result->data){
        die("Error allocating %zu bytes", size * 4);
    }

    *Result->byteCount = 0;
    *Result->data = 0;

    return Result;
}

void stringFreeHeap(string *str){
    free(str->tokenId);
    free(str->byteCount);
    free(str->data);
    free(str);
}

void doubleSize(string *str){
    str->maxSize <<= 2;
    str->byteCount = realloc(str->byteCount, str->maxSize);
    if(!str->byteCount){
        die("Error reallocating %zu bytes", str->maxSize);
    }
    str->data = realloc(str->data, str->maxSize * 4);
    if(!str->data){
        die("Error reallocating %zu bytes", str->maxSize * 4);
    }

    str->tokenId = realloc(str->tokenId, str->maxSize);
    if(!str->tokenId){
        die("Error reallocating %zu bytes", str->tokenId);
    }
}

void stringAppendEnd(string *str, const char *c, int size){
    while(str->len + size >= str->maxSize) doubleSize(str);

    int pos = 0;
    while(pos < size){
        int byteCount = charGetByteCount(c[pos]);
        if(byteCount == 0) break;
        str->byteCount[str->len]= byteCount;
        str->tokenId[str->len] = TOKEN_NEUTRAL;        
        
        int j = 0;
        for(; j < byteCount; j++){
            str->data[str->byteLen++] = c[pos + j];
        }
        str->data[str->byteLen] = 0;
        pos += byteCount;
        str->len += 1;
    }
    str->byteCount[str->len] = 0;
    str->tokenId[str->len] = 0;

    if(pos > size * 4) die("Fatal error. Pos: %d > Size: %d", pos, size);
}

int stringCharToByteCount(string *str, int startOffset, int endOffset, int maxCharCount, int *outStartOffset){
    if(maxCharCount == 0) maxCharCount = INT_MAX / 2;
    int Result = 0;
    if(startOffset > str->len) return -1;

    int byteCountToReachSubstring = 0;
    for(int i = 0; i < startOffset; i++){
        byteCountToReachSubstring += str->byteCount[i];
    }
    *outStartOffset = byteCountToReachSubstring;

    for(int i = startOffset; (i < str->len - endOffset) && (i < maxCharCount + startOffset); i++){
        Result += str->byteCount[i];
    }

    // The loop counting bytes should by capped by maxCharCount not this!!!
    //if(Result > maxCharCount) Result = maxCharCount;
    return Result;
}

character_input getCharAtPos(string *str, int index){
    int bytesPassed = 0;
    character_input Result = {0};
    for(int i = 0; i < index; i++){
        bytesPassed += str->byteCount[i];
    }
    Result.byteCount = str->byteCount[index];
    memcpy(Result.arr, str->data + bytesPassed, Result.byteCount);

    return Result;
}

void clearBuffer(string *str){
    *str->byteCount = 0;
    *str->tokenId = 0;
    *str->data = 0;
    str->len = 0;
    str->byteLen = 0;    
}

string *bufferCreateFromString(const char *c, int size){
    string *Result = stringCreateHeap(MAX_VAL(64, powerOfTwoRoundUp(size)));
    stringAppendEnd(Result, c, size);
    return Result;
}

void terminateStringOnPos(string *str, int pos){
    int startOffsetInBytes = 0;
    int byteCount = stringCharToByteCount(str, pos, 0, 0, &startOffsetInBytes);

    memset(str->data + startOffsetInBytes, 0, byteCount);
    memset(str->byteCount + pos, 0, str->len - pos);
    memset(str->tokenId + pos, 0, str->len - pos);
    str->len = pos;
    str->byteLen = startOffsetInBytes;

    // str->data[str->byteLen] = 0;
    // str->byteCount[str->len] = 0;
}

static inline void shiftStringRight(char *str, int len){
#if 0
    for(int i = len - 1; i > 0; i--){
        str[i] = str[i-1];
    }
    *str = '~';
#else
    memmove(str + 1, str, len - 1);
#endif
}

static inline void shiftStringLeft(char *str, int len){
#if 0
    for(int i = 0; i < len - 1; i++){
        str[i] = str[i+1];
    }
    str[len-1] = '~';
#else
    memmove(str, str + 1, len - 1);
#endif
}

#if 0
void shiftStringUtf8Right(string *str, int startPossition, int lengthInChars, int shiftCountInChars){
    int endOffsetInChars = str->len - lengthInChars;
    int startPossitionInBytes;
    int lengthInBytes = stringCharToByteCount(str, startPossition, endOffsetInChars, 0, &startPossitionInBytes);
    
    for(int i = 0; i < shiftCountInChars; i++){
        
        int dataShiftCount = str->byteCount[lengthInChars];
        shiftStringRight(str->byteCount + startPossition, lengthInChars);
    }
}
#endif

void deleteCharFromPossition(string *str, int pos){
    int posInBytes = 0;
    int byteCount = str->byteCount[pos];
    stringCharToByteCount(str, pos, 0, 0, &posInBytes);

    shiftStringLeft(str->byteCount + pos, str->len - pos);
    shiftStringLeft(str->tokenId + pos, str->len - pos);
    str->len--;

    for(int i = 0; i < byteCount; i++){
        shiftStringLeft(str->data + posInBytes, str->byteLen - posInBytes);
        str->byteLen--;
    }

    str->byteCount[str->len] = 0;
    str->data[str->byteLen] = 0;
}

/* The cancer itself */
void insertCharAtPossition(string *str, character_input ci, int pos, int insertMode){
    if(str->maxSize <= str->len + 1) doubleSize(str);

    int shiftCount = 0;

    if(pos == str->len){
        stringAppendEnd(str, ci.arr, ci.byteCount);
    }else if(pos > str->len){
        for(int i = 0; i < pos - str->len; i++){
            stringAppendEnd(str, " ", 1);
        }
        stringAppendEnd(str, ci.arr, ci.byteCount);
    }else{
        int byteCount = str->byteCount[pos];
        int startOffset; // The offset from str->data at which the selected character starts
        shiftCount += byteCount;
        
        stringCharToByteCount(str, pos, 0, 0, &startOffset);
        
        if(insertMode){
            shiftStringRight(str->byteCount + pos, str->len - pos + 1);
            shiftStringRight(str->tokenId + pos, str->len - pos + 1);

            for(int j = 0; j < ci.byteCount; j++){
                shiftStringRight(str->data + startOffset, str->byteLen - startOffset + 1);
            }
    
            for(int i = 0; i < ci.byteCount; i++){
                str->data[startOffset + i] = ci.arr[i];
            }
            str->len++;
            str->byteLen += ci.byteCount;
        }
        else{
            int byteDiff = ci.byteCount - byteCount;
            if(byteDiff < 0){
                byteDiff *= -1;
                for(int i = 0; i < byteDiff; i++){
                    shiftStringLeft(str->data + startOffset, str->byteLen - startOffset + 1);
                }
                
                for(int i = 0; i < ci.byteCount; i++){
                    str->data[startOffset + i] = ci.arr[i];
                }
    
                str->byteLen = str->byteLen - byteCount + ci.byteCount;
            }else if(byteDiff > 0){
                for(int i = 0; i < byteDiff; i++){
                    shiftStringRight(str->data + startOffset, str->byteLen - startOffset + 1);
                }
    
                for(int i = 0; i < ci.byteCount; i++){
                    str->data[startOffset + i] = ci.arr[i];
                }
                str->byteLen = str->byteLen - byteCount + ci.byteCount;
            }else{
                for(int i = 0; i < ci.byteCount; i++){
                    str->data[startOffset + i] = ci.arr[i];
                }
            }
        }
    
        str->byteCount[pos] = ci.byteCount;
        str->byteCount[str->len] = 0;
        str->data[str->byteLen] = 0;
    }

}

string *createCopy(string *src, int startCharOffset, int charCount){
    if(charCount <= 0) return NULL;
    int startOffsetInBytes;
    int byteCount = stringCharToByteCount(src, startCharOffset, 0, charCount, &startOffsetInBytes);
    if(byteCount == -1) return NULL;

    string *Result = stringCreateHeap(powerOfTwoRoundUp(src->maxSize));
    Result->byteLen = src->byteLen - startOffsetInBytes;
    Result->len = src->len - startCharOffset;
    Result->maxSize = src->maxSize;

    memcpy(Result->data, src->data + startOffsetInBytes, byteCount);
    memcpy(Result->byteCount, src->byteCount + startCharOffset, charCount);
    memcpy(Result->tokenId, src->tokenId + startCharOffset, charCount);

    return Result;
}


/*** String List ***/

static inline list_node *nodeCreate(){
    list_node *Result = malloc(sizeof(list_node));
    memset(Result, 0, sizeof(list_node));
    return Result;
}

void listAppendEnd(string_list *list, string *str){
    list->size++;

    list_node *p = nodeCreate();
    p->str = str;
    p->next = NULL;
    p->prev = NULL;

    if(list->head == NULL || list->tail == NULL) {
        list->head = p;
        list->tail = p;
    }
    else{
        list_node *q = list->tail;
        q->next = p;
        p->prev = q;
        list->tail = p;
    }
}

void listInsertAtPossition(string_list *list, string *str, int index){
    if(index < 0) return;
    int oldSize = list->size;
    list_node *p = nodeCreate();
    p->str = str;

    if(index >= oldSize){
        int loopCount = index - oldSize;
        for(int i = 0; i < loopCount; i++){
            listAppendEnd(list, stringCreateHeap(64));
        }
        listAppendEnd(list, str);
    }
    else{
        list->size++;
        list_node *q = list->head;
        for(int i = 0;(i < index); q = q->next, i++){
            // if(q->next == NULL){
            //     listAppendEnd(list, stringCreateHeap(64));
            //     list->size++;
            // }
        }

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

string *getStringAtIndex(string_list *list, int index){
    if(index < 0 || index >= list->size) return NULL;
    list_node *q = list->head;
    for(int i = 0;(i < index) && (q != NULL); q = q->next, i++);

    if(q == NULL) die("(getStringAtIndex) q was NULL");
    return q->str;
}

static list_node *getNodeAtIndex(string_list *list, int index){
    if(index < 0 || index >= list->size) return NULL;
    list_node *q = list->head;
    for(int i = 0;(i < index) && (q != NULL); q = q->next, i++);

    if(q == NULL) die("(getStringAtIndex) q was NULL");
    return q;
}

void swapStringsForIndexes(string_list *list, int ind1, int ind2){
    list_node *first = getNodeAtIndex(list, ind1);
    list_node *second = getNodeAtIndex(list, ind2);

    string *temp = first->str;
    first->str = second->str;
    second->str = temp;
}

void printList(string_list *list){
    for(list_node *p = list->head; p != NULL; p = p->next){
        printf("%s\n", p->str->data);
    }
}

void freeList(string_list *list){
    list_node *p = list->head;

    while(p != NULL){
        list_node *q = p->next;
        stringFreeHeap(p->str);
        free(p);
        p = q;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = -1;
}

void clearList(string_list *list){
    for(list_node *p = list->head; p != NULL; p = p->next){
        clearBuffer(p->str);
    }
}

void listCreateLineAtIndex(string_list *list, int index, const char *str, int size){
    string *line = bufferCreateFromString(str, size);
    listInsertAtPossition(list, line, index);
}

void listDeleteRow(string_list *list, int index){
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

    stringFreeHeap(q->str);
    free(q);
}

string_list createListWithRows(int initRowCount){
    string_list Result = createList();
    for(int i = 0; i < initRowCount; i++){
        listAppendEnd(&Result, stringCreateHeap(64));
    }

    return Result;
}

void listForeachString(string_list *list, void (*funct)(string *)){
    for(list_node *p = list->head; p != NULL; p = p->next){
        funct(p->str);
    }
}

void listForeachNode(string_list *list, void (*funct)(list_node *)){
    for(list_node *p = list->head; p != NULL; p = p->next){
        funct(p);
    }
}