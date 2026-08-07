#include "utf8.h"
//#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *token_escape[] = {
    ESC_RESET_TEXT_ATTRIBUTES,
    ESC_TEXT_COLOR_BLUE,
    ESC_TEXT_COLOR_MAGENTA,
    ESC_TEXT_COLOR_YELLOW_BRIGHT,
    ESC_TEXT_COLOR_GREEN_BRIGHT,
    ESC_TEXT_COLOR_GREEN,
    ESC_TEXT_COLOR_CYAN,
    ESC_TEXT_COLOR_CYAN,
    ESC_BACKGROUND_COLOR_GREEN_BRIGHT
    ESC_TEXT_COLOR_RED_BRIGHT
};

static char *keyword1[] = {
    "while",
    "for",
    "if",
    "else",
    "switch",
    "case",
    "default",
    "return",
    "break",
    "continue"
};

static char *keyword2[] = {
    "int",
    "short",
    "char",
    "long",
    "unsigned",
    "signed",
    "static",
    "void",
    "float",
    "double",
    "inline",
    "auto",
    "const",

    "int8",
    "int16",
    "int32",
    "int64",
    "uint8",
    "uint16",
    "uint32",
    "uint64",

    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "size_t"
};

static char *preprocesorKeyword[] = {
    "#define",
    "#include",
    "#undef",
    "#ifdef",
    "#ifndef",
    "#else",
    "#elif",
    "#region",
    "#endregion",
    "#pragma",
    "#endif",
    "#warning",
    "#error"
};

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
    //syntaxHighlightString(str);
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

character getCharAtPosEx(string *str, int index){
    int bytesPassed = 0;
    character Result = {0};
    for(int i = 0; i < index; i++){
        bytesPassed += str->byteCount[i];
    }
    Result.byteCount = str->byteCount[index];
    Result.tokenId = str->tokenId[index];
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

    //syntaxHighlightString(str);

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
    // syntaxHighlightString(str);
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
    //syntaxHighlightString(str);
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


/*** Syntax coloring ***/
int syntaxHighlightStringKeyword(string *str, int flags){
    for(int i = 0; i < str->len; i++){
        /* Keyword highlighting */
        int startOffsetInBytes;
        stringCharToByteCount(str, i, 0, 0, &startOffsetInBytes);
        for(uint32 j = 0; j < sizeof(keyword1) / sizeof(char*); j++){
            // int startOffsetInBytes;
            // stringCharToByteCount(str, i, 0, 0, &startOffsetInBytes);
            int keyLen = strlen(keyword1[j]);
            if(isSeparator(*(str->data + startOffsetInBytes + keyLen)) 
                && (isSeparator(*(str->data + startOffsetInBytes - 1)) || i == 0)
                && !strncmp(keyword1[j], str->data + startOffsetInBytes, keyLen)){
                memset(str->tokenId + i, TOKEN_KEYWORD_1, keyLen);
                i += keyLen;
                goto START;
            }
        }
        for(uint32 j = 0; j < sizeof(keyword2) / sizeof(char*); j++){
            // int startOffsetInBytes;
            // stringCharToByteCount(str, i, 0, 0, &startOffsetInBytes);
            int keyLen = strlen(keyword2[j]);
            if(isSeparator(*(str->data + startOffsetInBytes + keyLen)) 
                && (isSeparator(*(str->data + startOffsetInBytes - 1)) || i == 0)
                && !strncmp(keyword2[j], str->data + startOffsetInBytes, keyLen)){
                memset(str->tokenId + i, TOKEN_KEYWORD_2, keyLen);
                i += keyLen;
                goto START;
            }
        }
        for(uint32 j = 0; j < sizeof(preprocesorKeyword) / sizeof(char*); j++){
            // int startOffsetInBytes;
            // stringCharToByteCount(str, i, 0, 0, &startOffsetInBytes);
            int keyLen = strlen(preprocesorKeyword[j]);
            if(isSeparator(*(str->data + startOffsetInBytes + keyLen)) 
                && (isSeparator(*(str->data + startOffsetInBytes - 1)) || i == 0)
                && !strncmp(preprocesorKeyword[j], str->data + startOffsetInBytes, keyLen)){
                memset(str->tokenId + i, TOKEN_PREPROCESSOR, keyLen);
                i += keyLen;
                goto START;
            }
        }
        START:
    }
    return flags;
}
int syntaxHighlightString(string *str, int flags){
    resetHighlight(str, flags);
    // TODO: Merge all the int8s into one variable.
    int8 inComment = flags & SYNTAX_MULTILINE_COMMENT;
    int8 singleLineComment = flags & SYNTAX_WAS_EXTENDED;
    int8 sep = 0;
    int8 prevSep = 0;
    int8 inString = 0;
    int8 inHashtag = 0;

    int8 oldInComment = 0, oldInString = 0, oldInHashtag = 0, oldSingleLineComment = 0;

    int8 commentFirstCond = 0;
    int8 commentBreakCond = 0;

    for(int i = 0; i < str->len; i++){
        character_input ci = getCharAtPos(str, i);
        /* The first check to see if its a comment */
        switch(ci.arr[0]){
            case '"':
            case '\'':
                inString ^= 1;
                break;
            case '#':
                inHashtag = 1;
                break;
            case '/':
                if(commentFirstCond){
                    inComment = 1;
                    singleLineComment = 1;
                }else if(commentBreakCond){
                    inComment = 0;
                }else{
                    commentFirstCond = 1;
                }
                break;
            case '*':
                if(commentFirstCond){
                    inComment = 1;
                    singleLineComment = 0;
                }else{
                    commentBreakCond = 1;
                }
                break;
            case ' ':
                sep = 1;
                break;
            default:
                commentFirstCond = 0;
                commentBreakCond = 0;
                break;
        }

        if(inString || oldInString){
            str->tokenId[i] = TOKEN_STRING;
        }else if(inComment || oldInComment){
            str->tokenId[i] = TOKEN_COMMENT;
        }else if(ci.byteCount == 1 && *ci.arr >= '0' && *ci.arr <= '9'){
            str->tokenId[i] = TOKEN_NUMBER;
        }else{
            str->tokenId[i] = TOKEN_NEUTRAL;
        }

        if(inComment && !oldInComment && i > 0) str->tokenId[i-1] = TOKEN_COMMENT;
        
        oldInComment = inComment;
        oldInHashtag = inHashtag;
        oldInString = inString;
        prevSep = sep;
    }

    // Checking for trailing whitespaces
    for(int i = str->len - 1; i >= 0; i--){
        character_input ci = getCharAtPos(str, i);
        if(*ci.arr == ' ') str->tokenId[i] = TOKEN_TRAILING_WHITE;
        else break;
    }

    oldSingleLineComment = singleLineComment;
    if(str->data[str->byteLen - 1] == '\\') singleLineComment = 0;
    return (inComment && !singleLineComment) | ((oldSingleLineComment && inComment) << 1);
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

void listForeachString(string_list *list, int (*funct)(string *, int flags)){
    int retVal = 0;
    for(list_node *p = list->head; p != NULL; p = p->next){
        retVal = funct(p->str, retVal);
    }
}

void listForeachStringEx(string_list *list, int startIndex, int maxLen, int (*funct)(string *, int flags)){
    int retVal = 0;
    list_node *p = getNodeAtIndex(list, startIndex);
    for(int i = 0; i < maxLen && p != NULL; p = p->next){
        retVal = funct(p->str, retVal);
    }
}

void listForeachNode(string_list *list, void (*funct)(list_node *)){
    for(list_node *p = list->head; p != NULL; p = p->next){
        funct(p);
    }
}