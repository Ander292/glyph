#include "system/io.h"
#include "input_parser.h"
#include "main.h"
#include "res/res.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

static global_editor_state E = {0};
static string_a GlobalWriteBuffer = {0};
static console_info GlobalConsoleInfo = {0};


static string_a *editorPromtHeader(char *promt);
static inline bool32 deleteLocalEditorState(u8 ind);
static bool32 createLocalEditorState(u8 ind);
static inline bool32 resetLocalEditorState(u8 ind);
static inline bool32 switchEditorState(u8 ind);

static inline void syntaxHighlightGlobal(string_list *list){
    ListForeachString(list, SyntaxHighlightString);
    ListForeachString(list, SyntaxHighlightStringKeyword);
}

static inline void syntaxHighlight(string_list *list){
    ListForeachStringEx(list, MAX_VAL(EC.Cursor.Y - ROW_OVERHIGHLIGHT_COUNT, 0), GlobalConsoleInfo.Rows + ROW_OVERHIGHLIGHT_COUNT, SyntaxHighlightString);
    ListForeachStringEx(list, EC.Offset.Y, GlobalConsoleInfo.Rows, SyntaxHighlightStringKeyword);
}

static inline void switchEditorContext(u8 bufferID){
    SET_EDITOR_ID(bufferID);
    if(!TEST_FLAG_LOCAL(FLAG_INITIALIZED)){
        resetLocalEditorState(bufferID);
    }
}

static inline void insertCharAtPosList(string_list *List, wchar32 ci, int Row, int Col){
    assert(List->type == LIST_TYPE_UTF32);
    string_u32 *RowString = (string_u32*)GetStringAtIndex(List, Row);
    InsertCharAtPosU32(RowString, ci, Col, TEST_FLAG_GLOBAL(GLOBAL_FLAG_INSERT));
}

static inline void deleteCharFromPosList(string_list *List, int Row, int Col){
    assert(List->type == LIST_TYPE_UTF32);
    string_u32 *RowString = (string_u32*)GetStringAtIndex(List, Row);
    DeleteCharFromPosU32(RowString, Col);
}

/* Cursor */

static inline void messageCreate(editor_message *dest, time_t duration, const char *text, ...){
    va_list args;
    va_start(args, text);
    vsnprintf(dest->Data, sizeof(dest->Data), text, args);
    va_end(args);
    //strcpy(dest->Data, text);
    dest->PostTime = GlobalConsoleInfo.CurrentTime;
    dest->Duration = duration;
}

static inline void setCursorPossition(u32 x, u32 y){
    char c[16];
    x++; y++;

    u32 maxRows = GlobalConsoleInfo.Rows;
    u32 maxCols = GlobalConsoleInfo.Cols;

    x += E.TextOffsetLeft;
    y += E.TextOffsetTop;
    
    if(x > maxCols) x = maxCols;
    if(y > maxRows) y = maxRows;

    snprintf(c, sizeof(c), ESC_SEQ "%d;%dH", (y), (x));
    PrintA(c);
}

static void fixCursorPossition(){
    /* Fixing the internal cursor possition and the offsets */
    string *str = GetStringAtIndex(&EC.List, EC.Cursor.Y);
    int currentWidth = 0;
    if(str != NULL) currentWidth = str->len;
    int destWidth = MIN_VAL(GlobalConsoleInfo.Cols, currentWidth);

    if(EC.Cursor.X < 0){
        EC.Cursor.X = 0;
    }else if(EC.Cursor.X > currentWidth){
        EC.Cursor.X = currentWidth;
    }

    // Vertical offset
    if((EC.Cursor.Y - EC.Offset.Y > GlobalConsoleInfo.Rows - 1 - GLOBAL_STRUCT_NAME.TextOffsetTop) && EC.Cursor.Y < EC.List.size){
        EC.Offset.Y = MAX_VAL(0, (EC.Cursor.Y) - (GlobalConsoleInfo.Rows - GLOBAL_STRUCT_NAME.TextOffsetTop) + 1);
    }else if((EC.Cursor.Y - EC.Offset.Y < 0)){
        EC.Offset.Y = MAX_VAL(0, EC.Offset.Y - (EC.Offset.Y - EC.Cursor.Y));
    }

    // Horizontal offset
    if(EC.Cursor.X - EC.Offset.X > GlobalConsoleInfo.Cols - 1 - GLOBAL_STRUCT_NAME.TextOffsetLeft){
        EC.Offset.X = MAX_VAL(0, EC.Cursor.X - (GlobalConsoleInfo.Cols - GLOBAL_STRUCT_NAME.TextOffsetLeft) + 1);
    }else if((EC.Cursor.X - EC.Offset.X < 0)){
        EC.Offset.X = MAX_VAL(0, EC.Offset.X - (EC.Offset.X - EC.Cursor.X));
    }

    // Verticall offset clamping
    if(EC.Offset.Y < 0) EC.Offset.Y = 0;
#if 0
    if(EC.Offset.Y > EC.List->size - (GlobalConsoleInfo.Rows - EC.TextOffsetTop)) 
        EC.Offset.Y = EC.List->size - (GlobalConsoleInfo.Rows - EC.TextOffsetTop);
#else
    if(EC.Offset.Y > EC.List.size - 1) EC.Offset.Y = EC.List.size - 1;
#endif

    if(EC.Cursor.Y < 0){
        EC.Cursor.Y = 0;
    }else if(EC.Cursor.Y > EC.List.size - 1){
        EC.Cursor.Y = EC.List.size - 1;
    }

    if(!(GLOBAL_STRUCT_NAME.maxCursor.X < 0) && (EC.Cursor.X > GLOBAL_STRUCT_NAME.maxCursor.X)) EC.Cursor.X = GLOBAL_STRUCT_NAME.maxCursor.X;
    if(!(GLOBAL_STRUCT_NAME.maxCursor.Y < 0) && (EC.Cursor.Y > GLOBAL_STRUCT_NAME.maxCursor.Y)) EC.Cursor.Y = GLOBAL_STRUCT_NAME.maxCursor.Y;
}


/* Saving */

static int editorSave(char *path, string_list *src){
#define ROWS_TO_SAVE (src)

    if(*EC.File == 0){
        string_a *str = editorPromtHeader("Save location");
        if(str != NULL){
            strncpy(EC.File, str->data, sizeof(EC.File));
            StringFreeHeap(str);
        }else{
            return -1;
        }
    }

    //string_a Final = StringCreateA(1024); // TODO: Is this possible
    ClearBuffer(&GlobalWriteBuffer);

    u32 SizeCount = 0;
    for(i32 i = 0; i < ROWS_TO_SAVE->size; i++){
        string_u32 *row = (string_u32*)GetStringAtIndex(ROWS_TO_SAVE, i);
        // SizeCount += row->byteLen;
        // stringAppendEnd(&Final, row->data, row->byteLen);

        for(u32 j = 0; j < row->len; j++){
            wchar32 c = GetCharAtPosU32(row, j);
            u32 byteLen = charGetByteCount(*c.bytes);

            StringAppendEndA(&GlobalWriteBuffer, c.bytes, byteLen);
            SizeCount += byteLen;
        }

        if(i < ROWS_TO_SAVE->size - 1){
            if(TEST_FLAG_LOCAL(FLAG_NEWLINE_ENTER)){
                StringAppendEndA(&GlobalWriteBuffer, "\n", 1);
                SizeCount += 1;
            }else{
                StringAppendEndA(&GlobalWriteBuffer, "\r\n", 2);
                SizeCount += 2;
            }
        }
    }

    int64 Result = io_FileWrite(path, GlobalWriteBuffer.data, GlobalWriteBuffer.len);
    
    if(Result != SizeCount){
        postEditorMessage(messageDurationDefault, "Error saving file. E: %u, W: %u", SizeCount, Result);
    }else{
        postEditorMessage(messageDurationDefault, "File properly saved (Wrote %u bytes)", Result);
        CLEAR_FLAG_LOCAL(FLAG_DIRTY);
    }

    ClearBuffer(&GlobalWriteBuffer);

    return Result != SizeCount;
#undef ROWS_TO_SAVE
}

static void stringIntoInput(const char *str, int64 len){
    string_u32 *inputString = BufferCreateFromStringU32(str, len);

    i32 wasRdonly = TEST_FLAG_LOCAL(FLAG_RDONLY);
    CLEAR_FLAG_LOCAL(FLAG_RDONLY);

    for(i32 i = 0; i < inputString->len; i++){
        wchar32 c = GetCharAtPosU32(inputString, i);
        u32 byteCount = charGetByteCount(*c.bytes);

        read_result rr = {0};
        rr.size = byteCount;
        *(u32*)rr.bytes = *(u32*)c.bytes;

        parser_ParseInput(rr);
    }

    if(wasRdonly) SET_FLAG_LOCAL(FLAG_RDONLY);
}

static int editorLoadFile(const char *path){
    clearEditorMessage();

    int64 fileSize = io_GetFileSize(path);
    if(fileSize <= 0) {
        postEditorMessage(2*messageDurationDefault, "FileSize error: %s : %s", path, io_GetInputOutputErrorString());
        return 1;
    }

    char *tempBuffer = malloc(fileSize + 1);
    if(tempBuffer == NULL) Die("malloc fault (%zu)", fileSize);
    tempBuffer[fileSize] = 0;

    int64 Feedback = io_FileRead(path, tempBuffer, fileSize);
    if(Feedback <= 0){
        postEditorMessage(2*messageDurationDefault, "fileRead error: %s : %s", path, io_GetInputOutputErrorString());
        return 1;
    }

    if(Feedback != fileSize) 
        postEditorMessage(messageDurationDefault, "Possible load error (Exp: %u | Rec: %u)", fileSize, Feedback);

    
    if(!TEST_FLAG_LOCAL(FLAG_FORCE)){
        /* TODO: Check for file types */
    }

    switch(GET_LOCAL_FORMAT_NUMBER()){
        case FORMAT_UTF8:{
            for(int64 i = 0; i < Feedback; i++){
                if(tempBuffer[i] == '\n') {
                    SET_FLAG_LOCAL(FLAG_NEWLINE_ENTER);
                    break;
                }
                else if(tempBuffer[i] == '\r'){
                    CLEAR_FLAG_LOCAL(FLAG_NEWLINE_ENTER);
                    break;
                }
            }
            stringIntoInput(tempBuffer, Feedback);
            break;
        }
        case FORMAT_UTF16:
            break;
        case FORMAT_BIN:{
            //byteIntoInput(tempBuffer, Feedback);
            break;
        }
    }

    free(tempBuffer);
    EC.Cursor.X = 0;
    EC.Cursor.Y = 0;
    fixCursorPossition();
    CLEAR_FLAG_LOCAL(FLAG_DIRTY);
}

/* Rendering */

static void drawRows(string_a *buffer){
    debug_check(EC.List.type != LIST_TYPE_UTF32){
        Breakpoint
        Die("Invalid list type caught in drawRows()");
    }

    char rowNum[5];
    i32 maxRows = GlobalConsoleInfo.Rows;
    if(TEST_FLAG_GLOBAL(GLOBAL_FLAG_SHOWHEADER)) maxRows -= 1;

    i32 maxCols = GlobalConsoleInfo.Cols - 1;
    if(TEST_FLAG_GLOBAL(GLOBAL_FLAG_SHOWNUMBERS)) maxCols -= 3;

    /* Calculating verticall offset */
    i32 horizontalOffset = EC.Offset.X;
    i32 verticallOffset = EC.Offset.Y;

    i32 targetRows = MIN_VAL(EC.List.size - verticallOffset, maxRows);
    for(i32 i = 0; i < maxRows; i++){
        WriteToBufferA(buffer, ESC_CLEAR_LINE);
        if(i < targetRows){
            if(TEST_FLAG_GLOBAL(GLOBAL_FLAG_SHOWNUMBERS)){
                snprintf(rowNum, sizeof(rowNum), "%3d|", (i + verticallOffset + 1) % 1000);
                WriteToBufferA(buffer, rowNum);
            }else{
                WriteToBufferA(buffer, "~");
            }

            string *row = GetStringAtIndex(&EC.List, i + verticallOffset);
            if(row == NULL) Die("Row was null, im out");
#if 1
            //i32 startOffset = horizontalOffset;
            //(i < str->len - endOffset) && (i < maxCharCount + startOffset)

            i32 rowRemainingCharCount = (row->len - horizontalOffset);
            i32 targetCols = MIN_VAL(maxCols, rowRemainingCharCount);

            if(rowRemainingCharCount <= 0 && row->len != 0){
                WriteToBufferA(buffer, "<--");
            }else{
                byte prevTokenId = TOKEN_NEUTRAL;

                for(i32 j = 0; j < targetCols; j++){
                    wchar32 c = GetCharAtPosU32((string_u32*)row, horizontalOffset + j);
                    byte currentTokenId = GetTokenAtPosU32(row, horizontalOffset + j);

                    i32 byteCount = charGetByteCount(*c.bytes);

                    if(prevTokenId != currentTokenId){
                        prevTokenId = currentTokenId;
                        const char *escSeqInsert = token_escape[currentTokenId];
                        const i32 escSeqLen = strlen(escSeqInsert);
                        StringAppendEndA(buffer, escSeqInsert, escSeqLen);
                    }
                    StringAppendEndA(buffer, c.bytes, byteCount);
                }
                WriteToBufferA(buffer, ESC_RESET_TEXT_ATTRIBUTES);
            }

#else
#if 0
            /* Deciding how much to write */
            int startOffset;
            int byteCount = stringCharToByteCount(row, horizontalOffset, 0, maxCols, &startOffset);
            if(byteCount == -1){
                WriteToBuffer(buffer, "<--");
            }else{
                stringAppendEnd(buffer, row->data + startOffset, byteCount);
                WriteToBuffer(buffer, ESC_RESET_TEXT_ATTRIBUTES);
            }
#else
            int startOffset;
            int byteCount = stringCharToByteCount(row, horizontalOffset, 0, maxCols, &startOffset);
            if(byteCount == -1){
                WriteToBuffer(buffer, "<--");
            }else{
                string *formatBuffer = stringCreateHeap(row->maxSize);
                char prevTokenId = TOKEN_NEUTRAL;
                int escLen = 0;
                for(int j = horizontalOffset; (j < maxCols + horizontalOffset) && (j < row->len); j++){
                    character c = getCharAtPosEx(row, j);
                    if(c.tokenId != prevTokenId){
                        char *escSeqInsert = token_escape[c.tokenId];
                        escLen += strlen(escSeqInsert);
                        WriteToBuffer(formatBuffer, escSeqInsert);
                        prevTokenId = c.tokenId;
                    }
                    stringAppendEnd(formatBuffer, c.arr, c.byteCount);
                }

                stringAppendEnd(buffer, formatBuffer->data, byteCount + escLen);
                stringFreeHeap(formatBuffer);
                WriteToBuffer(buffer, ESC_RESET_TEXT_ATTRIBUTES);
            }
#endif
#endif
        }

        if(i < maxRows - 1)
            WriteToBufferA(buffer, "\r\n");
    }
}

static void formatHeader(string_a *buffer){
    // TODO: Show only part of the header in case the width is too small?

    i32 offsetLeft = 2;
    i32 width = GlobalConsoleInfo.Cols - offsetLeft;
    char dest[2*width];
    char fileNameStr[512];

    i32 fNameLen = snprintf(fileNameStr, 512, "%s %3s", (*EC.File == 0) ? ("New") : EC.File, (TEST_FLAG_LOCAL(FLAG_READONLY) ? ("(r)") : (TEST_FLAG_LOCAL(FLAG_DIRTY) ? "(m)" : "")));
    
    WriteToBufferA(buffer, ESC_CLEAR_LINE);
    for(i32 i = 0; i < offsetLeft; i++) WriteToBufferA(buffer, " ");
    
    i32 feedback = snprintf(dest, width*2, "|| F(%u): %s | LC: %4d | L: %4d | C: %4d | Mode: %s %s ||    |: %s :|",
        GET_EDITOR_ID(), fileNameStr, EC.List.size, EC.Cursor.Y + 1, EC.Cursor.X, GET_FORMAT_STRING(GET_LOCAL_FORMAT_NUMBER()), EndLine[TEST_FLAG_LOCAL(FLAG_NEWLINE_ENTER) != 0], (*E.Message.Data == 0 ? "---" : E.Message.Data));
    
    if(feedback > width && *E.Message.Data != 0){
        feedback = snprintf(dest, 2*width, "|: %s :|", (*E.Message.Data == 0 ? "---" : E.Message.Data));
    }

    WriteToBufferA(buffer, ESC_INVERTED_TEXT_COLOR ESC_TEXT_BOLD);
    WriteToBufferA(buffer, "    ");
    
    WriteToBufferA(buffer, dest);

    for(i32 i = 0; i < (width - feedback) - 4; i++) WriteToBufferA(buffer, " ");

    WriteToBufferA(buffer, ESC_RESET_TEXT_ATTRIBUTES);
    WriteToBufferA(buffer, "\r\n");
}

static void editorRefreshScreen(){
    /* Reseting cursor */
    PrintA(ESC_HIDE_CURSOR);
    PrintA(ESC_RESET_CURSOR_POSSITION);

    ClearBuffer(&GlobalWriteBuffer);
    /* Formating output */
    if(TEST_FLAG_GLOBAL(GLOBAL_FLAG_SHOWHEADER)) formatHeader(&GlobalWriteBuffer);
    drawRows(&GlobalWriteBuffer);
    
    
    /* Printing the output buffer */
    PrintBufferA(GlobalWriteBuffer);
    ClearBuffer(&GlobalWriteBuffer);

    setCursorPossition(EC.Cursor.X - EC.Offset.X,
        EC.Cursor.Y - EC.Offset.Y);
    PrintA(ESC_SHOW_CURSOR);
}


static inline bool32 deleteLocalEditorState(u8 ind){
    if(ind >= EDITOR_STATE_COUNT) return false;
    editor_state *const focusedState = E.states + ind;
    
    if(!TEST_FLAG(focusedState->Flags, FLAG_INITIALIZED)) return false;

    FreeList(&focusedState->List);

    E.stateCount--;

    return true;
}

static bool32 createLocalEditorState(u8 ind){
    if(ind >= EDITOR_STATE_COUNT) return false;
    editor_state *const focusedState = E.states + ind;
    
    if(TEST_FLAG(focusedState->Flags, FLAG_INITIALIZED)) return false;
    
    E.stateCount++;

    focusedState->File[0] = 0;
    focusedState->Flags = DEFAULT_LOCAL_FLAGS;
    focusedState->Cursor = (int_pair){0, 0};
    focusedState->Offset = (int_pair){0, 0};
    focusedState->oldCursor = (int_pair){-1, -1};
    focusedState->List = CreateListWithRows(1, LIST_TYPE_UTF32);
}

static inline bool32 resetLocalEditorState(u8 ind){
    deleteLocalEditorState(ind);
    
    return createLocalEditorState(ind);
}

static inline bool32 switchEditorState(u8 ind){
    if(ind > EDITOR_STATE_COUNT) return false;
    editor_state *focused = E.states + ind;

    if(!TEST_FLAG(focused->Flags, FLAG_INITIALIZED)){
        createLocalEditorState(ind);
    }

    SET_EDITOR_ID(ind);

    return true;
}

static inline void updateStatus(){
    /* Message expiring */
    if(E.Message.Duration != 0){
        if(GlobalConsoleInfo.CurrentTime - E.Message.PostTime > E.Message.Duration){
            *E.Message.Data = 0;
            E.Message.Duration = 0;
            E.Message.PostTime = 0;
            SET_FLAG_LOCAL(FLAG_RENDER);
        }
    }

    /* Offsets from top and left side */
    if(TEST_FLAG_GLOBAL(GLOBAL_FLAG_SHOWNUMBERS)){
        E.TextOffsetLeft = 4;
    }else{
        E.TextOffsetLeft = 1;
    }
    if(TEST_FLAG_GLOBAL(GLOBAL_FLAG_SHOWHEADER)){
        E.TextOffsetTop = 1;
    }else{
        E.TextOffsetTop = 0;
    }

#if 0
    /* Switching buffers */
    if(TEST_FLAG_LOCAL(FLAG_ALTVIEW)){
        uint8 bufferID = GET_ALT_BUFFERID();
        if(bufferID < ALT_RESERVED_COUNT) SET_FLAG(FLAG_RDONLY);
        else CLEAR_FLAG(FLAG_RDONLY);
        E.Rows = (E.AltBuffers + bufferID);

        if(E.oldCursor.X < 0) {
            //SET_FLAG(FLAG_RENDER);
            E.oldCursor = E.Cursor;
            E.Cursor = (int_pair){0, 0};
            FixCursorPossition();
        }
    }else{
        CLEAR_FLAG(FLAG_READONLY);
        E.Rows = &E.MainBuffer;
        if(E.oldCursor.X >= 0){
            //SET_FLAG(FLAG_RENDER);
            E.Cursor = E.oldCursor;
            E.oldCursor = (int_pair){-1, -1};
            FixCursorPossition();
        }
    }
#endif
}

static string_a *editorPromtHeader(char *promt){
    char *format = "%s : %s";
    string_a *strBuffer = StringCreateHeapA(64);

    while(1){
        postEditorMessage(messageDurationInfinite, format, promt, strBuffer->data);
        editorRefreshScreen();

        read_result ci = io_ReadConsoleInput();
        if(ci.type == TYPE_ESCAPE) continue;
        switch(ci.bytes[0]){
            case '\x1b':
            case '\b':
            case '\t':
                break;
            case 127:
                if(strBuffer->len)
                    DeleteCharFromPosA(strBuffer, strBuffer->len - 1);
                break;
            case '\r':
                TerminateStringOnPosA(strBuffer, strBuffer->len);
                return strBuffer;
            case CTRL_KEY('q'):
                StringFreeHeap(strBuffer);
                return NULL;
            default:
                StringAppendEndA(strBuffer, ci.bytes, ci.size);
                break;
        }
    }
}

static void executeCommand(command_type ct){
    switch(ct){
        case COMMAND_QUIT: {
            for(u32 BufferIndex = 0; BufferIndex < EDITOR_STATE_COUNT; BufferIndex++){
                editor_state *focusedState = &GLOBAL_STRUCT_NAME.states[BufferIndex];
                if(TEST_FLAG(focusedState->Flags, FLAG_INITIALIZED)){
                    if(TEST_FLAG_LOCAL(FLAG_DIRTY)){
                        SET_EDITOR_ID(BufferIndex);

                        postEditorMessage(4*messageDurationDefault, "Unsaved changes (%u). Quit? (Yes, No, Save)", BufferIndex);
                        while(1){
                            editorRefreshScreen();
                            read_result rr = io_ReadConsoleInput();
                            if(rr.type != TYPE_ANSI) continue;
                            switch(rr.bytes[0] & 0xdf){
                                case 'Y':
                                    //CLEAR_FLAG_GLOBAL(GLOBAL_FLAG_RUNNING);
                                    goto END_WHILE;
                                case 'N':
                                    clearEditorMessage();
                                    return;
                                case 'S':
                                    switch(editorSave(focusedState->File, &focusedState->List)){
                                        case 1: break;
                                        case 0:
                                            goto END_WHILE;
                                        case -1:
                                            postEditorMessage(messageDurationDefault, "Saving canceled on buffer %d", BufferIndex);
                                            return;
                                    }
                                    break;
                            }
                        }
                        END_WHILE:
                    }
                    CLEAR_FLAG(focusedState->Flags, FLAG_INITIALIZED);
                }
            }
            CLEAR_FLAG_GLOBAL(GLOBAL_FLAG_RUNNING);
            break;
        }
        case COMMAND_OPEN_HELP: break; // TODO: Implement help
        case COMMAND_SAVE: {
            editorSave(EC.File, &EC.List);
            break;
        }break;
        case COMMAND_LOAD: { // File open
            if(TEST_FLAG_LOCAL(FLAG_DIRTY) && EC.File[0] != 0){
                postEditorMessage(4*messageDurationDefault, "Unsaved changes (%u). Proceed? (Yes, No, Save)", GET_EDITOR_ID());
                while(1){
                    editorRefreshScreen();
                    read_result rr = io_ReadConsoleInput();
                    if(rr.type != TYPE_ANSI) continue;
                    switch(rr.bytes[0] & 0xdf){
                        case 'Y':
                            goto END_WHILE2;
                        case 'N':
                            clearEditorMessage();
                            return;
                        case 'S':
                            if(editorSave(EC.File, &EC.List)) break;
                            goto END_WHILE2;
                    }
                }
                END_WHILE2:
            }
            string_a *feedback = editorPromtHeader("Enter path:");
            if(feedback != NULL) {
                if(io_GetFileSize(feedback->data) <= 0) {
                    postEditorMessage(2*messageDurationDefault, "Invalid file: %s : %s", feedback->data, io_GetInputOutputErrorString());
                    break;
                }
#if 0
                FreeList(&EC.List);
                
                EC.List = CreateListWithRows(1, LIST_TYPE_UTF32);

                E.Cursor = (int_pair){0, 0};
#else
                resetLocalEditorState(GET_EDITOR_ID());
#endif
                //strncpy(EC.File, feedback->data, sizeof(EC.File));
                u32 copySize = MIN_VAL(feedback->len, sizeof(EC.File) - 1);
                memcpy(EC.File, feedback->data, copySize);
                EC.File[copySize] = 0;

                editorLoadFile(EC.File);
                StringFreeHeap(feedback);
            }
        }break;
        case COMMAND_SHOWNUMBER_TOGGLE: TOGGLE_FLAG_GLOBAL(GLOBAL_FLAG_SHOWNUMBERS); break;
        case COMMAND_SHOWHEADER_TOGGLE: TOGGLE_FLAG_GLOBAL(GLOBAL_FLAG_SHOWHEADER); break;
        case COMMAND_SYNTAX: TOGGLE_FLAG_LOCAL(FLAG_SYNTAX); break;
        case COMMAND_INSERT: TOGGLE_FLAG_GLOBAL(GLOBAL_FLAG_INSERT); break;
        case COMMAND_GOTO_LINE:{
            string_a *result = editorPromtHeader("Goto line:");
            if(result){
                char *endPtr;
                i32 val = strtoul(result->data, &endPtr, 0);
                StringFreeHeap(result);
                
                if(*endPtr != 0){
                    postEditorMessage(messageDurationDefault, "Invalid input: Not a number");
                }else if(errno == ERANGE){
                    postEditorMessage(messageDurationDefault, "Invalid input: Overflow (number too large)");
                }else{
                    if(val > 0){
#if 0
                        if(val < E.Rows->size){
                            E.Cursor.Y = val-1;
                            clearEditorMessage();
                        }else{
                            postEditorMessage(messageDurationDefault, "Out of bounds: %d < %d", E.Rows->size, val);
                        }
#else
                        EC.Cursor.Y = (val - 1 < EC.List.size ? val - 1 : EC.List.size - 1);
                        clearEditorMessage();
#endif
                    }else{
                        postEditorMessage(messageDurationDefault, "Out of bounds: %d is negative", val);
                    }
                }
            }else{
                postEditorMessage(messageDurationDefault, "Canceled");
            }
        }break;
        case COMMAND_FIND: {
#if 0
            SEARCH_REPEAT:
            // Search (Ctrl+f)
            string_a *Result = editorPromtHeader("Find:");
            if(Result){
                i32 matchCount = 0;
                int_pair oldCursor = EC.Cursor;
                do{
                    list_node *p = EC.List.head;
                    for(i32 i = 0; i < EC.List.size; i++, p = p->next){
                        string_u32 *focusedStr = (string_u32 *)p->str;
                        char *resultPtr = strstr(focusedStr->data, Result->data);
                        while(resultPtr != NULL){
                            matchCount++;
                            i32 xOffset = resultPtr - focusedStr->data;
                            EC.Cursor.X = xOffset;
                            EC.Cursor.Y = i;

                            FixCursorPossition();
                            if(HAS_FLAG(FLAG_SYNTAX))
                                syntaxHighlight(&EC.List);
                            else
                                resetSyntaxHighlight(&EC.List);

                            /* Highlighting the string */
                            int xOffsetInChars;
                            int charLen = stringByteToCharCount(p->str, xOffset, 0, Result->byteLen, &xOffsetInChars);
                            syntaxForce(p->str, xOffsetInChars, TOKEN_SEARCH_HIGHTLIGHT, charLen);

                            editorRefreshScreen();

                            character_input ci;
                            do{
                                ci = pollInput();
                            }while(ci.byteCount == 0);

                            switch(*rr.bytes){
                                case '\r':
                                    // Search further
                                    resultPtr = strstr(resultPtr + Result->byteLen, Result->data);
                                    break;
                                case 'r':
                                    // Return to old possition
                                    EC.Cursor = oldCursor;
                                    ATR_FALLTROUGHT;
                                case CTRL_KEY('f'):
                                    stringFreeHeap(Result);
                                    goto SEARCH_REPEAT;
                                    break;
                                default:
                                    // Break search
                                    resultPtr = NULL;
                                    goto WHILE_END;
                                    break;
                            }
                        }
                    }

                }while(matchCount);
                WHILE_END:
                if(matchCount)
                    clearEditorMessage();
                else
                    postEditorMessage(messageDurationDefault, "No matches");
                    stringFreeHeap(Result);
            }else{
                postEditorMessage(messageDurationDefault, "Canceled");
            }
#endif
        }break;
        case COMMAND_SWITCH_CHARSET: break;
        case COMMAND_TOGGLE_READONLY: TOGGLE_FLAG_LOCAL(FLAG_READONLY); break;
        case COMMAND_BACKSPACE: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;

            if(EC.Cursor.X == 0){
                string_u32 *rowNext = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);

                if(EC.Cursor.Y != 0){
                    string_u32 *rowPrev = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y-1);
                    EC.Cursor.X = rowPrev->len;
                    ConcatStringsU32(rowPrev, rowNext);
                    ListDeleteRow(&EC.List, EC.Cursor.Y);

                    EC.Cursor.Y--;
                    SET_FLAG_LOCAL(FLAG_DIRTY);
                }
            }else{
                deleteCharFromPosList(&EC.List, EC.Cursor.Y, EC.Cursor.X - 1);
                EC.Cursor.X--;
                SET_FLAG_LOCAL(FLAG_DIRTY);
            }
        }break;
        case COMMAND_REMOVE_WORD_LEFT: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;
            string_u32 *current = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);

            i32 startCharOffset = EC.Cursor.X;
            wchar32 *start = current->data + startCharOffset;
            i32 removeCountLeft;

            if(((start - 1 < current->data) || start[-1].full == U' ')){
                removeCountLeft = CountBackToNotChar32(current, EC.Cursor.X, WCHAR32(' '));
            }else{
                removeCountLeft = CountBackToChar32(current, EC.Cursor.X, WCHAR32(' '));
            }
            if(removeCountLeft != 0){
                SET_FLAG_LOCAL(FLAG_DIRTY);
                for(i32 i = 0; i < removeCountLeft; i++){
                    DeleteCharFromPosU32(current, EC.Cursor.X-1);
                    EC.Cursor.X--;
                }
            }
        } break;
        case COMMAND_DELETE: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;

            string_u32 *currentRow = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);
            if(EC.Cursor.X == currentRow->len){
                if(EC.Cursor.Y + 1 < EC.List.size){
                    string_u32 *nextRow = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y + 1);

                    ConcatStringsU32(currentRow, nextRow);
                    ListDeleteRow(&EC.List, EC.Cursor.Y);
                    SET_FLAG_LOCAL(FLAG_DIRTY);
                }

            }else{
                deleteCharFromPosList(&EC.List, EC.Cursor.Y, EC.Cursor.X);
                SET_FLAG_LOCAL(FLAG_DIRTY);
            }
        }break;
        case COMMAND_REMOVE_WORLD_RIGHT: break;
        case COMMAND_NEWLINE: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;

            SET_FLAG_LOCAL(FLAG_DIRTY);
            string_u32 *row = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);
            string_u32 *str = StringCreateHeapU32(64);
            ListInsertAtPossition(&EC.List, (string*)str, EC.Cursor.Y + 1);
            
            if(EC.Cursor.X != row->len){
                i32 startCharOffset = EC.Cursor.X;
                i32 charsToCopy = row->len - startCharOffset;

                ConcatStringsExU32(str, row, startCharOffset, 0);
                TerminateStringOnPosU32(row, startCharOffset);
            }

            EC.Cursor.Y++;
            EC.Cursor.X = 0;
        }break;
        case COMMAND_TAB: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;
            SET_FLAG_LOCAL(FLAG_DIRTY);

            wchar32 whitespace = WCHAR32(U' ');
            insertCharAtPosList(&EC.List, whitespace, EC.Cursor.Y, EC.Cursor.X);
            EC.Cursor.X++;
            for(; EC.Cursor.X % TAB_SPACE_SIZE; ++EC.Cursor.X){
                insertCharAtPosList(&EC.List, whitespace, EC.Cursor.Y, EC.Cursor.X);
                EC.Cursor.X++;
            }
        }break;

        case COMMAND_UP: EC.Cursor.Y--; break;
        case COMMAND_LEFT: EC.Cursor.X--; break;
        case COMMAND_RIGHT: EC.Cursor.X++; break;
        case COMMAND_DOWN: EC.Cursor.Y++; break;
        case COMMAND_PAGEUP: 
            EC.Cursor.Y -= (GlobalConsoleInfo.Rows - E.TextOffsetTop);
            EC.Offset.Y -= (GlobalConsoleInfo.Rows - E.TextOffsetTop);
            break;
        case COMMAND_PAGEDOWN: 
            EC.Cursor.Y += (GlobalConsoleInfo.Rows - E.TextOffsetTop);
            EC.Offset.Y += (GlobalConsoleInfo.Rows - E.TextOffsetTop);
            break;
        case COMMAND_HOME: EC.Cursor.X = 0; break;
        case COMMAND_END: {
            string *str = GetStringAtIndex(&EC.List, EC.Cursor.Y);
            if(str != NULL){
                EC.Cursor.X = str->len;
            }
        }break;

        case COMMAND_OFFSET_UP: EC.Offset.Y--; break;
        case COMMAND_OFFSET_DOWN: EC.Offset.Y++; break;
        case COMMAND_SKIP_WORD_RIGHT: {
            string_u32 *current = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);

            i32 startCharOffset = EC.Cursor.X;
            wchar32 *start = current->data + startCharOffset;
            i32 skipCount;

            if(start[0].full == U' '){
                skipCount = CountToNotChar32(current, EC.Cursor.X, WCHAR32(' '));
            }else{
                skipCount = CountToChar32(current, EC.Cursor.X, WCHAR32(' '));
            }

            EC.Cursor.X += skipCount;
        } break;
        case COMMAND_SKIP_WORD_LEFT: {
            string_u32 *current = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);

            i32 startCharOffset = EC.Cursor.X;
            wchar32 *start = current->data + startCharOffset;
            i32 skipCount;

            if((start - 1 < current->data) || (start[-1].full == U' ')){
                skipCount = CountBackToNotChar32(current, EC.Cursor.X, WCHAR32(' '));
            }else{
                skipCount = CountBackToChar32(current, EC.Cursor.X, WCHAR32(' '));
            }

            EC.Cursor.X -= skipCount;
        } break;

        case COMMAND_MOVE_ROW_UP: {
            if(EC.Cursor.Y > 0 && EC.List.size > 1){
                SwapStringsForIndexes(&EC.List, EC.Cursor.Y, EC.Cursor.Y - 1);
                EC.Cursor.Y--;
            }
        }break;
        case COMMAND_MOVE_ROW_DOWN: {
            if(EC.Cursor.Y + 1 < EC.List.size && EC.List.size > 1){
                SwapStringsForIndexes(&EC.List, EC.Cursor.Y, EC.Cursor.Y + 1);
                EC.Cursor.Y++;
            }
        }break;
        case COMMAND_NEXT_BUFFER: {
            u8 contextIndex = GET_EDITOR_ID();

            switchEditorState((contextIndex + 1U) % EDITOR_STATE_COUNT);
        }break;
        case COMMAND_PREV_BUFFER: {
            u8 currentIndex = GET_EDITOR_ID();
            currentIndex -= 1U;
            
            if(currentIndex > EDITOR_STATE_COUNT) currentIndex = EDITOR_STATE_COUNT - 1;
            switchEditorState(currentIndex);
        }break;

        case COMMAND_NEUTRAL:
            postEditorMessage(messageDurationDefault, "COMMAND_NEUTRAL recieved");
            break;
        default:
            postEditorMessage(messageDurationDefault, "Invalid command type : %x", ct);
            break;
    }    
}

int main(int argc, char *argv[], char *envp[]){  
    term_PrepareConsole();
    GlobalWriteBuffer = StringCreateA(4096);
    CLEAR_FLAG_GLOBAL(0xffffffff);
    SET_FLAG_GLOBAL(DEFAULT_GLOBAL_FLAGS);
    E.maxCursor.X = DEFAULT_CURSOR_MAXIMUM_X;
    E.maxCursor.Y = DEFAULT_CURSOR_MAXIMUM_Y;

    GlobalConsoleInfo = term_GetConsoleSystemInfo();
    
    postEditorMessage(messageDurationDefault, "Ctrl+e to open help");

    switchEditorContext(0);
    resetKeybinds();

    if(argc > 1){
        strcpy(EC.File, argv[1]);
        editorLoadFile(EC.File);
    }else{
        EC.File[0] = 0;
    }

    /* Syntax highlight */
    // TODO: Actually implement this

    setCursorPossition(0, 0);
    SET_FLAG_LOCAL(FLAG_RENDER);

    while(TEST_FLAG_GLOBAL(GLOBAL_FLAG_RUNNING)){
        GlobalConsoleInfo = term_GetConsoleSystemInfo();
        updateStatus();

        if(TEST_FLAG_LOCAL(FLAG_RENDER)){
            editorRefreshScreen();
            CLEAR_FLAG_LOCAL(FLAG_RENDER);
        }

        read_result r = io_ReadConsoleInput();
        if(r.bytes != 0){
            parser_result pr = parser_ParseInput(r);
            if(pr.type == RESULT_TYPE_COMMAND){
                executeCommand(translateCommandKey(pr.command));
            }else{
                if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;
                //stringAppendEnd(&OutputBuffer, rr.bytes, ci.byteCount);
            
                SET_FLAG_LOCAL(FLAG_DIRTY);
                insertCharAtPosList(&EC.List, pr.character, EC.Cursor.Y, EC.Cursor.X);
                EC.Cursor.X++;
            }
            SET_FLAG_LOCAL(FLAG_RENDER);
            fixCursorPossition();
        }
    }

    return 0;
}