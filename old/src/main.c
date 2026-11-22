#include "types.h"
#include "system.h"
#include "main.h"
#include "utf8.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static console_info ConInfo;

typedef struct int_pair{
    int X, Y;
} int_pair;

static struct{
    uint32 Flags;

#if 1
    int_pair Cursor; /* The possition of the cursor inside the internal buffer */
    int_pair oldCursor; /* The possitions in the main buffer, saved here during altbuffer switches */
    int_pair Offset;
    int_pair maxCursor;
#else
    int CursorX, CursorY;
    int oldCursorX, oldCursorY;
    int OffsetX, OffsetY;
    int maxCursorX, maxCursorY;
#endif
    char File[FILE_PATH_LENGTH];
    string_list *Rows;
    string_list MainBuffer;
    string_list AltBuffers[ALT_BUFFER_COUNT];

    int TextOffsetTop, TextOffsetLeft;

    editor_message Message;
} E;

string OutputBuffer;

static inline void updateStatus();
static void editorRefreshScreen();
static string *editorPromtHeader(char *promt);
static int editorLoadFile(char *path);
static void stringIntoInput(const char *str, int64 len);

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
static const unsigned long long helpFullLength = sizeof(helpFull);


static inline void syntaxHighlightGlobal(string_list *list){
    listForeachString(list, syntaxHighlightString);
}

static inline void syntaxHighlight(string_list *list){
    listForeachStringEx(list, MAX_VAL(E.Cursor.Y - ROW_OVERHIGHLIGHT_COUNT, 0), ConInfo.Rows + ROW_OVERHIGHLIGHT_COUNT, syntaxHighlightString);
    listForeachStringEx(list, E.Offset.Y, ConInfo.Rows, syntaxHighlightStringKeyword);
}

#if 0
static inline void syntaxHighlightNarrow(string_list *list){
    listForeachStringEx(list, E.Cursor.Y, ConInfo.Rows, syntaxHighlightStringKeyword);
}
#endif
static inline void resetSyntaxHighlight(string_list *list){
    listForeachStringEx(list, MAX_VAL(E.Cursor.Y - ConInfo.Rows, 0), ConInfo.Rows + ConInfo.Rows, resetHighlight);
}

static void switchToAltBuffer(uint8 bufferID){
    SET_FLAG(FLAG_ALTVIEW);

    SET_ALT_BUFFERID(bufferID);
    if(bufferID < ALT_RESERVED_COUNT) SET_FLAG(FLAG_RDONLY);
    else CLEAR_FLAG(FLAG_RDONLY);
    E.Rows = (E.AltBuffers + bufferID);
}

static void switchToMainBuffer(){
    CLEAR_FLAG(FLAG_ALTVIEW);
    CLEAR_FLAG(FLAG_RDONLY);
    E.Rows = &E.MainBuffer;
}

static int countToNotChar(string *str, int startPos, uint32 character){
    int Result = 0;
    character_input ci = {0};
    ci.byteCount = charGetByteCount(((char *)&character)[0]);
    ci.arr[0] = ((char *)&character)[0];
    ci.arr[1] = ((char *)&character)[1];
    ci.arr[2] = ((char *)&character)[2];
    ci.arr[3] = ((char *)&character)[3];

    int startOffsetInBytes;
    stringCharToByteCount(str, startPos, 0, 0, &startOffsetInBytes);

    int bytesPassed = 0;
    for(int i = startPos; i < str->len; i++){
        int startOffset;
        int byteCount = str->byteCount[i];
        
        if(byteCount == ci.byteCount){
            for(int j = 0; j < ci.byteCount; j++){
                if(ci.arr[j] != (str->data + startOffsetInBytes + bytesPassed)[j]) goto CHECK_SUCCESS;
            }
            goto CHECK_FAILED;
        }
        CHECK_FAILED:
        Result++;
        bytesPassed += byteCount;
    }

    CHECK_SUCCESS:
    return Result;
}

static int countBackToNotChar(string *str, int startPos, uint32 character){
    int Result = 0;
    character_input ci = {0};
    ci.byteCount = charGetByteCount(((char *)&character)[0]);
    ci.arr[0] = ((char *)&character)[0];
    ci.arr[1] = ((char *)&character)[1];
    ci.arr[2] = ((char *)&character)[2];
    ci.arr[3] = ((char *)&character)[3];

    int startOffsetInBytes;
    stringCharToByteCount(str, startPos, 0, 0, &startOffsetInBytes);

    int bytesPassed = 0;
    for(int i = startPos - 1; i >= 0; i--){
        int startOffset;
        int byteCount = str->byteCount[i];
        
        if(byteCount == ci.byteCount){
            for(int j = 0; j < ci.byteCount; j++){
                if(ci.arr[j] != (str->data + startOffsetInBytes - bytesPassed - ci.byteCount)[j]) goto CHECK_SUCCESS;
            }
            goto CHECK_FAILED;
        }
        CHECK_FAILED:
        Result++;
        bytesPassed += byteCount;
    }

    CHECK_SUCCESS:
    return Result;
}

static int countBackToChar(string *str, int startPos, uint32 character){
    int Result = 0;
    character_input ci = {0};
    ci.byteCount = charGetByteCount(((char *)&character)[0]);
    ci.arr[0] = ((char *)&character)[0];
    ci.arr[1] = ((char *)&character)[1];
    ci.arr[2] = ((char *)&character)[2];
    ci.arr[3] = ((char *)&character)[3];

    int startOffsetInBytes;
    stringCharToByteCount(str, startPos, 0, 0, &startOffsetInBytes);

    int bytesPassed = 0;
    for(int i = startPos - 1; i >= 0; i--){
        int startOffset;
        int byteCount = str->byteCount[i];
        
        if(byteCount == ci.byteCount){
            for(int j = 0; j < ci.byteCount; j++){
                if(ci.arr[j] != (str->data + startOffsetInBytes - bytesPassed - ci.byteCount)[j]) goto CHECK_FAILED;
            }
            goto CHECK_SUCCESS;
        }
        CHECK_FAILED:
        Result++;
        bytesPassed += byteCount;
    }

    CHECK_SUCCESS:
    return Result;
}

static int countToChar(string *str, int startPos, uint32 character){
    int Result = 0;
    character_input ci = {0};
    ci.byteCount = charGetByteCount(((char *)&character)[0]);
    ci.arr[0] = ((char *)&character)[0];
    ci.arr[1] = ((char *)&character)[1];
    ci.arr[2] = ((char *)&character)[2];
    ci.arr[3] = ((char *)&character)[3];

    int startOffsetInBytes;
    stringCharToByteCount(str, startPos, 0, 0, &startOffsetInBytes);

    int bytesPassed = 0;
    for(int i = startPos; i < str->len; i++){
        int startOffset;
        int byteCount = str->byteCount[i];
        
        if(byteCount == ci.byteCount){
            for(int j = 0; j < ci.byteCount; j++){
                if(ci.arr[j] != (str->data + startOffsetInBytes + bytesPassed)[j]) goto CHECK_FAILED;
            }
            goto CHECK_SUCCESS;
        }
        CHECK_FAILED:
        Result++;
        bytesPassed += byteCount;
    }

    CHECK_SUCCESS:
    return Result;
}

static inline void insertCharAtPosList(character_input ci, int Row, int Col){
    string *RowString = getStringAtIndex(E.Rows, Row);
    insertCharAtPossition(RowString, ci, Col, HAS_FLAG(FLAG_INSERT_MODE));
}

static inline void deleteCharFromPosList(string_list *list, int Row, int Col){
    string *str = getStringAtIndex(list, Row);
    deleteCharFromPossition(str, Col);
}

static inline void messageCreate(editor_message *dest, time_t duration, const char *text, ...){
    va_list args;
    va_start(args, text);
    vsnprintf(dest->Data, sizeof(dest->Data), text, args);
    va_end(args);
    //strcpy(dest->Data, text);
    dest->PostTime = ConInfo.CurrentTime;
    dest->Duration = duration;
}

static int editorSave(char *path, string_list *src){
#define ROWS_TO_SAVE (src)

    if(*E.File == 0){
        string *str = editorPromtHeader("Save location");
        if(str != NULL){
            strcpy(E.File, str->data);
            stringFreeHeap(str);
        }else{
            return -1;
        }
    }

    string Final = stringCreate(1024);
    uint32 SizeCount = 0;
    for(int i = 0; i < ROWS_TO_SAVE->size; i++){
        string *row = getStringAtIndex(ROWS_TO_SAVE, i);
        SizeCount += row->byteLen;
        stringAppendEnd(&Final, row->data, row->byteLen);
        if(i < ROWS_TO_SAVE->size - 1){
            if(HAS_FLAG(FLAG_NEWLINE_ENTER)){
                stringAppendEnd(&Final, "\n", 1);
                SizeCount += 1;
            }else{
                stringAppendEnd(&Final, "\r\n", 2);
                SizeCount += 2;
            }
        }
    }

    int64 Result = fileWrite(path, Final.data, Final.byteLen);
    
    if(Result != SizeCount){
        postEditorMessage(messageDurationDefault, "Error saving file. E: %u, W: %u", SizeCount, Result);
    }else{
        postEditorMessage(messageDurationDefault, "File properly saved (Wrote %u bytes)", Result);
        UNSET_FLAG(FLAG_DIRTY);
    }

    stringFree(&Final);

    return Result != SizeCount;
#undef ROWS_TO_SAVE
}

static void FixCursorPossition(){
    /* Fixing the internal cursor possition and the offsets */
    string *str = getStringAtIndex(E.Rows, E.Cursor.Y);
    int currentWidth = 0;
    if(str != NULL) currentWidth = str->len;
    int destWidth = MIN_VAL(ConInfo.Cols, currentWidth);

    if(E.Cursor.X < 0){
        E.Cursor.X = 0;
    }else if(E.Cursor.X > currentWidth){
        E.Cursor.X = currentWidth;
    }

    // Vertical offset
    if((E.Cursor.Y - E.Offset.Y > ConInfo.Rows - 1 - E.TextOffsetTop) && E.Cursor.Y < E.Rows->size){
        E.Offset.Y = MAX_VAL(0, (E.Cursor.Y) - (ConInfo.Rows - E.TextOffsetTop) + 1);
    }else if((E.Cursor.Y - E.Offset.Y < 0)){
        E.Offset.Y = MAX_VAL(0, E.Offset.Y - (E.Offset.Y - E.Cursor.Y));
    }

    // Horizontal offset
    if(E.Cursor.X - E.Offset.X > ConInfo.Cols - 1 - E.TextOffsetLeft){
        E.Offset.X = MAX_VAL(0, E.Cursor.X - (ConInfo.Cols - E.TextOffsetLeft) + 1);
    }else if((E.Cursor.X - E.Offset.X < 0)){
        E.Offset.X = MAX_VAL(0, E.Offset.X - (E.Offset.X - E.Cursor.X));
    }

    // Verticall offset clamping
    if(E.Offset.Y < 0) E.Offset.Y = 0;
#if 0
    if(E.Offset.Y > E.Rows->size - (ConInfo.Rows - E.TextOffsetTop)) 
        E.Offset.Y = E.Rows->size - (ConInfo.Rows - E.TextOffsetTop);
#else
    if(E.Offset.Y > E.Rows->size - 1) E.Offset.Y = E.Rows->size - 1;
#endif

    if(E.Cursor.Y < 0){
        E.Cursor.Y = 0;
    }else if(E.Cursor.Y > E.Rows->size - 1){
        E.Cursor.Y = E.Rows->size - 1;
    }

    if(!(E.maxCursor.X < 0) && (E.Cursor.X > E.maxCursor.X)) E.Cursor.X = E.maxCursor.X;
    if(!(E.maxCursor.Y < 0) && (E.Cursor.Y > E.maxCursor.Y)) E.Cursor.Y = E.maxCursor.Y;
}





/* Setting cursor possition (zero based) */
static inline void setCursorPossition(int x, int y){
    char c[16];
    x++; y++;

    int maxRows = ConInfo.Rows;
    int maxCols = ConInfo.Cols;

    x += E.TextOffsetLeft;
    y += E.TextOffsetTop;

    if(x > maxCols) x = maxCols;
    if(y > maxRows) y = maxRows;

    sprintf(c, ESC_SEQ "%d;%dH", (y), (x));
    Print(c);
}

static void stringIntoInput(const char *str, int64 len){
    string *inputString = bufferCreateFromString(str, len);
    int bytesPassed = 0;

    int wasRdonly = HAS_FLAG(FLAG_RDONLY);
    CLEAR_FLAG(FLAG_RDONLY);

    for(int i = 0; i < inputString->len; i++){
        character_input ci = {0};
        ci.byteCount = inputString->byteCount[i];
        memcpy(ci.arr, inputString->data + bytesPassed, ci.byteCount);
        bytesPassed += ci.byteCount;

        processInput(ci);
    }

    if(wasRdonly) SET_FLAG(FLAG_RDONLY);
}

static void byteIntoInput(const char *str, int64 len){
    const char *digits = "0123456789abcdef";
#if 0
    char tempB[128];
    tempB[0] = ' ';
    tempB[1] = '|';
    int noWhite = 0;
    int pos = 2;
#endif
    for(int i = 0; i < len; i++){
        char byte = str[i];
        character_input ci = {0};
        ci.byteCount = 1;
        ci.arr[0] = digits[(byte & 0xf0) >> 4];
        processInput(ci);
        ci.arr[0] = digits[byte & 0x0f];
        processInput(ci);
#if 0
        if(byte > 31 && byte != 127){
            tempB[pos++] = byte;
        }else{
            tempB[pos++] = '\\';
            tempB[pos++] = UNCTRL_KEY(byte);
            noWhite = 1;
        }
#endif
        if((i + 1) % 16 == 0){
#if 0
            tempB[pos++] = '\r';
            tempB[pos] = 0;
            stringIntoInput(tempB, pos);
            pos = 2;
#endif
            ci.arr[0] = '\r';
            processInput(ci);
        }else{
            ci.arr[0] = ' ';
#if 0
            if(noWhite) noWhite = 0;
            else tempB[pos++] = ' ';
            tempB[pos++] = ' ';
#endif
            processInput(ci);
        }
    }
}

static int editorLoadFile(char *path){
    clearEditorMessage();
    char buffer[256];
    int64 fileSize = getFileSize(path);
    if(fileSize <= 0) {
        postEditorMessage(2*messageDurationDefault, "FileSize error: %s : %s", path, getInputOutputErrorString());
        return 1;
    }

    char *tempBuffer = malloc(fileSize + 1);
    if(tempBuffer == NULL) die("malloc fault (%zu)", fileSize);
    tempBuffer[fileSize] = 0;

    int64 Feedback = fileRead(path, tempBuffer, fileSize);
    if(Feedback <= 0){
        postEditorMessage(2*messageDurationDefault, "fileRead error: %s : %s", path, getInputOutputErrorString());
        return 1;
    }

    if(Feedback != fileSize) 
        postEditorMessage(messageDurationDefault, "Possible load error (Exp: %u | Rec: %u)", fileSize, Feedback);

    
    if(!HAS_FLAG(FLAG_FORCE)){
        /* TODO: Check for file types */
    }

    switch(GET_FORMAT_NUMBER()){
        case FORMAT_UTF8:{
            for(int64 i = 0; i < Feedback; i++){
                if(tempBuffer[i] == '\n') {
                    SET_FLAG(FLAG_NEWLINE_ENTER);
                    break;
                }
                else if(tempBuffer[i] == '\r'){
                    CLEAR_FLAG(FLAG_NEWLINE_ENTER);
                    break;
                }
            }
            stringIntoInput(tempBuffer, Feedback);
            break;
        }
        case FORMAT_UTF16:
            break;
        case FORMAT_BIN:{
            byteIntoInput(tempBuffer, Feedback);
            break;
        }
    }

    free(tempBuffer);
    E.Cursor.X = 0;
    E.Cursor.Y = 0;
    FixCursorPossition();
    CLEAR_FLAG(FLAG_DIRTY);
}

/* Rendering */

static void drawRows(string *buffer){
    char rowNum[5];
    int maxRows = ConInfo.Rows;
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
#if 1
    int offsetLeft = 2;
    int width = ConInfo.Cols - offsetLeft;
    char dest[2*width];
    char fileNameStr[512];

    int fNameLen;

    if(HAS_FLAG(FLAG_ALTVIEW)){
        fNameLen = snprintf(fileNameStr, 512, "%s (%d) %3s", "AltBuffer", GET_ALT_BUFFERID(), (HAS_FLAG(FLAG_READONLY) ? "(r)" : ""));
    }else{
        fNameLen = snprintf(fileNameStr, 512, "%s %3s", (*E.File == 0) ? ("New") : E.File, (HAS_FLAG(FLAG_READONLY) ? ("(r)") : (HAS_FLAG(FLAG_DIRTY) ? "(m)" : "")));
    }
    
    WriteToBuffer(buffer, ESC_CLEAR_LINE);
    for(int i = 0; i < offsetLeft; i++) WriteToBuffer(buffer, " ");
    
    int feedback = snprintf(dest, width*2, "|| F: %s | LC: %4d | L: %4d | C: %4d | Mode: %s %s ||    |: %s :|",
        fileNameStr, E.Rows->size, E.Cursor.Y + 1, E.Cursor.X, GET_FORMAT_STRING(GET_FORMAT_NUMBER()), EndLine[HAS_FLAG(FLAG_NEWLINE_ENTER) != 0], (*E.Message.Data == 0 ? "---" : E.Message.Data));
    
    if(feedback > width && *E.Message.Data != 0){
        feedback = snprintf(dest, 2*width, "|: %s :|", (*E.Message.Data == 0 ? "---" : E.Message.Data));
    }

    WriteToBuffer(buffer, ESC_INVERTED_TEXT_COLOR ESC_TEXT_BOLD);
    WriteToBuffer(buffer, "    ");
    
    WriteToBuffer(buffer, dest);

    for(int i = 0; i < (width - feedback) - 4; i++) WriteToBuffer(buffer, " ");

    WriteToBuffer(buffer, ESC_RESET_TEXT_ATTRIBUTES);
    WriteToBuffer(buffer, "\r\n");
#else
    char bufferS[256];
    sprintf(bufferS, ESC_CLEAR_LINE "LineC: %d; curX: %d, curY: %d (%d), OffsetX: %d, OffsetY: %d, conSX: %d, conSY: %d, BufferId: (%d)%d\r\n", 
        E.Rows->size, E.Cursor.X, E.Cursor.Y, E.Cursor.Y + 1, E.Offset.X, E.Offset.Y, ConInfo.Cols, ConInfo.Rows, HAS_FLAG(FLAG_ALTVIEW) != 0, GET_ALT_BUFFERID());
    WriteToBuffer(buffer, bufferS);
#endif
}

static void editorRefreshScreen(){
    /* Reseting cursor */
    Print(ESC_HIDE_CURSOR);
    Print(ESC_RESET_CURSOR_POSSITION);

    /* Formating output */
    if(HAS_FLAG(FLAG_SHOWHEADER)) formatHeader(&OutputBuffer);
    drawRows(&OutputBuffer);
    
    
    /* Printing the output buffer */
    PrintBuffer(OutputBuffer);
    clearBuffer(&OutputBuffer);

    setCursorPossition(E.Cursor.X - E.Offset.X, E.Cursor.Y - E.Offset.Y);
    Print(ESC_SHOW_CURSOR);
}

static inline void updateStatus(){
    /* Message expiring */
    if(E.Message.Duration != 0){
        if(ConInfo.CurrentTime - E.Message.PostTime > E.Message.Duration){
            *E.Message.Data = 0;
            E.Message.Duration = 0;
            E.Message.PostTime = 0;
            SET_FLAG(FLAG_RENDER);
        }
    }

    /* Offsets from top and left side */
    if(HAS_FLAG(FLAG_SHOWNUMBERS)){
        E.TextOffsetLeft = 4;
    }else{
        E.TextOffsetLeft = 1;
    }
    if(HAS_FLAG(FLAG_SHOWHEADER)){
        E.TextOffsetTop = 1;
    }else{
        E.TextOffsetTop = 0;
    }

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
}

static string *editorPromtHeader(char *promt){
    char *format = "%s : %s";
    string *strBuffer = stringCreateHeap(64);

    while(1){
        postEditorMessage(messageDurationInfinite, format, promt, strBuffer->data);
        editorRefreshScreen();

        character_input ci = pollInput();
        switch(ci.arr[0]){
            case '\x1b':
            case '\b':
            case '\t':
                break;
            case 127:
                if(strBuffer->len)
                    deleteCharFromPossition(strBuffer, strBuffer->len - 1);
                break;
            case '\r':
                return strBuffer;
            case CTRL_KEY('q'):
                stringFreeHeap(strBuffer);
                return NULL;
            default:
                stringAppendEnd(strBuffer, ci.arr, ci.byteCount);
                break;
        }
    }
}

int main(int argc, char *argv[], char *envp[]){
    prepareConsole();
    CLEAR_FLAG(0xffffffff);
    SET_FLAG(FLAG_RUNNING | FLAG_SHOWHEADER | FLAG_SHOWNUMBERS | FLAG_INSERT_MODE | FLAG_RENDER | FLAG_SYNTAX);
#if defined LINUX
    SET_FLAG(FLAG_NEWLINE_ENTER);
#endif

    SET_FORMAT_NUMBER(FORMAT_UTF8);
    E.Cursor.X = 0;
    E.Cursor.Y = 0;
    E.maxCursor.X = DEFAULT_CURSOR_MAXIMUM_X;
    E.maxCursor.Y = DEFAULT_CURSOR_MAXIMUM_Y;
    E.oldCursor.X = -1;
    E.oldCursor.Y = -1;

    ConInfo = getConsoleSystemInfo();

    OutputBuffer = stringCreate(64);
    postEditorMessage(messageDurationDefault, "Ctrl+e to open help");

    E.MainBuffer = createListWithRows(1);
    E.Rows = &E.MainBuffer;
    for(int i = 0; i < ALT_BUFFER_COUNT; i++){
        E.AltBuffers[i] = createListWithRows(1);
    }

    if(argc > 1){
        strcpy(E.File, argv[1]);
        editorLoadFile(E.File);
    }else{
        E.File[0] = 0;
    }
    
    syntaxHighlight(E.Rows); 


    while(E.Flags & FLAG_RUNNING){
        ConInfo = getConsoleSystemInfo();
        updateStatus();

        /* Rendering to the terminal */
        if(HAS_FLAG(FLAG_RENDER)){
            editorRefreshScreen();
            CLEAR_FLAG(FLAG_RENDER);
            SET_FLAG(FLAG_FORMAT);
        }else if(HAS_FLAG(FLAG_FORMAT)){
            if(HAS_FLAG(FLAG_SYNTAX))
                syntaxHighlight(E.Rows);
            else
                resetSyntaxHighlight(E.Rows);
            CLEAR_FLAG(FLAG_FORMAT);
            editorRefreshScreen();
        }

        /* Processing input: Reading from stdin, checking for special cases and adding to the main buffer */
        character_input ci = pollInput();
        if(ci.byteCount != 0) {
            processInput(ci);
            SET_FLAG(FLAG_RENDER);
        }
    }

    return 0;
}