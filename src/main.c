#include "types.h"
#include "system.h"
#include "main.h"
#include "utf8.h"

#include <stdio.h>

static console_info ConInfo;

static struct{
    uint32 Flags;
    int CursorX, CursorY; /* The possition of the cursor inside the internal buffer */
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
        case CTRL_KEY('h'):
            TOGGLE_FLAG(FLAG_SHOWHEADER);
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
                case '4':
                case '8':
                case 'F':
                    END:
                    E.CursorX = ConInfo.Cols;
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
    if(E.CursorY > ConInfo.Rows - 3) E.CursorY = ConInfo.Rows - 3;
}

/* Setting cursor possition (zero based) */
void setCursorPossition(int x, int y){
    char c[16];

    /* Calculating the real possition on the screen */
    if(HAS_FLAG(FLAG_SHOWHEADER)) y += 1;
    if(HAS_FLAG(FLAG_SHOWNUMBERS)) x += 4;
    else x += 1;

    sprintf(c, ESC_SEQ "%d;%dH", (y + 1) % ConInfo.Rows, (x + 1) % ConInfo.Cols);
    Print(c);
}

void drawRows(string *buffer){
    char rowNum[5];
    int targetRows = ConInfo.Rows - 1;
    if(HAS_FLAG(FLAG_SHOWHEADER)) targetRows -= 1;

    for(int i = 0; i < targetRows; i++){
        WriteToBuffer(buffer, ESC_CLEAR_LINE);

        if(HAS_FLAG(FLAG_SHOWNUMBERS)){
            sprintf(rowNum, "%3d|", (i + 1) % 1000);
            WriteToBuffer(buffer, rowNum);
        }else{
            WriteToBuffer(buffer, "~");
        }

        if(i < targetRows - 1)
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

    while(E.Flags & FLAG_RUNNING){
        /* Rendering to the terminal */
        editorRefreshScreen();

        /* Processing input: Reading from stdin, checking for special cases and adding to the main buffer */
        character_input ci = pollInput();
        if(ci.byteCount != 0) processInput(ci);

    }

    return 0;
}