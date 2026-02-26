#ifndef MAIN_H
    #include <conio.h>
    #include "buffer.h"
    #include "fileF.h"

    //-----Debug Defines-----//
        #undef DEBUG_INFO

    //-----Struct Definitions-----//

        typedef struct{
            int CursorX, CursorY;
            int ConsoleRows;
            int ConsoleColumns;

            //int RowCount;
            uint32_t RowOffset;
            uint32_t ColumnOffset;
            StringBufferArray RowArrayOrigin;
            StringBufferArray RowArrayDisplay;

            uint64_t CurrentTime;
            uint8_t InsertMode;
            uint8_t EditorDirty;
        } EditorInfo;

        typedef struct{
            uint32_t TimeOfCreation;
            char Message[128];
        } EditorMessage;

    //-----Global Variables-----//

        HANDLE hStdin;
        HANDLE hStdout;
        HANDLE hStderr;
        HANDLE hHeap;
        HANDLE hFile;

        BOOL Running = 1;

        CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
        StringBuffer Buffer;
        EditorInfo Inf;

        char ArrowKeys = 0;
        char FileName[128];
        //char DebugMessage[128] = {0};

        EditorMessage DebugMessage;

    //-----MacroFunctions-----//

        // Prints the string to stdout
        #define Print(str) \
            WriteFile(hStdout, str, StringLength(str), NULL, NULL)

        // Prints a char to the stdout
        #define PrintChar(c) \
            WriteFile(hStdout, &c, 1, NULL, NULL)

        #define ClearScreenAfterCursor Print("\x1b[2J")
        #define ClearScreen Print("\x1b[1J")

        #define ResetCursorPossition SetCursorPossition(0, 0)

        #define ClearLine Print("\x1b[K")
        
        // Returns the key code of key k + CTRL
        #define CTRL_KEY(k) ((k) & 0x1f)

        #define SetCursorPossition(X, Y) \
            SetConsoleCursorPosition(hStdout, (COORD){X, Y})

        #define CTRL_DELETE_COMMANDS \
        { \
                uint32_t n; \
                if(*(target->Memory + LinePossition) == ' ') \
                    n = CountForwardToWord(target->Memory, LinePossition); \
                else \
                    n = CountForwardToBlank(target->Memory, LinePossition); \
    \
                int counter = n - LinePossition; \
                while(1) { \
                    if(counter == 0) break; \
                    counter--; \
                    StringShiftLeft(target->Memory, LinePossition, 0); \
                } \
        }\

    //-----Constants-----//

        #define UP_ARROW    (char)0x01  //0000 0001
        #define DOWN_ARROW  (char)0x02  //0000 0010
        #define LEFT_ARROW  (char)0x04  //0000 0100
        #define RIGHT_ARROW (char)0x08  //0000 1000

        #define CTRL_UP     (char)0xfe  //1111 1110
        #define CTRL_DOWN   (char)0xfd  //1111 1101
        #define CTRL_LEFT   (char)0xfb  //1111 1011
        #define CTRL_RIGHT  (char)0xf7  //1111 0111

        #define PAGE_DOWN   (char)0x10  //0001 0000
        #define PAGE_UP     (char)0x20  //0010 0000
        #define HOME_KEY    (char)0x40  //0100 0000
        #define END_KEY     (char)0x80  //1000 0000

        #define DELETE_KEY  (char)0x03  //0000 0011
        #define INSERT_KEY  (char)0x05  //0000 0101

        #define CTRL_DELETE (char)0x06  //0000 0110


        #define PAGE_UPDOWN_CONSTANT 10 // Unused

        #define TIMEOUT_MS 2 // Unused
        #define EDITOR_MESSAGE_TIME 15 // The (minimum) length in seconds of a screen message

        #define TAB_SPACE_COUNT 4 // How much spacebars will a tab create
        #define FIRST_LINE_EMPTY_FIELDS 6 // How many fields are empty in the first line before inverting the color
        #define LINE_NUMBER_WIDTH 4 // How long is the line number string


    //-----EndSequences-----//
        
        #define INVERTED_TEXT_COLOR "\x1b[7m"
        #define RESET_TEXT_ATTRIBUTES  "\x1b[m"

#define MAIN_H
#endif