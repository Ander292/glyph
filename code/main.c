#include "system/io.h"
#include "input_parser.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

static global_editor_state E = {0};
static string_a GlobalWriteBuffer = {0};
static console_info GlobalConsoleInfo = {0};

static const char helpFull[] =
    "Ctrl key combinations: \r"

        "\te -> enter/leave help\r"
        "\tq -> quit\r"
        "\to -> open new file\r"
        "\ts -> save current file\r"
        "\tg -> goto line\r"

        "\th -> same as ctrl+backspace, delete whole word\r"
        
        "\tt -> show header\r"
        "\tp -> toggle syntax highlighting\r"
        "\tn -> toggle numbers\r"
        "\tu -> switch between utf16 and utf8 mode"

        //"\tr -> read-only mode\r"
;
static const u64 helpFullLength = sizeof(helpFull);

static string_a *editorPromtHeader(char *promt);

static inline void syntaxHighlightGlobal(string_list *list){
    ListForeachString(list, SyntaxHighlightString);
    ListForeachString(list, SyntaxHighlightStringKeyword);
}

static inline void syntaxHighlight(string_list *list){
    ListForeachStringEx(list, MAX_VAL(CURRENT_LOCAL_STRUCT.Cursor.Y - ROW_OVERHIGHLIGHT_COUNT, 0), GlobalConsoleInfo.Rows + ROW_OVERHIGHLIGHT_COUNT, SyntaxHighlightString);
    ListForeachStringEx(list, CURRENT_LOCAL_STRUCT.Offset.Y, GlobalConsoleInfo.Rows, SyntaxHighlightStringKeyword);
}

static void resetLocalEditorState(u8 ind);

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
    string *str = GetStringAtIndex(&CURRENT_LOCAL_STRUCT.List, CURRENT_LOCAL_STRUCT.Cursor.Y);
    int currentWidth = 0;
    if(str != NULL) currentWidth = str->len;
    int destWidth = MIN_VAL(GlobalConsoleInfo.Cols, currentWidth);

    if(CURRENT_LOCAL_STRUCT.Cursor.X < 0){
        CURRENT_LOCAL_STRUCT.Cursor.X = 0;
    }else if(CURRENT_LOCAL_STRUCT.Cursor.X > currentWidth){
        CURRENT_LOCAL_STRUCT.Cursor.X = currentWidth;
    }

    // Vertical offset
    if((CURRENT_LOCAL_STRUCT.Cursor.Y - CURRENT_LOCAL_STRUCT.Offset.Y > GlobalConsoleInfo.Rows - 1 - GLOBAL_STRUCT_NAME.TextOffsetTop) && CURRENT_LOCAL_STRUCT.Cursor.Y < CURRENT_LOCAL_STRUCT.List.size){
        CURRENT_LOCAL_STRUCT.Offset.Y = MAX_VAL(0, (CURRENT_LOCAL_STRUCT.Cursor.Y) - (GlobalConsoleInfo.Rows - GLOBAL_STRUCT_NAME.TextOffsetTop) + 1);
    }else if((CURRENT_LOCAL_STRUCT.Cursor.Y - CURRENT_LOCAL_STRUCT.Offset.Y < 0)){
        CURRENT_LOCAL_STRUCT.Offset.Y = MAX_VAL(0, CURRENT_LOCAL_STRUCT.Offset.Y - (CURRENT_LOCAL_STRUCT.Offset.Y - CURRENT_LOCAL_STRUCT.Cursor.Y));
    }

    // Horizontal offset
    if(CURRENT_LOCAL_STRUCT.Cursor.X - CURRENT_LOCAL_STRUCT.Offset.X > GlobalConsoleInfo.Cols - 1 - GLOBAL_STRUCT_NAME.TextOffsetLeft){
        CURRENT_LOCAL_STRUCT.Offset.X = MAX_VAL(0, CURRENT_LOCAL_STRUCT.Cursor.X - (GlobalConsoleInfo.Cols - GLOBAL_STRUCT_NAME.TextOffsetLeft) + 1);
    }else if((CURRENT_LOCAL_STRUCT.Cursor.X - CURRENT_LOCAL_STRUCT.Offset.X < 0)){
        CURRENT_LOCAL_STRUCT.Offset.X = MAX_VAL(0, CURRENT_LOCAL_STRUCT.Offset.X - (CURRENT_LOCAL_STRUCT.Offset.X - CURRENT_LOCAL_STRUCT.Cursor.X));
    }

    // Verticall offset clamping
    if(CURRENT_LOCAL_STRUCT.Offset.Y < 0) CURRENT_LOCAL_STRUCT.Offset.Y = 0;
#if 0
    if(CURRENT_LOCAL_STRUCT.Offset.Y > CURRENT_LOCAL_STRUCT.List->size - (GlobalConsoleInfo.Rows - CURRENT_LOCAL_STRUCT.TextOffsetTop)) 
        CURRENT_LOCAL_STRUCT.Offset.Y = CURRENT_LOCAL_STRUCT.List->size - (GlobalConsoleInfo.Rows - CURRENT_LOCAL_STRUCT.TextOffsetTop);
#else
    if(CURRENT_LOCAL_STRUCT.Offset.Y > CURRENT_LOCAL_STRUCT.List.size - 1) CURRENT_LOCAL_STRUCT.Offset.Y = CURRENT_LOCAL_STRUCT.List.size - 1;
#endif

    if(CURRENT_LOCAL_STRUCT.Cursor.Y < 0){
        CURRENT_LOCAL_STRUCT.Cursor.Y = 0;
    }else if(CURRENT_LOCAL_STRUCT.Cursor.Y > CURRENT_LOCAL_STRUCT.List.size - 1){
        CURRENT_LOCAL_STRUCT.Cursor.Y = CURRENT_LOCAL_STRUCT.List.size - 1;
    }

    if(!(GLOBAL_STRUCT_NAME.maxCursor.X < 0) && (CURRENT_LOCAL_STRUCT.Cursor.X > GLOBAL_STRUCT_NAME.maxCursor.X)) CURRENT_LOCAL_STRUCT.Cursor.X = GLOBAL_STRUCT_NAME.maxCursor.X;
    if(!(GLOBAL_STRUCT_NAME.maxCursor.Y < 0) && (CURRENT_LOCAL_STRUCT.Cursor.Y > GLOBAL_STRUCT_NAME.maxCursor.Y)) CURRENT_LOCAL_STRUCT.Cursor.Y = GLOBAL_STRUCT_NAME.maxCursor.Y;
}


/* Saving */

static int editorSave(char *path, string_list *src){
#define ROWS_TO_SAVE (src)

    if(*CURRENT_LOCAL_STRUCT.File == 0){
        string_a *str = editorPromtHeader("Save location");
        if(str != NULL){
            strncpy(CURRENT_LOCAL_STRUCT.File, str->data, sizeof(CURRENT_LOCAL_STRUCT.File));
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
    char buffer[256];
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
    CURRENT_LOCAL_STRUCT.Cursor.X = 0;
    CURRENT_LOCAL_STRUCT.Cursor.Y = 0;
    fixCursorPossition();
    CLEAR_FLAG_LOCAL(FLAG_DIRTY);
}

/* Rendering */

static void drawRows(string_a *buffer){
    debug_check(CURRENT_LOCAL_STRUCT.List.type == LIST_TYPE_UTF32){
        Breakpoint
        Die("Invalid list type caught in drawRows()");
    }

    char rowNum[5];
    i32 maxRows = GlobalConsoleInfo.Rows;
    if(TEST_FLAG_GLOBAL(GLOBAL_FLAG_SHOWHEADER)) maxRows -= 1;

    i32 maxCols = GlobalConsoleInfo.Cols - 1;
    if(TEST_FLAG_GLOBAL(GLOBAL_FLAG_SHOWNUMBERS)) maxCols -= 3;

    /* Calculating verticall offset */
    i32 horizontalOffset = CURRENT_LOCAL_STRUCT.Offset.X;
    i32 verticallOffset = CURRENT_LOCAL_STRUCT.Offset.Y;

    i32 targetRows = MIN_VAL(CURRENT_LOCAL_STRUCT.List.size - verticallOffset, maxRows);
    for(i32 i = 0; i < maxRows; i++){
        WriteToBufferA(buffer, ESC_CLEAR_LINE);
        if(i < targetRows){
            if(TEST_FLAG_GLOBAL(GLOBAL_FLAG_SHOWNUMBERS)){
                snprintf(rowNum, sizeof(rowNum), "%3d|", (i + verticallOffset + 1) % 1000);
                WriteToBufferA(buffer, rowNum);
            }else{
                WriteToBufferA(buffer, "~");
            }

            string *row = GetStringAtIndex(&CURRENT_LOCAL_STRUCT.List, i + verticallOffset);
            if(row == NULL) Die("Row was null, im out");
#if 1
            //i32 startOffset = horizontalOffset;
            //(i < str->len - endOffset) && (i < maxCharCount + startOffset)

            i32 rowRemainingCharCount = (row->len - horizontalOffset);
            i32 targetCols = MIN_VAL(maxCols, rowRemainingCharCount);

            if(rowRemainingCharCount <= 0){
                WriteToBufferA(buffer, "<--");
            }else{
                byte prevTokenId = TOKEN_NEUTRAL;

                for(i32 j = horizontalOffset; j < targetCols; j++){
                    wchar32 c = GetCharAtPosU32((string_u32*)row, j);
                    byte currentTokenId = GetTokenAtPosU32(row, j);

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

    i32 fNameLen = snprintf(fileNameStr, 512, "%s %3s", (*CURRENT_LOCAL_STRUCT.File == 0) ? ("New") : CURRENT_LOCAL_STRUCT.File, (TEST_FLAG_LOCAL(FLAG_READONLY) ? ("(r)") : (TEST_FLAG_LOCAL(FLAG_DIRTY) ? "(m)" : "")));
    
    WriteToBufferA(buffer, ESC_CLEAR_LINE);
    for(i32 i = 0; i < offsetLeft; i++) WriteToBufferA(buffer, " ");
    
    i32 feedback = snprintf(dest, width*2, "|| F: %s | LC: %4d | L: %4d | C: %4d | Mode: %s %s ||    |: %s :|",
        fileNameStr, CURRENT_LOCAL_STRUCT.List.size, CURRENT_LOCAL_STRUCT.Cursor.Y + 1, CURRENT_LOCAL_STRUCT.Cursor.X, GET_FORMAT_STRING(GET_LOCAL_FORMAT_NUMBER()), EndLine[TEST_FLAG_LOCAL(FLAG_NEWLINE_ENTER) != 0], (*E.Message.Data == 0 ? "---" : E.Message.Data));
    
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

    /* Formating output */
    if(TEST_FLAG_GLOBAL(GLOBAL_FLAG_SHOWHEADER)) formatHeader(&GlobalWriteBuffer);
    drawRows(&GlobalWriteBuffer);
    
    
    /* Printing the output buffer */
    PrintBufferA(GlobalWriteBuffer);
    ClearBuffer(&GlobalWriteBuffer);

    setCursorPossition(CURRENT_LOCAL_STRUCT.Cursor.X - CURRENT_LOCAL_STRUCT.Offset.X,
        CURRENT_LOCAL_STRUCT.Cursor.Y - CURRENT_LOCAL_STRUCT.Offset.Y);
    PrintA(ESC_SHOW_CURSOR);
}

static void resetLocalEditorState(u8 ind){
    if(ind >= EDITOR_STATE_COUNT) postEditorMessage(messageDurationDefault, 
        "LocalStateIndex out of range %u. Limit: %u", ind, EDITOR_STATE_COUNT);
    
    editor_state *const focusedState = E.states + ind;
    
    if(TEST_FLAG(focusedState->Flags, FLAG_INITIALIZED)) 
        FreeList(&focusedState->List);
    
    focusedState->File[0] = 0;
    focusedState->Flags = DEFAULT_LOCAL_FLAGS;
    focusedState->Cursor = (int_pair){0, 0};
    focusedState->Offset = (int_pair){0, 0};
    focusedState->oldCursor = (int_pair){-1, -1};
    focusedState->List = CreateListWithRows(1, LIST_TYPE_UTF32);
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

    resetLocalEditorState(0);
    SET_EDITOR_ID(0);

    if(argc > 1){
        strcpy(CURRENT_LOCAL_STRUCT.File, argv[1]);
        editorLoadFile(CURRENT_LOCAL_STRUCT.File);
    }else{
        CURRENT_LOCAL_STRUCT.File[0] = 0;
    }

    /* Syntax highlight */
    // TODO: Actually implement this

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
                executeCommand(pr.command);
            }else{
                if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;
                //stringAppendEnd(&OutputBuffer, rr.bytes, ci.byteCount);
            
                SET_FLAG_LOCAL(FLAG_DIRTY);
                insertCharAtPosList(&CURRENT_LOCAL_STRUCT.List, pr.character, CURRENT_LOCAL_STRUCT.Cursor.Y, CURRENT_LOCAL_STRUCT.Cursor.X);
                CURRENT_LOCAL_STRUCT.Cursor.X++;
            }
            SET_FLAG_LOCAL(FLAG_RENDER);
        }
    }

    return 0;
}