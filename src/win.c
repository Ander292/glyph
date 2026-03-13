#include "win.h"

//-----GlobalVariables-----//
    HANDLE hStdin;
    HANDLE hStdout;
    HANDLE hStderr;
    HANDLE hHeap;
    HANDLE hFile;

    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;


//---WindowsLayer---//

    void GetConsoleSystemInfo(){
        GetConsoleScreenBufferInfo(hStdout, &(ScreenBufferInfo));
        Inf.ConsoleRows = ScreenBufferInfo.dwSize.Y - 1;
        Inf.ConsoleColumns = ScreenBufferInfo.dwSize.X - 1;

        Inf.CurrentTime = GetTickCount64();
    }

    void PrepareConsole(){
        hStdin = GetStdHandle(STD_INPUT_HANDLE);
        hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        hStderr = GetStdHandle(STD_ERROR_HANDLE);
        hHeap = GetProcessHeap();

        DWORD ConsoleMode;
        GetConsoleMode(hStdin, &ConsoleMode);

        //| ENABLE_MOUSE_INPUT
        ConsoleMode = ConsoleMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_OUTPUT) | (ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT);
        SetConsoleMode(hStdin, ConsoleMode);

        GetConsoleSystemInfo();

        //Inf.CursorX = 4;
        Inf.CursorX = 0;
        Inf.CursorY = 0;
        Inf.InsertMode = 0;
        
        Inf.RowArrayOrigin = CreateBufferArray(32);
        Inf.RowArray = CreateBufferArray(32);
        
        for(int i = 0; i < ScreenBufferInfo.dwSize.Y; i++) PrintChar("\n");
    }

    void DisableRawMode(){
        DWORD ConsoleMode;
        GetConsoleMode(hStdin, &ConsoleMode);
        ConsoleMode |= (ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_OUTPUT);
        SetConsoleMode(hStdin, ConsoleMode);
    }

    void ScrollScreen(){
        SMALL_RECT ScrollRect = {
            .Left = 0,
            .Top = 0,
            .Right = ScreenBufferInfo.dwSize.X,
            .Bottom = ScreenBufferInfo.dwSize.Y
        };
        COORD ScrollTarget = {
            .X = 0,
            .Y = (short) (0 - ScrollRect.Bottom)
        };
        CHAR_INFO CharInfo = {
            .Char.AsciiChar = ' ',
            .Attributes = ScreenBufferInfo.wAttributes
        };

        ScrollConsoleScreenBufferA(hStdout, &ScrollRect, NULL, ScrollTarget, &CharInfo);

        ScreenBufferInfo.dwCursorPosition.X = 0;
        ScreenBufferInfo.dwCursorPosition.Y = 0;

        SetConsoleCursorPosition(hStdout, ScreenBufferInfo.dwCursorPosition);
    }

    void ErrorExit(char *ErrorStr){
        ScrollScreen();
        if(hFile) CloseHandle(hFile);

        Print(ErrorStr);
        DisableRawMode();

        exit(1);
    }

    //---Cursor---//

        void DisplayConsoleCursor(){
            CONSOLE_CURSOR_INFO CursorInfo;

            GetConsoleCursorInfo(hStdout, &CursorInfo);
            CursorInfo.bVisible = TRUE;
            SetConsoleCursorInfo(hStdout, &CursorInfo);
        }

        void HideConsoleCursor(){
            CONSOLE_CURSOR_INFO CursorInfo;

            GetConsoleCursorInfo(hStdout, &CursorInfo);
            CursorInfo.bVisible = FALSE;
            SetConsoleCursorInfo(hStdout, &CursorInfo);
        }

    //---I/O---//

        void EditorOpen(char* fName){

            if(StringLength(fName) > 128) ErrorExit("File name too long (128 characters max)");
            
            hFile = CreateFileA(
                fName,
                GENERIC_READ | GENERIC_WRITE, 
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (hFile == INVALID_HANDLE_VALUE) {
                Print("Cannot open file, creating...\r\n");

                hFile = CreateFileA(
                    fName,
                    GENERIC_READ | GENERIC_WRITE, 
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_NEW,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );

                if(hFile == INVALID_HANDLE_VALUE){
                    DisableRawMode();
                    Print("Fatal error: Couldn't create file\r\n");
                    exit(1);
                }
            }

            StringCopy(FileName, fName);
            FileReadPortionS(hFile, 256, &(Inf.RowArrayOrigin));
            TranslateStringArray();
            
            CloseHandle(hFile);
            hFile = NULL;
            Inf.EditorDirty = 0;
        }

        uint8_t EditorSave(char *fName){

        hFile = CreateFileA(
                fName,
                GENERIC_READ | GENERIC_WRITE, 
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS, // Erases old file content
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

        StringBuffer TempBuffer = CreateBuffer(64);
        uint32_t LoopLimit = Inf.RowArray.NumberOfElements;
        for (uint32_t i = 0; i < LoopLimit; i++) {
            StringBuffer *temp = StringBufferGetElemenetAt(&(Inf.RowArray), i);
            uint32_t StringSize = StringLength(temp->Memory);

            AppendBuffer(&TempBuffer, temp->Memory);

            if(i < LoopLimit - 1) AppendBuffer(&TempBuffer, "\r\n");
        }

        DWORD BytesWritten;
        DWORD BytesToWrite = StringLength(TempBuffer.Memory) - 1;
        WriteFile(hFile, TempBuffer.Memory, BytesToWrite, &BytesWritten, 0);

        DeleteBuffer(&TempBuffer);
        CloseHandle(hFile);
        hFile = NULL;

        if(BytesWritten != BytesToWrite) return 1;
        Inf.EditorDirty = 0;
        return 0;
    }

        // I became like microsoft...
        char ReadCharacterEx(){
            int c;
            c = _getch();
            if(c == 0xE0 || c == 0){
                c = _getch();
                switch(c){
                    case 72: ArrowKeys = UP_ARROW; break;
                    case 80: ArrowKeys = DOWN_ARROW; break;
                    case 77: ArrowKeys = RIGHT_ARROW; break;
                    case 75: ArrowKeys = LEFT_ARROW; break;

                    case 141: ArrowKeys = CTRL_UP; break;
                    case 145: ArrowKeys = CTRL_DOWN; break;
                    case 116: ArrowKeys = CTRL_RIGHT; break;
                    case 115: ArrowKeys = CTRL_LEFT; break;

                    case 81: ArrowKeys = PAGE_DOWN; break;
                    case 73: ArrowKeys = PAGE_UP; break;

                    case 71: ArrowKeys = HOME_KEY; break;
                    case 79: ArrowKeys = END_KEY; break;

                    case 82: ArrowKeys = INSERT_KEY; break;
                    case 83: ArrowKeys = DELETE_KEY; break;

                    case 147: ArrowKeys = CTRL_DELETE; break;
                    default: ArrowKeys = 0; break;
                }
                return 0;
            }
            else {
                ArrowKeys = 0;
                return (char)c;
            }
        }

        char ReadCharacter(){
        char C;
        DWORD read;
        do{
            ReadFile(hStdin, &C, 1, &read, NULL);
        } while(read != 1);
        return C;
    }

