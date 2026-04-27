#include "main.h"

#if defined WINDOWS
//-----Windows-----//

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

    //| ENABLE_MOUSE_INPUT ; ENABLE_QUICK_EDIT_MODE
    OldConsoleMode = ConsoleMode;
    ConsoleMode = ConsoleMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_INPUT) | (ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT);
    SetConsoleMode(hStdin, ConsoleMode);

    GetConsoleSystemInfo();

    //Inf.CursorX = 4;
    Inf.StringMode = MODE_UTF16;

    if(Inf.StringMode == MODE_UTF8){
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
    else if(Inf.StringMode == MODE_UTF16){
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }

    Inf.CursorX = 0;
    Inf.CursorY = 0;
    Inf.InsertMode = 0;
    Inf.MouseEnabled = 0;

    Inf.RowArrayOrigin = CreateBufferArray(32);
    Inf.RowArray = CreateBufferArray(32);

    //for(int i = 0; i < ScreenBufferInfo.dwSize.Y; i++) PrintChar("\n");
    PrintA(MOVE_TO_AUX_BUFFER);
    //PrintA(ENABLE_MOUSE_TRACKING);
    PrintA(DISABLE_MOUSE_TRACKING);
}

void DisableRawMode(){
    DWORD ConsoleMode = OldConsoleMode;

    //GetConsoleMode(hStdin, &ConsoleMode);
    //ConsoleMode |= (ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_OUTPUT);

    SetConsoleMode(hStdin, ConsoleMode);

    PrintA(DISABLE_MOUSE_TRACKING);
}

void SwitchMouseMode(){
    if(Inf.MouseEnabled){
        PushEditorMessage(L"Mouse Disabled");
        PrintA(DISABLE_MOUSE_TRACKING);
        Inf.MouseEnabled = 0;
    }
    else{
        PushEditorMessage(L"Mouse Enabled");
        PrintA(ENABLE_MOUSE_TRACKING);
        Inf.MouseEnabled = 1;
    }
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
        .Char.UnicodeChar= L' ',
        .Attributes = ScreenBufferInfo.wAttributes
    };

    ScrollConsoleScreenBufferA(hStdout, &ScrollRect, NULL, ScrollTarget, &CharInfo);



    ScreenBufferInfo.dwCursorPosition.X = 0;
    ScreenBufferInfo.dwCursorPosition.Y = 0;

    SetConsoleCursorPosition(hStdout, ScreenBufferInfo.dwCursorPosition);
}

void ScrollScreenEx(){
    ResetCursorPossition();
    int ConY = Inf.ConsoleRows;

    char BufferA[16];
    wsprintfA(BufferA, ESC_SEQ "%dM", ConY);
    PrintA(BufferA);
    //SetCursorPossition(Inf.CursorX, Inf.CursorY);
}

void ErrorExit(wchar *ErrorStr){
    ScrollScreenEx();

    if(hFile) CloseHandle(hFile);
    PrintA(MOVE_TO_MAIN_BUFFER);
    Print(ErrorStr);
    DisableRawMode();

    exit(1);
}

//---Cursor---//

// TODO: Maybe these should be macros???

static inline void DisplayConsoleCursor(){
    PrintA(ESC_SHOW_CURSOR);
}

static inline void HideConsoleCursor(){
    PrintA(ESC_HIDE_CURSOR);
}

static inline void SetCursorPossition(int x, int y){
    char Buffer[16];
    int len = wsprintfA(Buffer, ESC_SEQ "%d;%dH", y, x);
    PrintAL(Buffer, len + 1);
}

static inline void ResetCursorPossition(){
    PrintA(ESC("0;0H"));
}


//---I/O---//

void EditorOpen(char *fNameANSI){
    wchar fName[128];
    //if(!MultiByteToWideChar(CP_UTF8, 0, fNameANSI, -1, fName, 128)){
    if(TranslateToUtf16(fName, 128, fNameANSI, -1)){
        ErrorExit(L"Fatal error while converting filename");
    }
    if(StringLength(fName) > 128) ErrorExit(L"File name too long (128 characters max)");
    
    hFile = CreateFileW(
        fName,
        GENERIC_READ | GENERIC_WRITE, 
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        Print(L"Cannot open file, creating...\r\n");

        hFile = CreateFileW(
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
            Print(L"Fatal error: Couldn't create file\r\n");
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

uint8_t EditorSave(wchar *fName){

    hFile = CreateFileW(
            fName,
            GENERIC_READ | GENERIC_WRITE, 
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS, // Erases old file content
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

    StringBufferA TempBuffer = CreateBufferA(64);
    uint32_t LoopLimit = Inf.RowArray.NumberOfElements;
    for (uint32_t i = 0; i < LoopLimit; i++) {
        StringBuffer *temp = StringBufferGetElemenetAt(&(Inf.RowArray), i);
        uint32_t StringSize = StringLength(temp->Memory);

        StringBufferA LoopConvertBuffer = TranslateToUtf8Ex(temp->Memory);

        AppendBufferA(&TempBuffer, LoopConvertBuffer.Memory);
        DeleteBufferA(&LoopConvertBuffer);

        if(i < LoopLimit - 1) AppendBufferA(&TempBuffer, "\r\n");
    }

    DWORD BytesWritten;
    DWORD BytesToWrite = StringLengthA(TempBuffer.Memory) - 1;
    WriteFile(hFile, TempBuffer.Memory, BytesToWrite, &BytesWritten, 0);

    DeleteBufferA(&TempBuffer);
    CloseHandle(hFile);
    hFile = NULL;

    if(BytesWritten != BytesToWrite) return 1;
    Inf.EditorDirty = 0;
    return 0;
}

wchar ReadCharacter(){
    INPUT_RECORD InpRec;
    DWORD InputFeedback = 0;
    wchar Result = 0;

    GetNumberOfConsoleInputEvents(hStdin, &InputFeedback);

    if(InputFeedback){
        RefreshInf.ToRender = TRUE;
        RefreshInf.ToFixCursor = TRUE;
        if(!ReadConsoleInputW(hStdin, &InpRec, 1, &InputFeedback)){
            PushEditorMessage((wchar *)"ConsoleInputError");
        }

        if(InpRec.EventType == KEY_EVENT){
            KEY_EVENT_RECORD KeyInfo = InpRec.Event.KeyEvent;
            if(KeyInfo.bKeyDown){
                switch(KeyInfo.wVirtualKeyCode){
                    case VK_UP: 
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)
                            ArrowKeys = CTRL_UP;
                        else ArrowKeys = UP_ARROW; 
                        break;
                    case VK_DOWN: 
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)    
                            ArrowKeys = CTRL_DOWN;
                        else ArrowKeys = DOWN_ARROW;    
                        break;
                    case VK_RIGHT:
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)
                            ArrowKeys = CTRL_RIGHT;
                        else ArrowKeys = RIGHT_ARROW; 
                        break;
                    case VK_LEFT: 
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)
                            ArrowKeys = CTRL_LEFT;
                        else ArrowKeys = LEFT_ARROW; 
                        break;

                    case VK_NEXT: ArrowKeys = PAGE_DOWN; break;
                    case VK_PRIOR: ArrowKeys = PAGE_UP; break;

                    case VK_HOME: ArrowKeys = HOME_KEY; break;
                    case VK_END: ArrowKeys = END_KEY; break;

                    case VK_RETURN: ArrowKeys = NEWLINE; break;
                    case VK_TAB: ArrowKeys = TAB; break;

                    case VK_INSERT: ArrowKeys = INSERT_KEY; break;
                    
                    case VK_DELETE: 
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)
                            ArrowKeys = CTRL_DELETE;
                        else ArrowKeys = DELETE_KEY;
                        break;
                    case VK_BACK: 
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)
                            ArrowKeys = CTRL_BACKSPACE;
                        else ArrowKeys = BACKSPACE;
                        break;
                    
                
                    case 'Q':
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)
                            ArrowKeys = CTRL_Q;
                        else goto default_jump;
                        break;
                    case 'C':
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)
                            ArrowKeys = CTRL_C;
                        else goto default_jump;
                        break;
                    case 'S':
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)
                            ArrowKeys = CTRL_S;
                        else goto default_jump;
                        break;
                    case 'D':
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)
                            ArrowKeys = CTRL_DELETE;
                        else goto default_jump;
                        break;
                    case 'W':
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED)
                            ArrowKeys = CTRL_W;
                        else goto default_jump;
                        break;
                    case 'M':
                        if(KeyInfo.dwControlKeyState & LEFT_CTRL_PRESSED) {
                            if(KeyInfo.wRepeatCount > 1) return 0;
                            ArrowKeys = CTRL_M;
                        }
                        else goto default_jump;
                        break;

                    default: 
                        default_jump:
                        ArrowKeys = 0;
                        Result = (wchar)KeyInfo.uChar.UnicodeChar;
                }
            }
        }
        else if(InpRec.EventType == MOUSE_EVENT){
            MOUSE_EVENT_RECORD mRecord = InpRec.Event.MouseEvent;
            if(mRecord.dwEventFlags == 0){
                int MouseX = mRecord.dwMousePosition.X;
                int MouseY = mRecord.dwMousePosition.Y;

                if(mRecord.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED){
                    PushEditorMessage(L"Left mouse clicked");
                    Inf.CursorX = MouseX - LINE_NUMBER_WIDTH;
                    Inf.CursorY = MouseY - 1;
                }
                else Sleep(TIMEOUT_MS);
            }
        }
        else if(InpRec.EventType == WINDOW_BUFFER_SIZE_EVENT){
            WINDOW_BUFFER_SIZE_RECORD bRecord = InpRec.Event.WindowBufferSizeEvent;
            PushEditorMessage(L"Window fiddled!");
        }
    }
    else {
        Sleep(TIMEOUT_MS);
        RefreshInf.ToRender = FALSE;
        RefreshInf.ToFixCursor = FALSE;
    }
    return Result;
}

#elif defined LINUX

#endif

//---Editor---//

void TranslateStringBuffer(StringBuffer *Destination, StringBuffer *Source){
    int TabCount = CharacterCount(Source->Memory, '\t');

    while(Destination->Length < Source->Length + TabCount)
        DoubleSize(Destination);

    ZeroBuffer(Destination);
    int Limit = StringLength(Source->Memory) - 1;

    int DestPos = 0;
    for(int SrcPos = 0; SrcPos < Limit; SrcPos++){
        if(Source->Memory[SrcPos] == '\t')
            for(int i = 0; i < TAB_SPACE_COUNT; i++) Destination->Memory[DestPos++] = ' ';
        else
            Destination->Memory[DestPos++] = Source->Memory[SrcPos];
    }

    Destination->Memory[DestPos] = '\0';
}

void TranslateStringArray(){
    while(Inf.RowArray.MaxNumberOfElements < Inf.RowArrayOrigin.MaxNumberOfElements){
        DoubleArrayCapacity(&(Inf.RowArray));
    }
    for(int i = 0; i <= Inf.RowArrayOrigin.NumberOfElements; i++){
        Inf.RowArray.NumberOfElements = i;


        StringBuffer *DisplayBuffer = StringBufferGetElemenetAt(&(Inf.RowArray), i);
        StringBuffer *OriginBuffer = StringBufferGetElemenetAt(&(Inf.RowArrayOrigin), i);
        if(DisplayBuffer==NULL || OriginBuffer == NULL)
            continue;

        TranslateStringBuffer(
            DisplayBuffer,
            OriginBuffer
        );
    }

    DeleteBufferArray(&(Inf.RowArrayOrigin));
}

void FixCursorPossitionEx(){
    if(Inf.CursorY < 0) Inf.CursorY = 0;
    if(Inf.CursorY > Inf.RowArray.NumberOfElements - 1) Inf.CursorY = Inf.RowArray.NumberOfElements - 1;

    StringBuffer *target = StringBufferGetElemenetAt(&(Inf.RowArray), Inf.CursorY);
    uint32_t LineLength = StringLength(target->Memory) - 1;
    
    if(Inf.CursorX < 0) Inf.CursorX = 0;
    if((uint32_t)Inf.CursorX > LineLength) Inf.CursorX = LineLength;
}


//---Header Formating---//

uint32_t FormatInfoString(char* StrOut){
    wchar StrFileName[64] = {0};
    ReturnFileName(FileName, StrFileName);
    char fName8[64];
    TranslateToUtf8(fName8, 64, StrFileName, -1);

    uint32_t LineCount = Inf.RowArray.NumberOfElements;
    uint32_t Line = Inf.CursorY + 1;
    uint32_t Column = Inf.CursorX + 1;
    wsprintfA(StrOut,
        "|| F: %s | LC: %4u | L: %4u | C: %4u ||",
        fName8, LineCount, Line, Column);
}

void PushEditorMessage(wchar *Str){
    StringCopy(DebugMessage.Message, Str);
    DebugMessage.TimeOfCreation = Inf.CurrentTime;
}

uint8_t SyncEditorMessage(){
    if(Inf.CurrentTime - DebugMessage.TimeOfCreation > EDITOR_MESSAGE_TIME * 1000){
        DebugMessage.TimeOfCreation = 0;
        DebugMessage.Message[0] = L'\0';
        StringConcat(DebugMessage.Message, L"---");
        return 1U;
    }
    return 0U;
}

void FormatHeader(){
    int X = Inf.ConsoleColumns;

    char StrMain[X*2+1];
    char StrInfo[128];
    char MessageBuffer[128];
    char StrSpaces[X*2]; StrSpaces[0] = (char)0;

    //StringBufferA TempMessageBuffer = TranslateToUtf8Ex(DebugMessage.Message);
    TranslateToUtf8(MessageBuffer, 128, DebugMessage.Message, -1);
    uint32_t MessageSize = StringLengthA(MessageBuffer);

    uint32_t InfoLength = FormatInfoString(StrInfo);
    int Space = X - 2 * FIRST_LINE_EMPTY_FIELDS;
    int MessageSpace = Space - InfoLength - 6;
    int FreeSpace = MessageSpace - MessageSize - 2;

    if(FreeSpace < 0){
        if(DebugMessage.TimeOfCreation == 0) {
            int Blanks = Space - InfoLength;
            if(Blanks < 0){
                for(int i = 0; i < Space; i++){
                    StringConcatA(StrMain, " ");
                }
                StringConcatA(StrMain, "\n");
            }
            else{
                for(int i = 0; i < Blanks; i++){
                    StringConcatA(StrSpaces, " ");
                }
                wsprintfA(StrMain,
                "%"TO_STR(FIRST_LINE_EMPTY_FIELDS)"s""%s%s%s%s%s\n",
                "\0", INVERTED_TEXT_COLOR, StrInfo, "\0", StrSpaces, RESET_TEXT_ATTRIBUTES);
            }
        }
        else{
            int Blanks = Space - MessageSize - 6;
            if(Blanks < 0){
                for(int i = 0; i < Space; i++){
                    StringConcatA(StrMain, " ");
                }
                StringConcatA(StrMain, "\n");
            }
            else {
                for(int i = 0; i < Blanks; i++){
                    StringConcatA(StrSpaces, " ");
                }
                wsprintfA(StrMain,
                "%"TO_STR(FIRST_LINE_EMPTY_FIELDS)"s""%s|: %s :|%s%s\n",
                "\0", INVERTED_TEXT_COLOR, MessageBuffer, StrSpaces, RESET_TEXT_ATTRIBUTES);
            }
        }
    }
    else{
        for(int i = 0; i < FreeSpace; i++){
            StringConcatA(StrSpaces, " ");
        }
        wsprintfA(StrMain,
            "%"TO_STR(FIRST_LINE_EMPTY_FIELDS)"s""%s""%s""%4s""|: %s :|%s%s\n",
            "\0", INVERTED_TEXT_COLOR, StrInfo, "\0", MessageBuffer, StrSpaces, RESET_TEXT_ATTRIBUTES);
    }

    PrintToBufferA(&Buffer, StrMain);
}


//---Main---//

void DrawRows(){
    char Str[64];

    if(Inf.CursorX > (int)(Inf.ConsoleColumns - 4 + Inf.ColumnOffset))
        Inf.ColumnOffset = Inf.CursorX - Inf.ConsoleColumns + 4;
    if(Inf.CursorY > (int)(Inf.ConsoleRows + Inf.RowOffset - 1))
        Inf.RowOffset = Inf.CursorY - Inf.ConsoleRows + 1;
    
    if(Inf.CursorX < (int)(Inf.ColumnOffset))
        Inf.ColumnOffset = Inf.CursorX;
    if(Inf.CursorY < (int)(Inf.RowOffset))
        Inf.RowOffset = Inf.CursorY;

    for(int i = 0; (i + Inf.RowOffset) < Inf.RowArray.NumberOfElements && i < (Inf.ConsoleRows); i++) {
        int RowNumber = i + Inf.RowOffset;

        UintToStringA((RowNumber + 1) % 1000, Str, 3);

        PrintToBufferA(&Buffer, Str);
        PrintToBufferA(&Buffer, "|");

        StringBuffer *temp = StringBufferGetElemenetAt(&(Inf.RowArray), RowNumber);
        int StringSize = StringLength(temp->Memory) - 1;

        StringBufferA TempBuffer = TranslateToUtf8Ex(temp->Memory);

        if(StringSize < Inf.ColumnOffset)
            AppendBufferExA(&Buffer, "<--", 3, 0);
        else
            AppendBufferExA(&Buffer, TempBuffer.Memory, (Inf.ConsoleColumns - 3), Inf.ColumnOffset);

        DeleteBufferA(&TempBuffer);

        if(i < (Inf.ConsoleRows - 1)) PrintToBufferA(&Buffer, "\r\n");
    }
}

void RefreshScreen(){
    HideConsoleCursor();
    ResetCursorPossition();

    DeleteBufferA(&Buffer);
    Buffer = CreateBufferA(64);

    SyncEditorMessage();
    FormatHeader();
    DrawRows();
    
    ScrollScreenEx();
    PrintA(Buffer.Memory);

    SetCursorPossition(Inf.CursorX + LINE_NUMBER_WIDTH - Inf.ColumnOffset + 1, Inf.CursorY + 2 - Inf.RowOffset);
    DisplayConsoleCursor();
}

void InsertCharacter(wchar C){
    if(C == 0) return;

    Inf.EditorDirty = 1;
    int CurrentColumn = Inf.CursorX;
    int CurrentLine = Inf.CursorY;

    StringBuffer *target = StringBufferGetElemenetAt(&(Inf.RowArray), CurrentLine);
    int CurrentLineLength = StringLength(target->Memory);

    int CurrentBufferSize = target->Length;

    if(CurrentLineLength + 1 > CurrentBufferSize) {
        DoubleSize(target);

        target = StringBufferGetElemenetAt(&(Inf.RowArray), CurrentLine);

        ZeroBufferEx(target, CurrentBufferSize);

        PushEditorMessage(L"Doubled buffer size!");
    }

    if(!Inf.InsertMode) {
        int StringSize = StringLength(target->Memory);
        while(StringSize + 2 > target->Length) DoubleSize(target);
        StringShiftRight(target->Memory, CurrentColumn, 0);
    }
        
    *(target->Memory + CurrentColumn) = C;
    Inf.CursorX++;
}

wchar ProcessKeypress(){
    wchar C = ReadCharacter();

    StringBuffer *target = StringBufferGetElemenetAt(
        &(Inf.RowArray),
        Inf.CursorY
    );

    switch(ArrowKeys){
        case UP_ARROW:{
                Inf.CursorY--;
            ArrowKeys = 0;
        } return 0;
        case DOWN_ARROW:{
                Inf.CursorY++;
            ArrowKeys = 0;
        } return 0;
        case RIGHT_ARROW:{
                Inf.CursorX++;
            ArrowKeys = 0;
        } return 0;

        case LEFT_ARROW:{
                Inf.CursorX--;
            ArrowKeys = 0;
        } return 0;
        case CTRL_RIGHT:{
            int n;
            if(*(target->Memory + Inf.CursorX) == ' ')
                n = CountForwardToWordEx(target->Memory, Inf.CursorX);
            else
                n = CountForwardToBlankEx(target->Memory, Inf.CursorX);
            Inf.CursorX = Inf.CursorX + n;
            ArrowKeys = 0;
        } return 0;
        case CTRL_LEFT:{
            if(Inf.CursorX == 0) return 0;
            int n;
            if(*(target->Memory + Inf.CursorX - 1) == ' ')
                n = CountBackToWordEx(target->Memory, Inf.CursorX);
            else
                n = CountBackToBlankEx(target->Memory, Inf.CursorX);

            Inf.CursorX = Inf.CursorX - n;
            ArrowKeys = 0;
        } return 0;
        /*
            PosOnScreen:
                CursorY - RowOffset < 0             --> CursorY++; (down move)
                CursorY - RowOffset > ConsoleRows   --> CursorY--; (up move)
        
        */
        case CTRL_UP:{
            ArrowKeys = 0;
            if(Inf.RowOffset > 0){
                Inf.RowOffset--;
                // Will move the cursor up if it cannot scroll
                if(Inf.CursorY - Inf.RowOffset == Inf.ConsoleRows)
                    Inf.CursorY--;
            } 
        } return 0;
        case CTRL_DOWN:{
            ArrowKeys = 0;
            if(Inf.RowOffset < Inf.RowArray.NumberOfElements){
                Inf.RowOffset++;
                // Will move the cursor down if it cannot scroll
                if(Inf.CursorY + 1 - Inf.RowOffset == 0) 
                    Inf.CursorY++;
            }
        } return 0;
        case PAGE_DOWN:{
            Inf.CursorY += Inf.ConsoleRows;
            ArrowKeys = 0;
        } return 0;
        case PAGE_UP:{
            Inf.CursorY -= Inf.ConsoleRows;
            ArrowKeys = 0;
        } return 0;
        case HOME_KEY:{
            Inf.CursorX = 0;
            ArrowKeys = 0;
        } return 0;
        case END_KEY:{
            int LineNumber = Inf.CursorY;
            int LineSize = StringLength(target->Memory) - 1;
            
            Inf.CursorX = LineSize;
            ArrowKeys = 0;
        } return 0;
        case DELETE_KEY:{
            Inf.EditorDirty = 1;
            ArrowKeys = 0;

            if(
                (Inf.CursorX == StringLength(target->Memory) - 1)
                && Inf.CursorY < Inf.RowArray.NumberOfElements - 1
            ){
                StringBuffer *NextLine = StringBufferGetElemenetAt(&(Inf.RowArray), Inf.CursorY);
                if(NextLine == NULL) return 0;

                StringBuffer temp = CreateBuffer(64);
                while(NextLine->Length > temp.Length) DoubleSize(&temp);
                RemoveLineEx(&(Inf.RowArray), Inf.CursorY + 1, temp.Memory);
                Inf.RowArray.NumberOfElements--;

                int FullStrLength = StringLength(target->Memory) + StringLength(temp.Memory) - 1;

                while(FullStrLength > target->Length) DoubleSize(target);

                StringConcat(target->Memory, temp.Memory);
                DeleteBuffer(&temp);

                return 0;
            }

            StringShiftLeft(target->Memory, Inf.CursorX, 0);
            

        } return 0;

        case CTRL_DELETE:{
            Inf.EditorDirty = 1;
            uint32_t n;
            if(*(target->Memory + Inf.CursorX) == L' ')
                n = CountForwardToWordEx(target->Memory, Inf.CursorX);
            else if(*(target->Memory + Inf.CursorX) == L'\0')
                n = 0;
            else
                n = CountForwardToBlankEx(target->Memory, Inf.CursorX);
        
            int counter = n;
            while(1) {
                if(counter == 0) break;
                counter--;
                StringShiftLeft(target->Memory, Inf.CursorX, 0);
            }
            ArrowKeys = 0;
        } return 0;
        case CTRL_Q:
        {
            ArrowKeys = 0;
            if(Inf.EditorDirty) {
                PushEditorMessage(L"Exit without saving? (Yes, No, Save)");
                RefreshScreen();
                
                repeat:
                wchar C = ReadCharacter();
                ArrowKeys = 0;
                if(C == L's' || C == L'S') EditorSave(FileName);
                else if(C == L'n' || C == L'N') {
                    DebugMessage.TimeOfCreation = 0;
                    DebugMessage.Message[0] = L'\0';
                    return 0;
                }
                else if(C == L'y' || C == L'Y');
                else{
                    Sleep(TIMEOUT_MS);
                    goto repeat;
                }
            }
            ResetCursorPossition;
            Running = 0;
        } return 0;

        case CTRL_C:
        {  
            ArrowKeys = 0;
            wchar Str[64];
            UintToString(Inf.CursorX, Str, 0);
            PushEditorMessage(Str);
        } return 0;
        case CTRL_S:
        {
            ArrowKeys = 0;
            uint8_t Failed = EditorSave(FileName);
            if(Failed)
                PushEditorMessage(L"Saving failed!");
            else
                PushEditorMessage(L"File saved");
        } return 0;

        case CTRL_BACKSPACE:
        {
            ArrowKeys = 0;
            Inf.EditorDirty = 1;
            if(Inf.CursorX == 0) return 0;

            int n;
            if(*(target->Memory + Inf.CursorX - 1) == L' ')
                n = CountBackToWordEx(target->Memory, Inf.CursorX);
            else
                n = CountBackToBlankEx(target->Memory, Inf.CursorX);
            
            int counter = n;
            int OldCursor = Inf.CursorX;

            while(1){
                if(counter == 0) break;
                counter--;
                StringShiftLeft(target->Memory, Inf.CursorX - 1, 0);
                Inf.CursorX--;
            }

        } return 0;
        case BACKSPACE: // Backspace
        {
            ArrowKeys = 0;
            Inf.EditorDirty = 1;
            if((int)Inf.CursorX == 0) {
                if((Inf.CursorY == 0 && Inf.RowOffset == 0)) return 0;

                StringBuffer temp = CreateBuffer(64);
                while(target->Length > temp.Length) DoubleSize(&temp);
                RemoveLineEx(&(Inf.RowArray), Inf.CursorY, temp.Memory);
                Inf.RowArray.NumberOfElements--;

                Inf.CursorY--;
                StringBuffer *NewLine = StringBufferGetElemenetAt(&(Inf.RowArray), Inf.CursorY);
                int StrLength = StringLength(NewLine->Memory) - 1;

                int FullStrLength = StringLength(temp.Memory) + StrLength;

                while(FullStrLength > NewLine->Length) DoubleSize(NewLine);

                StringConcat(NewLine->Memory, temp.Memory);
                DeleteBuffer(&temp);
                
                Inf.CursorX = StrLength;

                return 0;
            }

            if(Inf.InsertMode == 1) { // Unsuported
                target->Memory[Inf.CursorX - 1] = L'\0';

                Inf.CursorX--;
            }
            else {
                StringShiftLeft(target->Memory, Inf.CursorX-1, 0);
                Inf.CursorX--;
            }
        } return 0;

        case NEWLINE:{
            ArrowKeys = 0;
            Inf.EditorDirty = 1;
            InsertLine(&(Inf.RowArray), Inf.CursorY + 1, target->Memory + Inf.CursorX);
            target = StringBufferGetElemenetAt(
                &(Inf.RowArray), 
                Inf.CursorY
            );
            target->Memory[Inf.CursorX] = L'\0';
            Inf.CursorY++;
            Inf.CursorX = 0;
        } return 0;

        case TAB:{
            ArrowKeys = 0;
            InsertCharacter(L' ');
            while((Inf.CursorX) % TAB_SPACE_COUNT != 0) {
                InsertCharacter(L' ');

            }
        } return 0;
        
        case CTRL_M:{
#if 0
            SwitchMouseMode();
#endif
        } return 0;
    }
    return C;
}

int main(int argc, char* argv[]){

    PrepareConsole();
    ScrollScreenEx();

    if(argc == 2)
        EditorOpen(argv[1]);
    else{
        PrintA(MOVE_TO_MAIN_BUFFER);
        PrintA("Error: You must provide an argument!\n");
        return 1;
    }

    wchar C;

    PushEditorMessage(L"Ctrl+q to quit");
    RefreshInf.ToRender = TRUE;

    while(Running){
        GetConsoleSystemInfo();

        if(RefreshInf.ToRender) RefreshScreen();
        RefreshInf.ToRender = FALSE;

        C = ProcessKeypress();
        InsertCharacter(C);
        
        if(RefreshInf.ToFixCursor){
            HideConsoleCursor();
            FixCursorPossitionEx();
            DisplayConsoleCursor();
        }
        RefreshInf.ToFixCursor = FALSE;
        //Sleep(TIMEOUT_MS);
    }
    ScrollScreenEx();
    PrintA(MOVE_TO_MAIN_BUFFER);
    DisableRawMode();

    return 0;
}