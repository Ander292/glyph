#ifndef WIN32_C
#define WIN32_c

#include <windows.h>
#include "escape.h"
#include "types.h"

DWORD OldConsoleModeIn;
DWORD OldConsoleModeOut;
HANDLE hStdin;
HANDLE hStdout;
HANDLE hStderr;

/* Terminal */
void term_DisableRawMode(){
    DWORD ConsoleMode = OldConsoleModeIn;
    SetConsoleMode(hStdin, ConsoleMode);
    ConsoleMode = OldConsoleModeOut;
    SetConsoleMode(hStdout, ConsoleMode);

    Print(ESC_MOVE_TO_MAIN_BUFFER);
    Print(ESC_ENABLE_CURSOR_BLINKING);
    Print(ESC_RESET_CURSOR_ATTRIBUTES);
}

void term_EnableRawMode(){
    DWORD ConsoleMode;

    /* Stdin */
    GetConsoleMode(hStdin, &ConsoleMode);
    OldConsoleModeIn = ConsoleMode;

    ConsoleMode = ConsoleMode & ~(ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT | 
        ENABLE_LINE_INPUT | ENABLE_MOUSE_INPUT | 
        ENABLE_WINDOW_INPUT) | (ENABLE_VIRTUAL_TERMINAL_INPUT);
    SetConsoleMode(hStdin, ConsoleMode);

    /* Stdout */
    GetConsoleMode(hStdout, &ConsoleMode);
    OldConsoleModeOut = ConsoleMode;

    ConsoleMode = ConsoleMode & ~(ENABLE_WRAP_AT_EOL_OUTPUT);
    SetConsoleMode(hStdout, ConsoleMode);
    Print(ESC_DISABLE_CURSOR_BLINKING);
    Print(ESC_FORCE_STATIC_CURSOR);
}

// console_info getConsoleSystemInfo(){
//     console_info Result;
//     CONSOLE_SCREEN_BUFFER_INFO si;
//     GetConsoleScreenBufferInfo(hStdout, &si);
//     Result.Rows = si.dwSize.Y;
//     Result.Cols = si.dwSize.X;

//     Result.CurrentTime = time(NULL);
//     return Result;
// }

void term_PrepareConsole(){
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    hStderr = GetStdHandle(STD_ERROR_HANDLE);
    enableRawMode();
    Print(ESC_MOVE_TO_AUX_BUFFER);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    FlushConsoleInputBuffer(hStdin);
    atexit(term_DisableRawMode);
}

static inline LPSTR WIN32_formatError(DWORD dwError){
    LPSTR MessageBuffer;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_MAX_WIDTH_MASK,
        NULL, dwError, 0, 
        (LPSTR)&MessageBuffer,
        0, NULL
    );

    return MessageBuffer;
}

static void WIN32_errorExit(char *format, ...){
    char str[256];

    va_list args;
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);

    DWORD ErrorCode = GetLastError();
    LPSTR WindowsErrorString = WIN32_formatError(ErrorCode);

    printf("%s : (%u) %s\n", format, ErrorCode, WindowsErrorString);

    readByteBlock();
    exit(2);
}

/* Input output */
static inline char readByteBlock(){
    char result;
    DWORD readFeedback;
    if(!ReadFile(hStdin, &result, 1, &readFeedback, NULL)){
        exit(1);
    }

    return result;
}

static inline u32 writeOutput(char *src, u32 size){
    DWORD Feedback;

    if((!WriteFile(hStdout, src, size, &Feedback, NULL)) || (Feedback != size)){
        WIN32_ErrorExit("Error writting to stdout. %p R: %u F: %u", src, size, Feedback);
    }

    return Feedback;
}

#endif