#include "system/io.h"
#include "input_parser.h"
#include "main.h"

static global_editor_state E = {0};
static string_a GlobalWriteBuffer = {0};
static console_info GlobalConsoleInfo = {0};

static void resetLocalEditorState(u32 ind){
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

/* Rendering */

static void drawRows(string *buffer){
    char rowNum[5];
    int maxRows = GlobalConsoleInfo.Rows;
    if(HAS_FLAG(FLAG_SHOWHEADER)) maxRows -= 1;

    int maxCols = ConInfo.Cols - 1;
    if(HAS_FLAG(FLAG_SHOWNUMBERS)) maxCols -= 3;

    /* Calculating verticall offset */
#if 0
    int verticallOffset = MAX_VAL(E.Cursor.Y - maxRows, 0);
    int horizontalOffset = MAX_VAL(E.Cursor.X - maxCols, 0);
    E.Offset.X = horizontalOffset;
    E.Offset.Y = verticallOffset;
#else
    int horizontalOffset = E.Offset.X;
    int verticallOffset = E.Offset.Y;
#endif
    int targetRows = MIN_VAL(E.Rows->size - verticallOffset, maxRows);
    for(int i = 0; i < maxRows; i++){
        WriteToBuffer(buffer, ESC_CLEAR_LINE);
        if(i < targetRows){
            if(HAS_FLAG(FLAG_SHOWNUMBERS)){
                sprintf(rowNum, "%3d|", (i + verticallOffset + 1) % 1000);
                WriteToBuffer(buffer, rowNum);
            }else{
                WriteToBuffer(buffer, "~");
            }

            string *row = getStringAtIndex(E.Rows, i + verticallOffset);
            if(row == NULL) die("Row was null, im out");

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
        }

        if(i < maxRows - 1)
            WriteToBuffer(buffer, "\r\n");
    }
}

static void formatHeader(string *buffer){
    // TODO: Show only part of the header in case the width is too small?

    i32 offsetLeft = 2;
    i32 width = GlobalConsoleInfo.Cols - offsetLeft;
    char dest[2*width];
    char fileNameStr[512];

    i32 fNameLen = snprintf(fileNameStr, 512, "%s %3s", (*CURRENT_LOCAL_STRUCT.File == 0) ? ("New") : CURRENT_LOCAL_STRUCT.File, (HAS_FLAG(FLAG_READONLY) ? ("(r)") : (HAS_FLAG(FLAG_DIRTY) ? "(m)" : "")));
    
    WriteToBuffer(buffer, ESC_CLEAR_LINE);
    for(i32 i = 0; i < offsetLeft; i++) WriteToBuffer(buffer, " ");
    
    i32 feedback = snprintf(dest, width*2, "|| F: %s | LC: %4d | L: %4d | C: %4d | Mode: %s %s ||    |: %s :|",
        fileNameStr, CURRENT_LOCAL_STRUCT.List.size, CURRENT_LOCAL_STRUCT.Cursor.Y + 1, CURRENT_LOCAL_STRUCT.Cursor.X, GET_FORMAT_STRING(GET_FORMAT_NUMBER()), EndLine[HAS_FLAG(FLAG_NEWLINE_ENTER) != 0], (*E.Message.Data == 0 ? "---" : E.Message.Data));
    
    if(feedback > width && *E.Message.Data != 0){
        feedback = snprintf(dest, 2*width, "|: %s :|", (*E.Message.Data == 0 ? "---" : E.Message.Data));
    }

    WriteToBuffer(buffer, ESC_INVERTED_TEXT_COLOR ESC_TEXT_BOLD);
    WriteToBuffer(buffer, "    ");
    
    WriteToBuffer(buffer, dest);

    for(i32 i = 0; i < (width - feedback) - 4; i++) WriteToBuffer(buffer, " ");

    WriteToBuffer(buffer, ESC_RESET_TEXT_ATTRIBUTES);
    WriteToBuffer(buffer, "\r\n");
}

static void editorRefreshScreen(){
    /* Reseting cursor */
    PrintA(ESC_HIDE_CURSOR);
    PrintA(ESC_RESET_CURSOR_POSSITION);

    /* Formating output */
    if(HAS_GLOBAL_FLAG(GLOBAL_FLAG_SHOWHEADER)) formatHeader(&GlobalWriteBuffer);
    drawRows(&GlobalWriteBuffer);
    
    
    /* Printing the output buffer */
    PrintBufferA(GlobalWriteBuffer);
    ClearBuffer(&GlobalWriteBuffer);

    setCursorPossition(CURRENT_LOCAL_STRUCT.Cursor.X - CURRENT_LOCAL_STRUCT.Offset.X,
        CURRENT_LOCAL_STRUCT.Cursor.Y - CURRENT_LOCAL_STRUCT.Offset.Y);
    PrintA(ESC_SHOW_CURSOR);
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
    if(HAS_FLAG(FLAG_ALTVIEW)){
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

int main(int argc, char *argv[], char *envp[]){  
    term_PrepareConsole();
    GlobalWriteBuffer = StringCreateA(4096);
    CLEAR_FLAG_GLOBAL(0xffffffff);
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


    while(TEST_FLAG_GLOBAL(GLOBAL_FLAG_RUNNING)){
        GlobalConsoleInfo = term_GetConsoleSystemInfo();
        updateStatus();

        if(TEST_FLAG_LOCAL(FLAG_RENDER)){
            editorRefreshScreen();
            CLEAR_FLAG_LOCAL(FLAG_RENDER);
        }

        read_result r = io_ReadConsoleInput();
        if(r.bytes != 0){
            parser_ParseInput(r);
            SET_FLAG_LOCAL(FLAG_RENDER);
        }
    }

    return 0;
}