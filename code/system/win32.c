#ifndef WIN32_C
#define WIN32_c

#include <stdio.h>
#include <windows.h>
#include "res/escape.h"
#include "types.h"
#include "io.h"

DWORD OldConsoleModeIn;
DWORD OldConsoleModeOut;
HANDLE hStdin;
HANDLE hStdout;
HANDLE hStderr;

static inline char readByte();

/* Terminal */
void term_disableRawMode(){
    DWORD ConsoleMode = OldConsoleModeIn;
    SetConsoleMode(hStdin, ConsoleMode);
    ConsoleMode = OldConsoleModeOut;
    SetConsoleMode(hStdout, ConsoleMode);

    PrintA(ESC_MOVE_TO_MAIN_BUFFER);
    PrintA(ESC_ENABLE_CURSOR_BLINKING);
    PrintA(ESC_RESET_CURSOR_ATTRIBUTES);
}

void term_enableRawMode(){
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
    PrintA(ESC_DISABLE_CURSOR_BLINKING);
    PrintA(ESC_FORCE_STATIC_CURSOR);
}

console_info term_GetConsoleSystemInfo(){
    console_info Result;
    CONSOLE_SCREEN_BUFFER_INFO si;
    GetConsoleScreenBufferInfo(hStdout, &si);
    Result.Rows = si.dwSize.Y;
    Result.Cols = si.dwSize.X;

    Result.CurrentTime = time(NULL);
    return Result;
}

void term_PrepareConsole(){
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    hStderr = GetStdHandle(STD_ERROR_HANDLE);
    term_enableRawMode();
    PrintA(ESC_MOVE_TO_AUX_BUFFER);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    FlushConsoleInputBuffer(hStdin);
    atexit(term_disableRawMode);
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

/* Input output */
static inline char readByte(){
    char result;
    DWORD readFeedback;
    if(!ReadFile(hStdin, &result, 1, &readFeedback, NULL)){
        Die("Error reading stdin. (readByte)");
    }

    return result;
}

u32 writeOutput(const char *src, u32 size){
    DWORD Feedback;

    if((!WriteFile(hStdout, src, size, &Feedback, NULL)) || (Feedback != size)){
        Die("Error writting to stdout. %p R: %u F: %u", src, size, Feedback);
    }

    return Feedback;
}

// TODO: Better error handling for these functions!!!

i64 io_FileWrite(const char *destPath, char *string, u32 size){
    HANDLE hOutFile = CreateFileA(destPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, NULL);
    if(hOutFile == INVALID_HANDLE_VALUE){
        //WIN32_ErrorExit("CreateFile failed for path: %s", destPath);
        return -1;
    }
    DWORD Feedback;
    if(!WriteFile(hOutFile, string, size, &Feedback, NULL)){
        return -1;
    }

    CloseHandle(hOutFile);

    return (i64)Feedback;
}

i64 io_FileRead(const char *filePath, char *destBuffer, u32 maxBufferSize){
    HANDLE hInFile = CreateFileA(filePath, GENERIC_READ, 0, NULL, OPEN_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, NULL);
    if(hInFile == INVALID_HANDLE_VALUE){
        //Die("CreateFile failed for path: %s", filePath);
        return -1;
    }
    DWORD Feedback;
    if(!ReadFile(hInFile, destBuffer, maxBufferSize, &Feedback, NULL)){
        //Die("Read file failed (requested %u, read %u bytes)", maxBufferSize, Feedback);
        return -1;
    }

    CloseHandle(hInFile);
    return Feedback;
}

i64 io_GetFileSize(const char *filePath){
    WIN32_FILE_ATTRIBUTE_DATA fi;
    if(!GetFileAttributesExA(filePath, GetFileExInfoStandard, &fi)){
        return -1;
    }
    return (i64)((u64)fi.nFileSizeLow | ((u64)fi.nFileSizeHigh << 32));
}

#endif