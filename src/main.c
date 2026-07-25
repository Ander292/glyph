#include "types.h"
#include "system.h"
#include "main.h"
#include "utf8.h"

#include <stdio.h>

static console_info ConInfo;

static struct{
    uint32 Flags;
    int CursorX, CursorY; /* The possition of the cursor inside the internal buffer */
    string_list Rows;
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
                    E.CursorY -= (ConInfo.Rows - 2);
                    break;
                case '6': // Pagedown
                    E.CursorY += (ConInfo.Rows - 2);
                    break;
                case 'O':
                    if(ci.arr[3] == 'H') goto HOME;
                    else if(ci.arr[3] == 'F') goto END;
                    break;
                case '1': // Home key
                case '7':
                case 'H':
                    HOME:
                    E.CursorX = 0;
                    break;
                case '4': // End key
                case '8':
                case 'F':
                    END:
                    E.CursorX = ConInfo.Cols;
                    break;
                case '3': // Delete key
                    break;
                
            }
            break;
        default:
            stringAppendEnd(&OutputBuffer, ci.arr, ci.byteCount);
            break;
    }

    /* Fixing the internal buffer cursor possition */
    if(E.CursorX < 0) E.CursorX = 0;
    if(E.CursorY < 0) E.CursorY = 0;
    if(E.CursorX > ConInfo.Cols) E.CursorX = ConInfo.Cols;
    if(E.CursorY > MIN_VAL(ConInfo.Rows, E.Rows.size)) E.CursorY = MIN_VAL(ConInfo.Rows, E.Rows.size);
}

/* Setting cursor possition (zero based) */
void setCursorPossition(int x, int y){
    char c[16];

    /* Calculating the real possition on the screen */
    if(HAS_FLAG(FLAG_SHOWHEADER)) y += 1;
    if(HAS_FLAG(FLAG_SHOWNUMBERS)) x += 4;
    else x += 1;

    if(x >= ConInfo.Cols - 1) x = ConInfo.Cols - 1;
    if(y >= ConInfo.Rows - 1) y = ConInfo.Rows - 1;

    sprintf(c, ESC_SEQ "%d;%dH", (y + 1) % ConInfo.Rows, (x + 1) % ConInfo.Cols);
    Print(c);
}

void drawRows(string *buffer){
    char rowNum[5];
    int maxRows = ConInfo.Rows - 1;
    if(HAS_FLAG(FLAG_SHOWHEADER)) maxRows -= 1;

    int maxCols = ConInfo.Cols - 1;
    if(HAS_FLAG(FLAG_SHOWNUMBERS)) maxCols -= 3;

    /* Calculating verticall offset */
    int startIndex = MAX_VAL(E.CursorY - maxRows, 0);
    int horizontalOffset = MAX_VAL(E.CursorX - maxCols, 0);

    int targetRows = MIN_VAL(E.Rows.size - startIndex, maxRows);
    for(int i = 0; i < targetRows; i++){
        WriteToBuffer(buffer, ESC_CLEAR_LINE);

        if(HAS_FLAG(FLAG_SHOWNUMBERS)){
            sprintf(rowNum, "%3d|", (i + startIndex + 1) % 1000);
            WriteToBuffer(buffer, rowNum);
        }else{
            WriteToBuffer(buffer, "~");
        }

        string *row = getStringAtIndex(&E.Rows, i + startIndex);
        if(row == NULL) die("Row was null, im out");

        /* Deciding how much to write */
        int startOffset;
        int byteCount = stringCharToByteCount(row, horizontalOffset, 0, maxCols, &startOffset);
        if(byteCount == -1){
            WriteToBuffer(buffer, "<--");
        }else{
            stringAppendEnd(buffer, row->data + startOffset, byteCount);
        }

        if(i < maxRows - 1)
            WriteToBuffer(buffer, "\r\n");
    }
}

void formatHeader(string *buffer){
    int offsetLeft = 2;
    int width = ConInfo.Cols;
    for(int i = 0; i < offsetLeft; i++) WriteToBuffer(buffer, " ");
    WriteToBuffer(buffer, ESC_INVERTED_TEXT_COLOR);
    for(int i = 0; i < width - offsetLeft; i++) WriteToBuffer(buffer, " ");
    WriteToBuffer(buffer, ESC_RESET_TEXT_ATTRIBUTES);
    WriteToBuffer(buffer, "\r\n");
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

    setCursorPossition(E.CursorX, E.CursorY);
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
    stringAppendEnd(tempBuffer, "random ass", sizeof("random ass") - 1);

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


    while(E.Flags & FLAG_RUNNING){
        /* Rendering to the terminal */
        editorRefreshScreen();

        /* Processing input: Reading from stdin, checking for special cases and adding to the main buffer */
        character_input ci = pollInput();
        if(ci.byteCount != 0) processInput(ci);
    }

    return 0;
}