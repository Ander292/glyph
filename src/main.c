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

#define ROWS_TO_SAVE (&E.MainBuffer)
int editorSave(char *path){
    string Final = stringCreate(1024);
    uint32 SizeCount = 0;
    for(int i = 0; i < ROWS_TO_SAVE->size; i++){
        string *row = getStringAtIndex(ROWS_TO_SAVE, i);
        SizeCount += row->byteLen;
        stringAppendEnd(&Final, row->data, row->byteLen);
        if(i < ROWS_TO_SAVE->size - 1){
            stringAppendEnd(&Final, "\r\n", 2);
            SizeCount += 2;
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
                postEditorMessage(20, "File not saved. Are u sure u want to quit? (yes, no, save)");
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
                            if(editorSave(E.File)) break;
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
        case CTRL_KEY('s'):
            //postEditorMessage(5, "Saving not implemented yet");
            editorSave(E.File);
            break;
        case '\b': // Backspace (or ctrl + h)
        case 127:  // Delete key in ascii but its backspace for some reason
        {
            SET_FLAG(FLAG_DIRTY);
            if(E.CursorX == 0){
                string *rowNext = getStringAtIndex(E.Rows, E.CursorY);
                if(E.CursorY != 0){
                    string *rowPrev = getStringAtIndex(E.Rows, E.CursorY-1);
                    E.CursorX = rowPrev->len;
                    stringAppendEnd(rowPrev, rowNext->data, rowNext->byteLen);
                    listDeleteRow(E.Rows, E.CursorY);
                    E.CursorY--;
                }
            }else{
                deleteCharFromPosList(E.Rows, E.CursorY, E.CursorX - 1);
                E.CursorX--;
            }
            break;
        }
        case '\r':{
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
        case '\n':
            break;
        // case 127: // Delete key
        //     postEditorMessage(5, "Delete key ):");
        //     SET_FLAG(FLAG_DIRTY);
        //     break;
        case '\x1b':{
            switch(ci.arr[2]){
                case 'A': // Up arrow
                    E.CursorY--;
                    break;
                case 'B': // Down arrow
                    E.CursorY++;
                    break;
                case 'C': // Right arrow
                    E.CursorX++;
                    break;
                case 'D': // Left arrow
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
                    SET_FLAG(FLAG_DIRTY);
                    string *currentRow = getStringAtIndex(E.Rows, E.CursorY);
                    if(E.CursorX == currentRow->len){
                        if(E.CursorY + 1< E.Rows->size){
                            string *nextRow = getStringAtIndex(E.Rows, E.CursorY + 1);
    
                            stringAppendEnd(currentRow, nextRow->data, nextRow->byteLen);
                            listDeleteRow(E.Rows, E.CursorY + 1);
                        }
                    }else{
                        deleteCharFromPosList(E.Rows, E.CursorY, E.CursorX);
                    }
                    break;
                
            }
            break;
        }
        default:{
            SET_FLAG(FLAG_DIRTY);
            //stringAppendEnd(&OutputBuffer, ci.arr, ci.byteCount);
            insertCharAtPosList(ci, E.CursorY, E.CursorX);
            E.CursorX++;
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

    stringIntoInput(tempBuffer, Feedback);

    free(tempBuffer);
    E.CursorX = 0;
    E.CursorY = 0;
    FixCursorPossition();
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

    WriteToBuffer(buffer, ESC_CLEAR_LINE);
    for(int i = 0; i < offsetLeft; i++) WriteToBuffer(buffer, " ");

    WriteToBuffer(buffer, ESC_INVERTED_TEXT_COLOR ESC_TEXT_BOLD);
    int feedback = sprintf(dest, "|| F: %s %3s | LC: %4d | L: %4d | C: %4d | Mode: %s ||    |: %s :|",
        E.File, (HAS_FLAG(FLAG_DIRTY) ? "(m)" : ""), E.Rows->size, E.CursorY + 1, E.CursorX, "UTF8", (*E.Message.Data == 0 ? "---" : E.Message.Data));

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

#if 0
/* Test code */
    string *tempBuffer = stringCreateHeap(64);
    stringAppendEnd(tempBuffer, 
        "I'm 67 monster imposter. Please kick me. 67 monster imposter. six seven monster imposter. Testing if letters are trully broken or its me", 
        sizeof("I'm 67 monster imposter. Please kick me. 67 monster imposter. six seven monster imposter. Testing if letters are trully broken or its me") - 1);

    string *tempBuffer1 = stringCreateHeap(64);
    stringAppendEnd(tempBuffer1, "random ass2", sizeof("random ass2") - 1);

    string *tempBuffer2 = stringCreateHeap(64);
    stringAppendEnd(tempBuffer2, "random ass3", sizeof("random ass3") - 1);

    string *tempBuffer3 = stringCreateHeap(64);
    stringAppendEnd(tempBuffer3, "random ass5", sizeof("random ass5") - 1);

    E.MainBuffer = createList();
    E.AltBuffers[0] = createList();
    E.Rows = &E.MainBuffer;

    listAppendEnd(E.Rows, tempBuffer);
    listAppendEnd(E.Rows, tempBuffer1);
    listAppendEnd(E.Rows, tempBuffer2);
    listInsertAtPossition(E.Rows, tempBuffer3, 5);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);

    E.Rows = E.AltBuffers + 0;
    listAppendEnd(E.Rows, tempBuffer);
    listAppendEnd(E.Rows, tempBuffer1);
    listAppendEnd(E.Rows, tempBuffer2);
    listInsertAtPossition(E.Rows, tempBuffer3, 5);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    listAppendEnd(E.Rows, tempBuffer2);
    /* End of test code */
#endif

    E.MainBuffer = createList();
    E.AltBuffers[0] = createList();
    E.Rows = &E.MainBuffer;
    string *temp = stringCreateHeap(64);
    listAppendEnd(E.Rows, temp);

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