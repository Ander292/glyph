#include "system.h"
#include <stdarg.h>
#include <stdio.h>

/* Platform specific code */
#if defined WINDOWS

DWORD OldConsoleModeIn;
DWORD OldConsoleModeOut;
HANDLE hStdin;
HANDLE hStdout;
HANDLE hStderr;

void disableRawMode(){
    DWORD ConsoleMode = OldConsoleModeIn;
    SetConsoleMode(hStdin, ConsoleMode);
    ConsoleMode = OldConsoleModeOut;
    SetConsoleMode(hStdout, ConsoleMode);

    Print(ESC_MOVE_TO_MAIN_BUFFER);
}

void enableRawMode(){
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
}

console_info getConsoleSystemInfo(){
    console_info Result;
    CONSOLE_SCREEN_BUFFER_INFO si;
    GetConsoleScreenBufferInfo(hStdout, &si);
    Result.Rows = si.dwSize.Y;
    Result.Cols = si.dwSize.X;

    Result.CurrentTime = time(NULL);
    return Result;
}

void prepareConsole(){
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    hStderr = GetStdHandle(STD_ERROR_HANDLE);
    enableRawMode();

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    FlushConsoleInputBuffer(hStdin);
    atexit(disableRawMode);
    Print(ESC_MOVE_TO_AUX_BUFFER);
}

LPSTR WIN32_FormatError(DWORD dwError){
    LPSTR MessageBuffer;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dwError, 0, 
        (LPSTR)&MessageBuffer,
        0, NULL
    );

    return MessageBuffer;
}

static void WIN32_ErrorExit(char *format, ...){
    char str[256];

    va_list args;
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);

    DWORD ErrorCode = GetLastError();
    LPSTR WindowsErrorString = WIN32_FormatError(ErrorCode);

    printf("%s : (%u) %s\n", format, ErrorCode, WindowsErrorString);

    getchar();
    exit(2);
}

int GetNumberOfCharInputEvents(){
    DWORD TotalEvents = 0;
    
    // Get the total number of events in the buffer
    if(!GetNumberOfConsoleInputEvents(hStdin, &TotalEvents)) {
        WIN32_ErrorExit("Fatal error reading input events");
    }
    if(TotalEvents == 0){
        return 0;
    }

    if(TotalEvents > 100){
        int testVar = 67 + 68;
    }

    INPUT_RECORD records[TotalEvents];
    DWORD EventsRead = 0;

    if(!PeekConsoleInputA(hStdin, records, TotalEvents, &EventsRead)) {
        return 0;
    }

    // Count only the keyboard events
    int CharMsgCount = 0;
    for(DWORD i = 0; i < EventsRead; ++i){
        if(records[i].EventType == KEY_EVENT){
            KEY_EVENT_RECORD key = records[i].Event.KeyEvent;
            
            if(key.bKeyDown && key.uChar.UnicodeChar != L'\0'){
                CharMsgCount++;
            }
        }
    }

    return CharMsgCount;
}

/* Nonblocking character input. Returns 1 byte */
character_input pollInput(){
    character_input Result = {0};
    DWORD Feedback;

    unsigned long elapsed = 0;
    unsigned long interval = 1; // Poll granularity in ms
    while(elapsed < TIMEOUT_MS){
        switch(GetFileType(hStdin)){
            case FILE_TYPE_CHAR:
                Feedback = GetNumberOfCharInputEvents();
                break;
            case FILE_TYPE_PIPE:
                PeekNamedPipe(hStdin, NULL, 0, NULL, &Feedback, NULL);
                break;
            default:
                printf("Error reading stdin : Invalid stdin handle!\n");
                break;
        }
        if(Feedback != 0){
            if(!ReadFile(hStdin, Result.arr, 1, &Feedback, NULL)){
                WIN32_ErrorExit("Error reading stdin");
            }

            int byteCount = charGetByteCount(Result.arr[0]);
            /* Rudimentary escape sequence support*/
            if(Result.arr[0] == '\x1b'){
                byteCount = 3; 
            }
            for(int i = 1; i < byteCount; i++){
                if(!ReadFile(hStdin, Result.arr + i, 1, &Feedback, NULL)){
                    WIN32_ErrorExit("Error reading stdin");
                }
                if(Result.arr[i] <= '9' && Result.arr[i] >= '0' || Result.arr[i] == 'O') byteCount = 4;
            }

            Result.byteCount = byteCount;

            break;
        }
        Sleep(interval);
        elapsed += interval;
    }

    return Result;
}

uint32 writeOutput(char *src, uint32 size){
    DWORD Feedback;

    if((!WriteFile(hStdout, src, size, &Feedback, NULL)) || (Feedback != size)){
        WIN32_ErrorExit("Error writting to stdout. %p R: %u F: %u", src, size, Feedback);
    }

    return Feedback;
}

#elif defined LINUX

struct termios orig_termios;

void disableRawMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode(){
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void PrepareConsole(){
    enterRawMode();
}

char pollInput(){
    fd_set readfds; 
    struct timeval timeout;
    char result = 0;
    
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    timeout.tv_sec = TIMEOUT_MS / 1000;
    timeout.tv_usec = (TIMEOUT_MS - timeout.tv_sec * 1000) * 1000;

    int ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

    if(ready > 0 && FD_ISSET(STDIN_FILENO, &readfds)){
        read(STDIN_FILENO, &result, 1);
    }

    return result;
}

#endif

void die(const char *format, ...){
    char str[256];

    va_list args;
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);

    char *errStr = strerror(errno);

    printf("%s : (%u) %s\n", str, errno, errStr);

    getchar();
    exit(1);
}