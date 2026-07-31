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
    Print(ESC_ENABLE_CURSOR_BLINKING);
    Print(ESC_RESET_CURSOR_ATTRIBUTES);
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
    Print(ESC_DISABLE_CURSOR_BLINKING);
    Print(ESC_FORCE_STATIC_CURSOR);
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
    Print(ESC_MOVE_TO_AUX_BUFFER);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    FlushConsoleInputBuffer(hStdin);
    atexit(disableRawMode);
}

static inline LPSTR WIN32_FormatError(DWORD dwError){
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
                if(((Result.arr[i] >= 'a' && Result.arr[i] <= 'z') || 
                    (Result.arr[i] >= 'A' && Result.arr[i] <= 'Z')) && byteCount < 2) byteCount = 2; 
                if((Result.arr[i] <= '9' && Result.arr[i] >= '0' 
                    || Result.arr[i] == 'O') && byteCount < 4) byteCount = 4;
                if(Result.arr[i] == ';') byteCount = 6;
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

// TODO: Better error handling for these functions!!!

int64 fileWrite(char *destPath, char *string, uint32 size){
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

    return (int64)Feedback;
}

int64 fileRead(char *filePath, char *destBuffer, uint32 maxBufferSize){
    HANDLE hInFile = CreateFileA(filePath, GENERIC_READ, 0, NULL, OPEN_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, NULL);
    if(hInFile == INVALID_HANDLE_VALUE){
        //WIN32_ErrorExit("CreateFile failed for path: %s", filePath);
        return -1;
    }
    DWORD Feedback;
    if(!ReadFile(hInFile, destBuffer, maxBufferSize, &Feedback, NULL)){
        //WIN32_ErrorExit("Read file failed (requested %u, read %u bytes)", maxBufferSize, Feedback);
        return -1;
    }

    CloseHandle(hInFile);
    return Feedback;
}

int64 getFileSize(char *filePath){
    WIN32_FILE_ATTRIBUTE_DATA fi;
    if(!GetFileAttributesExA(filePath, GetFileExInfoStandard, &fi)){
        return -1;
    }
    return (int64)((uint64)fi.nFileSizeLow | ((uint64)fi.nFileSizeHigh << 32));
}

#elif defined LINUX

struct termios orig_termios;

void disableRawMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    Print(ESC_MOVE_TO_MAIN_BUFFER);
    Print(ESC_YESOVERFLOW);
}

void enterRawMode(){
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

void prepareConsole(){
    enterRawMode();
    Print(ESC_MOVE_TO_AUX_BUFFER);
    Print(ESC_NOOVERFLOW);
    atexit(disableRawMode);
}

character_input pollInput(){
    character_input Result = {0};

    fd_set readfds; 
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    timeout.tv_sec = TIMEOUT_MS / 1000;
    timeout.tv_usec = (TIMEOUT_MS - timeout.tv_sec * 1000) * 1000;

    int ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

    if(ready > 0 && FD_ISSET(STDIN_FILENO, &readfds)){
        
        if(read(STDIN_FILENO, Result.arr, 1) <= 0) die("Error reading stdin");

        int byteCount = charGetByteCount(Result.arr[0]);

        if(Result.arr[0] == '\x1b'){
            byteCount = 3;
        }
        int Feedback;
        for(int i = 1; i < byteCount; i++){
            if((Feedback = read(STDIN_FILENO, Result.arr + i, 1)) <= 0){
                die("Error reading stdin");
            }
            if(((Result.arr[i] >= 'a' && Result.arr[i] <= 'z') || 
                (Result.arr[i] >= 'A' && Result.arr[i] <= 'Z')) && byteCount < 2) byteCount = 2; 
            if((Result.arr[i] <= '9' && Result.arr[i] >= '0' 
                || Result.arr[i] == 'O') && byteCount < 4) byteCount = 4;
            if(Result.arr[i] == ';') byteCount = 6;
        }
        Result.byteCount = byteCount;
    }

    return Result;
}

uint32 writeOutput(char *src, uint32 size){
    return write(STDOUT_FILENO, src, size);
}

console_info getConsoleSystemInfo(){
    console_info Result;

    struct winsize ws;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
        die("Couldn't get terminal size");
    }else{
        Result.Cols = ws.ws_col;
        Result.Rows = ws.ws_row;
    }

    Result.CurrentTime = time(NULL);

    return Result;
}

int64 fileWrite(char *destPath, char *string, uint32 size){
    int fd = open(destPath, O_CREAT | O_WRONLY, 0644);

    if(fd < 0) return -1; //die("Error crating file %s", destPath);

    uint32 Feedback = write(fd, string, size);
    if(Feedback == 0) return -1; //die("Fatal error writing to fd %d", fd);
    close(fd);

    return Feedback;
}

int64 fileRead(char *filePath, char *destBuffer, uint32 maxBufferSize){
    int fd = open(filePath, O_RDONLY);
    if(fd < 0) return -1; //die("Error opening file %s", filePath);

    uint32 Feedback = read(fd, destBuffer, maxBufferSize);
    if(Feedback == 0) return -1; //die("Fatal error reading fd %d", fd);
    close(fd);
    return Feedback;
}

int64 getFileSize(char *filePath){
    struct stat st;
    if(stat(filePath, &st) == 1) return -1; //die("Error stating file %s", filePath);

    int64 Result = st.st_size;

    return Result;
}

#endif

static inline uint32 getInputOutputErrorCode(){
#if defined WINDOWS
    return GetLastError();
#elif defined LINUX
    return errno;
#endif
}

char *getInputOutputErrorString(){
#if defined WINDOWS
    return WIN32_FormatError((DWORD)getInputOutputErrorCode());
#elif defined LINUX
    return strerror(errno);
#endif
}

void die(const char *format, ...){
    char str[256];

    va_list args;
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);

    char *errStr = strerror(errno);

    printf(ESC_CLEAR_SCREEN "%s : (%u) %s\n", str, errno, errStr);

    getchar();
    exit(1);
}