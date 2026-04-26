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
            wchar Message[128];
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

        wchar ArrowKeys = 0;
        wchar FileName[128];

        EditorMessage DebugMessage;

    //-----EndSequences-----//
        
        #define ESC_SEQ "\x1b["
        #define ESC(c) ESC_SEQ c

        #define ESC_HIDE_CURSOR ESC("?25l")
        #define ESC_SHOW_CURSOR ESC("?25h")

        #define MOVE_TO_AUX_BUFFER  ESC("?1049h")
        #define MOVE_TO_MAIN_BUFFER ESC("?1049l")

        #define INVERTED_TEXT_COLOR L"\x1b[7m"
        #define RESET_TEXT_ATTRIBUTES  L"\x1b[m"



    //-----MacroFunctions-----//

        #if defined WINDOWS

            #define PrintAL(str, len) \
                WriteFile(hStdout, (str), (len), NULL, NULL)

            // Prints the string to stdout (possibly convert to a function and use WriteConsoleOutputW)
            #define PrintA(str) \
                PrintAL((str), StringLengthA(str))

            #define Print(str) \
                WriteFile(hStdout, str, StringLength(str) * 2, NULL, NULL)


            // Prints a wchar to the stdout
            #define PrintChar(c) \
                WriteFile(hStdout, &c, 2, NULL, NULL)
#if 0
            #define ResetCursorPossitionM SetCursorPossition(0, 0)

            #define SetCursorPossitionM(X, Y) \
                SetConsoleCursorPosition(hStdout, (COORD){X, Y})
#else

#endif
        #elif defined LINUX

        #endif

        #define ClearLine Print("\x1b[K")

        #define ClearScreenAfterCursor Print("\x1b[2J")

        #define ClearScreen Print("\x1b[1J")
        
        // Returns the key code of key k + CTRL
        #define CTRL_KEY(k) ((k) & 0x1f)        

    //-----Constants-----//

        #define UP_ARROW    (wchar)0x01  //0000 0001
        #define DOWN_ARROW  (wchar)0x02  //0000 0010
        #define LEFT_ARROW  (wchar)0x04  //0000 0100
        #define RIGHT_ARROW (wchar)0x08  //0000 1000

        #define CTRL_UP     (wchar)0xfe  //1111 1110
        #define CTRL_DOWN   (wchar)0xfd  //1111 1101
        #define CTRL_LEFT   (wchar)0xfb  //1111 1011
        #define CTRL_RIGHT  (wchar)0xf7  //1111 0111

        #define PAGE_DOWN   (wchar)0x10  //0001 0000
        #define PAGE_UP     (wchar)0x20  //0010 0000
        #define HOME_KEY    (wchar)0x40  //0100 0000
        #define END_KEY     (wchar)0x80  //1000 0000

        #define DELETE_KEY  (wchar)0x03  //0000 0011
        #define INSERT_KEY  (wchar)0x05  //0000 0101

        #define CTRL_DELETE (wchar)0x06  //0000 0110

        #define CTRL_Q      (wchar)0x07  //0000 0111
        #define CTRL_C      (wchar)0x09  //0000 1001
        #define CTRL_S      (wchar)0x0A  //0000 1010
        //#define CTRL_D      (wchar)0x0B  //0000 1011

        #define CTRL_W      (wchar)0x0C  //0000 1100
        #define BACKSPACE   (wchar)0x0D  //0000 1101
        #define NEWLINE     (wchar)0x0E  //0000 1110
        #define TAB         (wchar)0x0F  //0000 1111
        #define CTRL_BACKSPACE CTRL_W


        #define PAGE_UPDOWN_CONSTANT 10 // Unused

        #define TIMEOUT_MS 5
        #define EDITOR_MESSAGE_TIME 8 // The (minimum) length in seconds of a screen message

        #define TAB_SPACE_COUNT 4 // How much spacebars will a tab create
        #define FIRST_LINE_EMPTY_FIELDS 6 // How many fields are empty in the first line before inverting the color
        #define LINE_NUMBER_WIDTH 4 // How long is the line number string

        #define MODE_UTF8 1
        #define MODE_UTF16 2


    //-----FunctionDeclarations-----//

        void TranslateStringArray();
        void PushEditorMessage(wchar *Str);
        static inline void ResetCursorPossition();
        static inline void SetCursorPossition(int x, int y);

#define MAIN_H
#endif