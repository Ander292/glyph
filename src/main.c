#include "types.h"
#include "system.h"
#include "main.h"
#include "utf8.h"

#include <stdio.h>
#include <stdarg.h>

static console_info ConInfo;

static struct{
    uint32 Flags;

    int CursorX, CursorY; /* The possition of the cursor inside the internal buffer */
    int OffsetX, OffsetY;

    char File[FILE_PATH_LENGTH];
    string_list *Rows;
    string_list MainBuffer;
    string_list AltBuffers[ALT_BUFFER_COUNT];

    int TextOffsetTop, TextOffsetLeft;

    editor_message Message;
} E;

string OutputBuffer;

static void editorRefreshScreen();

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

int editorSave(char *path, string_list *src){
#define ROWS_TO_SAVE (src)
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

    uint32 Result = fileWrite(path, Final.data, Final.byteLen);
    
    if(Result != SizeCount){
        postEditorMessage(5, "Error saving file. Expected: %u, Wrote: %u", SizeCount, Result);
    }else{
        postEditorMessage(5, "File properly saved (Wrote %u bytes)", Result);
        UNSET_FLAG(FLAG_DIRTY);
    }

    stringFree(&Final);

    return Result != SizeCount;
#undef ROWS_TO_SAVE
}

void FixCursorPossition(){
    /* Fixing the internal cursor possition and the offsets */
    string *str = getStringAtIndex(E.Rows, E.CursorY);
    int currentWidth = 0;
    if(str != NULL) currentWidth = str->len;
    int destWidth = MIN_VAL(ConInfo.Cols, currentWidth);

    if(E.CursorX < 0){
        E.CursorX = 0;
    }else if(E.CursorX > currentWidth){
        E.CursorX = currentWidth;
    }

    // Vertical offset
    if((E.CursorY - E.OffsetY > ConInfo.Rows - 1 - E.TextOffsetTop) && E.CursorY < E.Rows->size){
        E.OffsetY = MAX_VAL(0, (E.CursorY) - (ConInfo.Rows - E.TextOffsetTop) + 1);
    }else if((E.CursorY - E.OffsetY < 0)){
        E.OffsetY = MAX_VAL(0, E.OffsetY - (E.OffsetY - E.CursorY));
    }

    // Horizontal offset
    if(E.CursorX - E.OffsetX > ConInfo.Cols - 1 - E.TextOffsetLeft){
        E.OffsetX = MAX_VAL(0, E.CursorX - (ConInfo.Cols - E.TextOffsetLeft) + 1);
    }else if((E.CursorX - E.OffsetX < 0)){
        E.OffsetX = MAX_VAL(0, E.OffsetX - (E.OffsetX - E.CursorX));
    }

    // Verticall offset clamping
    if(E.OffsetY < 0) E.OffsetY = 0;
#if 0
    if(E.OffsetY > E.Rows->size - (ConInfo.Rows - E.TextOffsetTop)) 
        E.OffsetY = E.Rows->size - (ConInfo.Rows - E.TextOffsetTop);
#else
    if(E.OffsetY > E.Rows->size - 1) E.OffsetY = E.Rows->size - 1;
#endif

    if(E.CursorY < 0){
        E.CursorY = 0;
    }else if(E.CursorY > E.Rows->size - 1){
        E.CursorY = E.Rows->size - 1;
    }
}


static int processInput(character_input ci){
    switch(ci.arr[0]){
        case CTRL_KEY('q'):{
            if(HAS_FLAG(FLAG_DIRTY)){
                postEditorMessage(20, "Unsaved changes. Quit? (Yes, No, Save)");
                while(1){
                    editorRefreshScreen();
                    character_input ci = pollInput();
                    switch(ci.arr[0] & 0xdf){
                        case 'Y':
                            UNSET_FLAG(FLAG_RUNNING);
                            goto END_WHILE;
                        case 'N':
                            clearEditorMessage();
                            return 0;
                        case 'S':
                            if(editorSave(E.File, DEFAULT_SAVE_SOURCE_ADDRESS)) break;
                            goto END_WHILE;
                    }
                }
                END_WHILE:
            }
            UNSET_FLAG(FLAG_RUNNING);
            break;
        }
        case CTRL_KEY('n'):
            TOGGLE_FLAG(FLAG_SHOWNUMBERS);
            break;
        case CTRL_KEY('g'):
            TOGGLE_FLAG(FLAG_SHOWHEADER);
            break;
        case CTRL_KEY('p'):
            TOGGLE_FLAG(FLAG_ALTVIEW);
            SET_ALT_BUFFERID(0);
            break;
        case CTRL_KEY('w'):
        case '\b': // Backspace (or ctrl + h)
            string *current = getStringAtIndex(E.Rows, E.CursorY);
            int startOffset;
            int byteCountTillEnd = stringCharToByteCount(current, E.CursorX, 0, 0, &startOffset);
            char *start = current->data + startOffset;
            int removeCountLeft;
            if((*start == ' ' || *start == 0) && ((start - 1 < current->data) || start[-1] == ' ')){
                removeCountLeft = countBackToNotChar(current, E.CursorX, ' ');
            }else{
                removeCountLeft = countBackToChar(current, E.CursorX, ' ');
            }
            if(removeCountLeft != 0){
                SET_FLAG(FLAG_DIRTY);
                for(int i = 0; i < removeCountLeft; i++){
                    deleteCharFromPossition(current, E.CursorX - removeCountLeft);
                }
            }
            break;
        case CTRL_KEY('s'):
            //postEditorMessage(5, "Saving not implemented yet");
            editorSave(E.File, DEFAULT_SAVE_SOURCE_ADDRESS);
            break;
        case 127:  // Delete key in ascii but its backspace for some reason
        {
            if(E.CursorX == 0){
                string *rowNext = getStringAtIndex(E.Rows, E.CursorY);
                if(E.CursorY != 0){
                    string *rowPrev = getStringAtIndex(E.Rows, E.CursorY-1);
                    E.CursorX = rowPrev->len;
                    stringAppendEnd(rowPrev, rowNext->data, rowNext->byteLen);
                    listDeleteRow(E.Rows, E.CursorY);
                    E.CursorY--;
                    SET_FLAG(FLAG_DIRTY);
                }
            }else{
                deleteCharFromPosList(E.Rows, E.CursorY, E.CursorX - 1);
                E.CursorX--;
                SET_FLAG(FLAG_DIRTY);
            }
            break;
        }
        case '\n':
            if(HAS_FLAG(FLAG_NEWLINE_ENTER)) goto NEWLINE;
            break;
        case '\r':{
            //if(HAS_FLAG(FLAG_NEWLINE_ENTER)) break;
            NEWLINE:
            SET_FLAG(FLAG_DIRTY);
            string *row = getStringAtIndex(E.Rows, E.CursorY);
            string *str = stringCreateHeap(64);
            listInsertAtPossition(E.Rows, str, E.CursorY + 1);
            
            if(E.CursorX != row->len){
                int startOffset;
                int bytesToCopy = stringCharToByteCount(row, E.CursorX, 0, 0, &startOffset);
                stringAppendEnd(str, row->data + startOffset, bytesToCopy);
                character_input ci = {0};
                ci.arr[0] = 0;
                ci.byteCount = 1;
                //insertCharAtPossition(row, ci, E.CursorX, 0);
                terminateStringOnPos(row, E.CursorX);
            }

            E.CursorY++;
            E.CursorX = 0;
            break;
        }
        case '\t':
            SET_FLAG(FLAG_DIRTY);
            character_input tabs = {0};
            tabs.arr[0] = ' ';
            tabs.byteCount = 1;
            insertCharAtPosList(tabs, E.CursorY, E.CursorX);
            E.CursorX++;
            int oldCursorX = E.CursorX;
            for(;E.CursorX % TAB_SPACE_SIZE; E.CursorX++){
                insertCharAtPosList(tabs, E.CursorY, oldCursorX);
            }
            break;
        // case 127: // Delete key
        //     postEditorMessage(5, "Delete key ):");
        //     SET_FLAG(FLAG_DIRTY);
        //     break;
        case '\x1b':{
            switch(ci.arr[2]){
                case 'A': // Up arrow
                case 'a':
                    UP_ARROW:
                    E.CursorY--;
                    break;
                case 'B': // Down arrow
                case 'b':
                    DOWN_ARROW:
                    E.CursorY++;
                    break;
                case 'C': // Right arrow
                case 'c':
                    RIGHT_ARROW:
                    E.CursorX++;
                    break;
                case 'D': // Left arrow
                case 'd':
                    LEFT_ARROW:
                    E.CursorX--;
                    break;
                case '5': // Pageup (followed by ~ but not checked)
                    //E.CursorY -= (ConInfo.Rows - 2);
                    E.CursorY -= (ConInfo.Rows - E.TextOffsetTop);
                    E.OffsetY -= (ConInfo.Rows - E.TextOffsetTop);
                    break;
                case '6': // Pagedown (followed by ~ but not checked)
                    //E.CursorY += (ConInfo.Rows - 2);
                    E.CursorY += (ConInfo.Rows - E.TextOffsetTop);
                    E.OffsetY += (ConInfo.Rows - E.TextOffsetTop);
                    // E.CursorY = E.OffsetY + ConInfo.Rows - 1;
                    // if(E.CursorY > E.Rows->size - 1) E.CursorY = E.Rows->size - 1;
                    break;
                case '2': // Insert key (followed by ~ but not checked)
                    TOGGLE_FLAG(FLAG_INSERT_MODE);
                    break;
                case 'O': // For the home and end escape sequences. They are diferent for different systems
                    if(ci.arr[3] == 'H') goto HOME;
                    else if(ci.arr[3] == 'F') goto END;
                    break;
                /* Home key. Goes to X index 0 - the begining to the line */
                case '1':
                    if(ci.byteCount == 6){
                        if(ci.arr[3] == ';'){
                            switch(ci.arr[4]){
                                case '5':{ // Ctrl modifier
                                    switch(ci.arr[5]){
                                        case 'A': // up arrow
                                            E.OffsetY--;
                                            break;
                                        case 'B': // down arrow
                                            E.OffsetY++;
                                            break;
                                        case 'C':{ // right arrow
                                            string *current = getStringAtIndex(E.Rows, E.CursorY);
                                            int startOffset;
                                            int byteCountTillEnd = stringCharToByteCount(current, E.CursorX, 0, 0, &startOffset);
                                            char *start = current->data + startOffset;
                                            if(*start == ' '){
                                                E.CursorX += countToNotChar(current, E.CursorX, ' ');
                                            }else{
                                                E.CursorX += countToChar(current, E.CursorX, ' ');
                                            }
                                            break;
                                        }
                                        case 'D':{ // left arrow
                                            string *current = getStringAtIndex(E.Rows, E.CursorY);
                                            int startOffset;
                                            int byteCountTillEnd = stringCharToByteCount(current, E.CursorX, 0, 0, &startOffset);
                                            char *start = current->data + startOffset;
                                            if(*start == ' ' && ((start - 1 < current->data) || start[-1] == ' ')){
                                                E.CursorX -= countBackToNotChar(current, E.CursorX, ' ');
                                            }else{
                                                E.CursorX -= countBackToChar(current, E.CursorX, ' ');
                                            }
                                            break;
                                        }
                                    }
                                    break;
                                }
                                case '3':{ // Alt keys
                                    switch(ci.arr[5]){
                                        case 'A': // Up arrow
                                            if(E.CursorY > 0 && E.Rows->size > 1) {
                                                //string *currentLine = getStringAtIndex(E.Rows, E.CursorY);
                                                //string *prevLine = getStringAtIndex(E.Rows, E.CursorY - 1);
                                                swapStringsForIndexes(E.Rows, E.CursorY, E.CursorY - 1);
                                                E.CursorY--;
                                            }
                                            break;
                                        case 'B': // down arrow
                                            if(E.CursorY + 1 < E.Rows->size && E.Rows->size > 1){
                                                swapStringsForIndexes(E.Rows, E.CursorY, E.CursorY + 1);
                                                E.CursorY++;
                                            }
                                            break;
                                        case 'C': { // Right arrow
                                            if(HAS_FLAG(FLAG_ALTVIEW)){
                                                uint8 currentBufferId = GET_ALT_BUFFERID();

                                                SET_ALT_BUFFERID(((currentBufferId + 1U) % ALT_BUFFER_COUNT));
                                            }
                                            break;
                                        }
                                        case 'D': { // Left arrow
                                            if(HAS_FLAG(FLAG_ALTVIEW)){
                                                uint8 currentBufferId = GET_ALT_BUFFERID();
                                                currentBufferId -= 1U;
                                                if(currentBufferId > ALT_BUFFER_COUNT) currentBufferId = ALT_BUFFER_COUNT - 1;
                                                SET_ALT_BUFFERID(currentBufferId);
                                            }
                                            break;  
                                        } 
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case '7':
                case 'H':
                    HOME:
                    E.CursorX = 0;
                    break;
                /* End key. Goes to the end of the line (depending on its size) */
                case '4':
                case '8':
                case 'F':
                    END:
                    string *str = getStringAtIndex(E.Rows, E.CursorY);
                    if(str != NULL){
                        E.CursorX = str->len;
                    }
                    break;
                case '3': // Delete key
                    //postEditorMessage(5, "Delete key <3");
                    //SET_FLAG(FLAG_DIRTY);
                    string *currentRow = getStringAtIndex(E.Rows, E.CursorY);
                    if(E.CursorX == currentRow->len){
                        if(E.CursorY + 1< E.Rows->size){
                            string *nextRow = getStringAtIndex(E.Rows, E.CursorY + 1);
    
                            stringAppendEnd(currentRow, nextRow->data, nextRow->byteLen);
                            listDeleteRow(E.Rows, E.CursorY + 1);
                            SET_FLAG(FLAG_DIRTY);
                        }
                    }else{
                        deleteCharFromPosList(E.Rows, E.CursorY, E.CursorX);
                        SET_FLAG(FLAG_DIRTY);
                    }
                    break;
                
            }
            break;
        }
        default:{
            //stringAppendEnd(&OutputBuffer, ci.arr, ci.byteCount);
            if(ci.arr[0] != 0 && ci.arr[0] != ESC_SEQ[0]){
                SET_FLAG(FLAG_DIRTY);
                insertCharAtPosList(ci, E.CursorY, E.CursorX);
                E.CursorX++;
            }
            return 0;
        }
    }

    FixCursorPossition();
}


/* Setting cursor possition (zero based) */
static void setCursorPossition(int x, int y){
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

void stringIntoInput(const char *str, int len){
    string *inputString = bufferCreateFromString(str, len);
    int bytesPassed = 0;
    for(int i = 0; i < inputString->len; i++){
        character_input ci = {0};
        ci.byteCount = inputString->byteCount[i];
        memcpy(ci.arr, inputString->data + bytesPassed, ci.byteCount);
        bytesPassed += ci.byteCount;

        processInput(ci);
    }
}

void editorLoadFile(char *path){
    int64 fileSize = getFileSize(path);
    if(fileSize == 0) die("Error reading file: %s", path);

    char *tempBuffer = malloc(fileSize + 1);
    tempBuffer[fileSize] = 0;

    uint32 Feedback = fileRead(path, tempBuffer, fileSize);
    if(Feedback != fileSize) 
        postEditorMessage(5, "Possible load error (Exp: %u | Rec: %u)", fileSize, Feedback);

    for(uint32 i = 0; i < Feedback; i++){
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

    free(tempBuffer);
    E.CursorX = 0;
    E.CursorY = 0;
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
    int verticallOffset = MAX_VAL(E.CursorY - maxRows, 0);
    int horizontalOffset = MAX_VAL(E.CursorX - maxCols, 0);
    E.OffsetX = horizontalOffset;
    E.OffsetY = verticallOffset;
#else
    int horizontalOffset = E.OffsetX;
    int verticallOffset = E.OffsetY;
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

            /* Deciding how much to write */
            int startOffset;
            int byteCount = stringCharToByteCount(row, horizontalOffset, 0, maxCols, &startOffset);
            if(byteCount == -1){
                WriteToBuffer(buffer, "<--");
            }else{
                stringAppendEnd(buffer, row->data + startOffset, byteCount);
            }
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
    char fileNameStr[128];

    int fNameLen;

    if(HAS_FLAG(FLAG_ALTVIEW)){
        fNameLen = snprintf(fileNameStr, 128, "%s (%3d)", "AltBuffer", GET_ALT_BUFFERID());
    }else{
        fNameLen = snprintf(fileNameStr, 128, "%s %3s", E.File, (HAS_FLAG(FLAG_DIRTY) ? "(m)" : ""));
    }
    
    WriteToBuffer(buffer, ESC_CLEAR_LINE);
    for(int i = 0; i < offsetLeft; i++) WriteToBuffer(buffer, " ");
    
    int feedback = snprintf(dest, width*2, "|| F: %s | LC: %4d | L: %4d | C: %4d | Mode: %s ||    |: %s :|",
        fileNameStr, E.Rows->size, E.CursorY + 1, E.CursorX, "UTF8", (*E.Message.Data == 0 ? "---" : E.Message.Data));
        
    WriteToBuffer(buffer, ESC_INVERTED_TEXT_COLOR ESC_TEXT_BOLD);
    WriteToBuffer(buffer, "    ");
    
    WriteToBuffer(buffer, dest);

    for(int i = 0; i < (width - feedback) - 4; i++) WriteToBuffer(buffer, " ");

    WriteToBuffer(buffer, ESC_RESET_TEXT_ATTRIBUTES);
    WriteToBuffer(buffer, "\r\n");
#else
    char bufferS[256];
    sprintf(bufferS, ESC_CLEAR_LINE "LineC: %d; curX: %d, curY: %d (%d), OffsetX: %d, OffsetY: %d, conSX: %d, conSY: %d, BufferId: (%d)%d\r\n", 
        E.Rows->size, E.CursorX, E.CursorY, E.CursorY + 1, E.OffsetX, E.OffsetY, ConInfo.Cols, ConInfo.Rows, HAS_FLAG(FLAG_ALTVIEW) != 0, GET_ALT_BUFFERID());
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

    setCursorPossition(E.CursorX - E.OffsetX, E.CursorY - E.OffsetY);
    Print(ESC_SHOW_CURSOR);
}

int main(int argc, char *argv[], char *envp[]){
    prepareConsole();
    SET_FLAG(FLAG_RUNNING | FLAG_SHOWHEADER | FLAG_SHOWNUMBERS | FLAG_INSERT_MODE);
    E.CursorX = 0;
    E.CursorY = 0;

    ConInfo = getConsoleSystemInfo();

    OutputBuffer = stringCreate(64);
    postEditorMessage(5, "Ctrl+q to quit");

    E.MainBuffer = createListWithRows(1);
    E.Rows = &E.MainBuffer;
    for(int i = 0; i < ALT_BUFFER_COUNT; i++){
        E.AltBuffers[i] = createListWithRows(1);
    }

    if(argc != 2){
        die("Argc was not 2: %d", argc);
    }

    strcpy(E.File, argv[1]);
    editorLoadFile(E.File);
    //E.File = argv[1];

#if 1
    while(E.Flags & FLAG_RUNNING){
        ConInfo = getConsoleSystemInfo();
        
        /* Message expiring */
        if(E.Message.Duration != 0){
            if(ConInfo.CurrentTime - E.Message.PostTime > E.Message.Duration){
                *E.Message.Data = 0;
                E.Message.Duration = 0;
                E.Message.PostTime = 0;
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
            E.Rows = (E.AltBuffers + GET_ALT_BUFFERID());
        }else{
            E.Rows = &E.MainBuffer;
        }

        /* Rendering to the terminal */
        editorRefreshScreen();

        /* Processing input: Reading from stdin, checking for special cases and adding to the main buffer */
        character_input ci = pollInput();
        if(ci.byteCount != 0) processInput(ci);
    }
#else
    string *str = stringCreateHeap(64);
    stringAppendEnd(str, "wawa1234", 8);
    character_input ci = {0};
    char big[3] = "š";
    ci.arr[0] = big[0];
    ci.arr[1] = big[1];
    ci.byteCount = 2;
    insertCharAtPossition(str, ci, 2, 1);
    insertCharAtPossition(str, ci, 2, 1);
    insertCharAtPossition(str, ci, 0, 1);
    puts(str->data);
    deleteCharFromPossition(str, 3);
    puts(str->data);
    
    getchar();
#endif
    return 0;
}