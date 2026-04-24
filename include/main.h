#ifndef MAIN_H
    #include <conio.h>
    #include "buffer.h"
    #include "fileF.h"

    //-----Debug Defines-----//
        #undef DEBUG_INFO
        #define WINDOWS
        #define UNICODE
        #define _UNICODE
        
    //-----Struct Definitions-----//

        typedef struct{
            int CursorX, CursorY;
            int ConsoleRows;
            int ConsoleColumns;

            int RowOffset;
            int ColumnOffset;
            StringBufferArray RowArrayOrigin;
            StringBufferArray RowArray;

            uint64_t CurrentTime;
            uint8_t InsertMode;
            uint8_t EditorDirty;
            uint8_t StringMode;
            uint8_t ToRender;
            uint8_t ToFixCursor;
        } EditorInfo;

        typedef struct{
            uint64_t TimeOfCreation;
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

        EditorMessage DebugMessage;

    //-----MacroFunctions-----//

        #if defined WINDOWS

            // Prints the string to stdout
            #define Print(str) \
                WriteFile(hStdout, str, StringLength(str), NULL, NULL)

            // Prints a char to the stdout
            #define PrintChar(c) \
                WriteFile(hStdout, &c, 1, NULL, NULL)

            #define ResetCursorPossition SetCursorPossition(0, 0)

            #define SetCursorPossition(X, Y) \
                SetConsoleCursorPosition(hStdout, (COORD){X, Y})

        #elif defined LINUX

        #endif

        #define ClearLine Print("\x1b[K")

        #define ClearScreenAfterCursor Print("\x1b[2J")

        #define ClearScreen Print("\x1b[1J")
        
        // Returns the key code of key k + CTRL
        #define CTRL_KEY(k) ((k) & 0x1f)



        #define CTRL_DELETE_COMMANDS \
        

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

        #define CTRL_Q      (char)0x07  //0000 0111
        #define CTRL_C      (char)0x09  //0000 1001
        #define CTRL_S      (char)0x0A  //0000 1010
        //#define CTRL_D      (char)0x0B  //0000 1011

        #define CTRL_W      (char)0x0C  //0000 1100
        #define BACKSPACE   (char)0x0D  //0000 1101
        #define NEWLINE     (char)0x0E  //0000 1110
        #define TAB         (char)0x0F  //0000 1111
        #define CTRL_BACKSPACE CTRL_W


        #define PAGE_UPDOWN_CONSTANT 10 // Unused

        #define TIMEOUT_MS 10
        #define EDITOR_MESSAGE_TIME 8 // The (minimum) length in seconds of a screen message

        #define TAB_SPACE_COUNT 4 // How much spacebars will a tab create
        #define FIRST_LINE_EMPTY_FIELDS 6 // How many fields are empty in the first line before inverting the color
        #define LINE_NUMBER_WIDTH 4 // How long is the line number string

        #define MODE_UTF8 1
        #define MODE_UTF16 2

    //-----EndSequences-----//
        
        #define INVERTED_TEXT_COLOR "\x1b[7m"
        #define RESET_TEXT_ATTRIBUTES  "\x1b[m"


    //-----FunctionDeclarations-----//

        void TranslateStringArray();
        void PushEditorMessage(char *Str);

#define MAIN_H
#endif