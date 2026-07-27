#include "types.h"
#include "system.h"
#include "main.h"
#include "utf8.h"

#include <stdio.h>

static console_info ConInfo;

static struct{
    uint32 Flags;
    int CursorX, CursorY; /* The possition of the cursor inside the internal buffer */
    int offX, offY;
    string_list Rows;
    int TextOffsetTop, TextOffsetLeft;
} E;

string OutputBuffer;

int processInput(character_input ci){
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
                    E.offY -= (ConInfo.Rows - E.TextOffsetTop);
                    break;
                case '6': // Pagedown (followed by ~ but not checked)
                    //E.CursorY += (ConInfo.Rows - 2);
                    E.CursorY += (ConInfo.Rows - E.TextOffsetTop);
                    E.offY += (ConInfo.Rows - E.TextOffsetTop);
                    // E.CursorY = E.offY + ConInfo.Rows - 1;
                    // if(E.CursorY > E.Rows.size - 1) E.CursorY = E.Rows.size - 1;
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
                    string *str = getStringAtIndex(&E.Rows, E.CursorY);
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
    string *str = getStringAtIndex(&E.Rows, E.CursorY);
    int currentWidth = 0;
    if(str != NULL) currentWidth = str->len;
    int destWidth = MIN_VAL(ConInfo.Cols, currentWidth);

    if(E.CursorX < 0){
        E.CursorX = 0;
    }else if(E.CursorX > currentWidth){
        E.CursorX = currentWidth;
    }

    // Vertical offset
    if((E.CursorY - E.offY > ConInfo.Rows - 1 - E.TextOffsetTop) && E.CursorY < E.Rows.size){
        E.offY = MAX_VAL(0, (E.CursorY) - (ConInfo.Rows - E.TextOffsetTop) + 1);
    }else if((E.CursorY - E.offY < 0)){
        E.offY = MAX_VAL(0, E.offY - (E.offY - E.CursorY));
    }

    // Horizontal offset
    if(E.CursorX - E.offX > ConInfo.Cols - 1 - E.TextOffsetLeft){
        E.offX = MAX_VAL(0, E.CursorX - (ConInfo.Cols - E.TextOffsetLeft) + 1);
    }else if((E.CursorX - E.offX < 0)){
        E.offX = MAX_VAL(0, E.offX - (E.offX - E.CursorX));
    }

    // Verticall offset clamping
    if(E.offY < 0) E.offY = 0;
#if 0
    if(E.offY > E.Rows.size - (ConInfo.Rows - E.TextOffsetTop)) 
        E.offY = E.Rows.size - (ConInfo.Rows - E.TextOffsetTop);
#else
    if(E.offY > E.Rows.size - 1) E.offY = E.Rows.size - 1;
#endif

    if(E.CursorY < 0){
        E.CursorY = 0;
    }else if(E.CursorY > E.Rows.size - 1){
        E.CursorY = E.Rows.size - 1;
    }
}

/* Setting cursor possition (zero based) */
void setCursorPossition(int x, int y){
    char c[16];
    x++; y++;

    int maxRows = ConInfo.Rows;
    int maxCols = ConInfo.Cols;

#if 0
    /* Calculating the real possition on the screen */
    if(HAS_FLAG(FLAG_SHOWHEADER)) {
        y += 1;
        maxRows -= 1;
    }
    if(HAS_FLAG(FLAG_SHOWNUMBERS)) {
        x += 4;
        maxCols -= 4;
    }else {
        x += 1;
        maxCols -= 1;
    }
#else
    x += E.TextOffsetLeft;
    //maxCols -= E.TextOffsetLeft;
    y += E.TextOffsetTop;
    //maxRows -= E.TextOffsetTop;
#endif


    if(x > maxCols) x = maxCols;
    if(y > maxRows) y = maxRows;

    sprintf(c, ESC_SEQ "%d;%dH", (y), (x));
    Print(c);
}

void drawRows(string *buffer){
    char rowNum[5];
    int maxRows = ConInfo.Rows;
    if(HAS_FLAG(FLAG_SHOWHEADER)) maxRows -= 1;

    int maxCols = ConInfo.Cols - 1;
    if(HAS_FLAG(FLAG_SHOWNUMBERS)) maxCols -= 3;

    /* Calculating verticall offset */
#if 0
    int verticallOffset = MAX_VAL(E.CursorY - maxRows, 0);
    int horizontalOffset = MAX_VAL(E.CursorX - maxCols, 0);
    E.offX = horizontalOffset;
    E.offY = verticallOffset;
#else
    int horizontalOffset = E.offX;
    int verticallOffset = E.offY;
#endif
    int targetRows = MIN_VAL(E.Rows.size - verticallOffset, maxRows);
    for(int i = 0; i < maxRows; i++){
        WriteToBuffer(buffer, ESC_CLEAR_LINE);
        if(i < targetRows){
            if(HAS_FLAG(FLAG_SHOWNUMBERS)){
                sprintf(rowNum, "%3d|", (i + verticallOffset + 1) % 1000);
                WriteToBuffer(buffer, rowNum);
            }else{
                WriteToBuffer(buffer, "~");
            }

            string *row = getStringAtIndex(&E.Rows, i + verticallOffset);
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

void formatHeader(string *buffer){
#if 0
    int offsetLeft = 2;
    int width = ConInfo.Cols;
    for(int i = 0; i < offsetLeft; i++) WriteToBuffer(buffer, " ");
    WriteToBuffer(buffer, ESC_INVERTED_TEXT_COLOR);
    for(int i = 0; i < width - offsetLeft; i++) WriteToBuffer(buffer, " ");
    WriteToBuffer(buffer, ESC_RESET_TEXT_ATTRIBUTES);
    WriteToBuffer(buffer, "\r\n");
#else
    char bufferS[256];
    sprintf(bufferS, ESC_CLEAR_LINE "LineC: %d; curX: %d, curY: %d (%d), offX: %d, offY: %d, conSX: %d, conSY: %d\r\n", 
        E.Rows.size, E.CursorX, E.CursorY, E.CursorY + 1, E.offX, E.offY, ConInfo.Cols, ConInfo.Rows);
    WriteToBuffer(buffer, bufferS);
#endif
}

void editorRefreshScreen(){
    /* Reseting cursor */
    Print(ESC_HIDE_CURSOR);
    Print(ESC_RESET_CURSOR_POSSITION);

    /* Formating output */
    if(HAS_FLAG(FLAG_SHOWHEADER)) formatHeader(&OutputBuffer);
    drawRows(&OutputBuffer);
    
    
    /* Printing the output buffer */
    PrintBuffer(OutputBuffer);
    clearBuffer(&OutputBuffer);

    setCursorPossition(E.CursorX - E.offX, E.CursorY - E.offY);
    Print(ESC_SHOW_CURSOR);
}

int main(int argc, char *argv[], char *envp[]){
    prepareConsole();
    SET_FLAG(FLAG_RUNNING | FLAG_SHOWHEADER | FLAG_SHOWNUMBERS);
    E.CursorX = 0;
    E.CursorY = 0;

    ConInfo = getConsoleSystemInfo();

    OutputBuffer = stringCreate(64);

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

    E.Rows = createList();
    listAppendEnd(&E.Rows, tempBuffer);
    listAppendEnd(&E.Rows, tempBuffer1);
    listAppendEnd(&E.Rows, tempBuffer2);
    listInsertAtPossition(&E.Rows, tempBuffer3, 5);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);
    listAppendEnd(&E.Rows, tempBuffer2);

#if 1
    while(E.Flags & FLAG_RUNNING){
        ConInfo = getConsoleSystemInfo();
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
        /* Rendering to the terminal */
        editorRefreshScreen();

        /* Processing input: Reading from stdin, checking for special cases and adding to the main buffer */
        character_input ci = pollInput();
        if(ci.byteCount != 0) processInput(ci);
    }
#else
    for(int i = 0; i < 1000; i++){
        printf("%d:%d\n", i, powerOfTwoRoundUp(i));
    }
    getchar();
#endif
    return 0;
}