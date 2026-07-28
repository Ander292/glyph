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

    string_list *Rows;
    string_list MainBuffer;
    string_list AltBuffers[ALT_BUFFER_COUNT];

    int TextOffsetTop, TextOffsetLeft;

    editor_message Message;
} E;

string OutputBuffer;

static int processInput(character_input ci){
    switch(ci.arr[0]){
        case CTRL_KEY('q'):
            UNSET_FLAG(FLAG_RUNNING);
            break;
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
        case '\b': // Backspace (or ctrl + h)
            break;
        case '\x1b':
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
                    break;
                
            }
            break;
        default:
            stringAppendEnd(&OutputBuffer, ci.arr, ci.byteCount);
            break;
    }


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

/* Rendering */

static inline void messageCreate(editor_message *dest, time_t duration, const char *text, ...){
    va_list args;
    va_start(args, text);
    vsnprintf(dest->Data, sizeof(dest->Data), text, args);
    va_end(args);
    //strcpy(dest->Data, text);
    dest->PostTime = ConInfo.CurrentTime;
    dest->Duration = duration;
}

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
    int feedback = sprintf(dest, "|| F: %s | LC: %4d | L: %4d | C: %4d | Mode: %s ||    |: %s :|",
        "placeholder", E.Rows->size, E.CursorY + 1, E.CursorX, "UTF8", (*E.Message.Data == 0 ? "---" : E.Message.Data));

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
    SET_FLAG(FLAG_RUNNING | FLAG_SHOWHEADER | FLAG_SHOWNUMBERS);
    E.CursorX = 0;
    E.CursorY = 0;

    ConInfo = getConsoleSystemInfo();

    OutputBuffer = stringCreate(64);
    postEditorMessage(5, "Ctrl+q to quit");

#if 1
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

#if 0
    while(E.Flags & FLAG_RUNNING){
        ConInfo = getConsoleSystemInfo();
        
        /* Message expiring */
        if(ConInfo.CurrentTime - E.Message.PostTime > E.Message.Duration){
            *E.Message.Data = 0;
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
    
    getchar();
#endif
    return 0;
}