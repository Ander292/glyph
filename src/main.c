#include "main.h"

//---Setup---//

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

    Inf.CursorX = 4;
    Inf.CursorY = 1;
    Inf.InsertMode = 0;
    
    Inf.RowArrayOrigin = CreateBufferArray(32);
    Inf.RowArrayDisplay = CreateBufferArray(32);
    
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

void TranslateStringBuffer(StringBuffer *Destination, StringBuffer *Source){
    uint32_t TabCount = CharacterCount(Source->Memory, '\t');

    while(Destination->Length < Source->Length + TabCount)
        DoubleSize(Destination);

    ZeroBuffer(Destination);
    uint32_t Limit = StringLength(Source->Memory) - 1;

    uint32_t DestPos = 0;
    for(uint32_t SrcPos = 0; SrcPos < Limit; SrcPos++){
        if(Source->Memory[SrcPos] == '\t')
            for(uint32_t i = 0; i < TAB_SPACE_COUNT; i++) Destination->Memory[DestPos++] = ' ';
        else
            Destination->Memory[DestPos++] = Source->Memory[SrcPos];
    }

    Destination->Memory[DestPos] = '\0';
}

void TranslateStringArray(){
    while(Inf.RowArrayDisplay.MaxNumberOfElements < Inf.RowArrayOrigin.MaxNumberOfElements){
        DoubleArrayCapacity(&(Inf.RowArrayDisplay));
    }
    for(uint32_t i = 0; i <= Inf.RowArrayOrigin.NumberOfElements; i++){
        Inf.RowArrayDisplay.NumberOfElements = i;

        StringBuffer *DisplayBuffer = StringBufferGetElemenetAt(&(Inf.RowArrayDisplay), i);
        StringBuffer *OriginBuffer = StringBufferGetElemenetAt(&(Inf.RowArrayOrigin), i);
        if(DisplayBuffer==NULL || OriginBuffer == NULL)
            continue;

        TranslateStringBuffer(
            DisplayBuffer,
            OriginBuffer
        );
    }
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

void FixCursorPossition(){
    // TODO: Fix this shit
    // Down overflow //

    int CompValue = SmallerInteger(Inf.ConsoleRows, (int)(Inf.RowArrayDisplay.NumberOfElements));
    if(Inf.CursorY > CompValue){
        int Offset = Inf.CursorY - CompValue;
        Inf.CursorY -= Offset;

        Inf.RowOffset += (uint32_t)Offset;
        if(Inf.RowOffset + CompValue > (uint32_t)Inf.RowArrayDisplay.NumberOfElements){
            Inf.RowOffset = Inf.RowArrayDisplay.NumberOfElements - (uint32_t)(Inf.ConsoleRows);
            if((int)(Inf.RowOffset) < 0) Inf.RowOffset = 0;
        }
    }

    // Up overflow //
    else if(Inf.CursorY < 1){
        //int Offset = -1 * Inf.CursorY + 1;
        int Offset = 1 - Inf.CursorY;
        Inf.CursorY = 1;
        //Inf.CursorY += Offset;

        if (Inf.RowOffset >= (uint32_t)Offset)
            Inf.RowOffset -= Offset;
        else
            Inf.RowOffset = 0;
    }

    // Right Overflow //
    StringBuffer *temp = StringBufferGetElemenetAt(&(Inf.RowArrayDisplay), Inf.CursorY - 1 + Inf.RowOffset);
    uint32_t CurrentLength = StringLength(temp->Memory) + 3;
    uint32_t CompValueU = SmallerUnsigned((uint32_t)(Inf.ConsoleColumns), CurrentLength);
    uint32_t CompValueReverse = LargerUnsigned((uint32_t)(Inf.ConsoleColumns), CurrentLength);

    if((uint32_t)(Inf.CursorX) > CompValueU){

        uint32_t Offset = AbsoluteUnsigned((uint32_t)(Inf.CursorX) - CompValueU);
        Inf.CursorX = CompValueU;
        
        if(CompValueU == (uint32_t)(Inf.ConsoleColumns)){
            Inf.ColumnOffset += Offset;

            if(Inf.ColumnOffset > CurrentLength - (uint32_t)(Inf.ConsoleColumns))
                Inf.ColumnOffset = (CurrentLength - (uint32_t)(Inf.ConsoleColumns));
        }
        
        else{
            if(Inf.ColumnOffset > CurrentLength - 4){
                Inf.ColumnOffset = CurrentLength - 4;
            }
        }
    }

    // Left Overflow //
    if(Inf.CursorX < 4) {
        uint32_t Offset = 4 - (uint32_t)(Inf.CursorX);
        Inf.CursorX = 4;

        Inf.ColumnOffset -= Offset;
        if((int)(Inf.ColumnOffset) < 0) Inf.ColumnOffset = 0;
    }

    // End and Home fix //
    if(ArrowKeys == END_KEY) {
        if((int)CurrentLength > Inf.ConsoleColumns){
            Inf.ColumnOffset = CurrentLength - Inf.ConsoleColumns;
            Inf.CursorX = Inf.ConsoleColumns;
        }
        else{
            Inf.CursorX = CurrentLength;
            Inf.ColumnOffset = 0;
        }
    }
    else if(ArrowKeys == HOME_KEY) {
        Inf.CursorX = 4;
        Inf.ColumnOffset = 0;
    }

    // RowOffset fix //

    if(Inf.RowOffset + Inf.CursorY > Inf.RowArrayDisplay.NumberOfElements) 
        Inf.RowOffset = Inf.RowArrayDisplay.NumberOfElements - Inf.CursorY;
    
}

void FixCursorPossitionEx(){

    // Vertical Overflow //
    uint32_t TotalLines = Inf.RowArrayDisplay.NumberOfElements;
    int DisplayedRows = SmallerInteger(Inf.ConsoleRows, (int)TotalLines);

    while(Inf.CursorY < 1){
        Inf.CursorY++;
        if(Inf.RowOffset != 0)
            Inf.RowOffset--;
    }
    while(Inf.CursorY > DisplayedRows) {
        Inf.CursorY--;
        if(Inf.RowOffset + Inf.CursorY < TotalLines)
            Inf.RowOffset++;
    } 

    // Horizontal Overflow //
    uint32_t CurrentLine = Inf.CursorY + Inf.RowOffset - 1;
    StringBuffer *line = StringBufferGetElemenetAt(&(Inf.RowArrayDisplay), CurrentLine);
    uint32_t CurrentLineLength = StringLength(line->Memory);
    
    uint32_t TotalColumns = CurrentLineLength + 3; // +3 not +4 because the StringLength counts the termination char too

    uint32_t minCursor = LINE_NUMBER_WIDTH;
    uint32_t maxCursor = SmallerUnsigned((uint32_t)Inf.ConsoleColumns, TotalColumns - Inf.ColumnOffset);
    
    while((uint32_t)(Inf.CursorX) < minCursor){
        Inf.CursorX++;
        if(Inf.ColumnOffset != 0)
            Inf.ColumnOffset--;
    }

    if((uint32_t)(Inf.ConsoleColumns) < TotalColumns - Inf.ColumnOffset){
        while((uint32_t)(Inf.CursorX) > maxCursor) {
            Inf.CursorX--;
            Inf.ColumnOffset++;
        }
    }
    else{
        /*if((uint32_t)(Inf.CursorX) > maxCursor){
            Inf.CursorX = TotalColumns;
            Inf.ColumnOffset = 0;
        }*/
        if((uint32_t)(Inf.CursorX) > maxCursor) {
            Inf.CursorX = maxCursor;
            if(Inf.ColumnOffset) Inf.CursorX++; // maxCursor is sometimes for 1 smaller than it should be
        }
    }
    uint32_t maxOffset = (TotalColumns > (uint32_t)Inf.ConsoleColumns) ? TotalColumns - Inf.ConsoleColumns : 0;
    if (Inf.ColumnOffset > maxOffset) Inf.ColumnOffset = maxOffset;
    if ((int)Inf.ColumnOffset < 0) Inf.ColumnOffset = 0;
}

//---File I/O---//

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
        //DisableRawMode();
        Print("Cannot open file, creating...\r\n");
        //exit(1);

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
    uint32_t LoopLimit = Inf.RowArrayDisplay.NumberOfElements;
    for (uint32_t i = 0; i < LoopLimit; i++) {
        StringBuffer *temp = StringBufferGetElemenetAt(&(Inf.RowArrayDisplay), i);
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

//---Header Formating---//

uint32_t FormatDebugInfoText(char *StrOut){
    char StrAux[8] = {0};
    char StrMain[128] = {0};

    //StringConcat(StrMain, INVERTED_TEXT_COLOR);
    StringConcat(StrMain, "||X:");

    UintToString(Inf.CursorX, StrAux, 3);
    StringConcat(StrMain, StrAux);

    StringConcat(StrMain, "|Y:");

    UintToString(Inf.CursorY, StrAux, 3);
    StringConcat(StrMain, StrAux);

    StringConcat(StrMain, "|RO:");

    UintToString(Inf.RowOffset, StrAux, 3);
    StringConcat(StrMain, StrAux);

    StringConcat(StrMain, "|CO:");

    UintToString(Inf.ColumnOffset, StrAux, 3);
    StringConcat(StrMain, StrAux);

    StringConcat(StrMain, "||");
    //StringConcat(StrMain, RESET_TEXT_ATTRIBUTES);
    
    StringCopy(StrOut, StrMain);

    return StringLength(StrMain);
}

uint32_t FormatInfoString(char *StrOut){
    char StrAux[8] = {0};
    char StrMain[128] = {0};

    //StringConcat(StrMain, INVERTED_TEXT_COLOR);

    //AddCharacters(StrMain, ' ', 3);

    StringConcat(StrMain, "|| F: ");

    char StrFileName[64];
    ReturnFileName(FileName, StrFileName);

    StringConcat(StrMain, StrFileName);
    
    StringConcat(StrMain, " | ");

    StringConcat(StrMain, "LC: ");

    uint32_t LineCount = Inf.RowArrayDisplay.NumberOfElements;
    UintToString(LineCount, StrAux, 3);
    StringConcat(StrMain, StrAux);

    StringConcat(StrMain, " | L:");

    UintToString(Inf.CursorY + Inf.RowOffset, StrAux, 3);
    StringConcat(StrMain, StrAux);

    StringConcat(StrMain, " | C:");

    UintToString(Inf.CursorX + Inf.ColumnOffset - 3, StrAux, 3);
    //uint32_t RowMaxSize = StringBufferGetElemenetAt(&(Inf.RowArrayDisplay), Inf.CursorY + Inf.RowOffset - 1)->Length;

    //UintToString(RowMaxSize, StrAux, 3);
    StringConcat(StrMain, StrAux);

    StringConcat(StrMain, " ||");

    //StringConcat(StrMain, RESET_TEXT_ATTRIBUTES);
    
    StringCopy(StrOut, StrMain);
    return StringLength(StrMain);
}

void PushEditorMessage(char *Str){
    //uint64_t CurrentTime = GetTickCount64();
    //uint32_t InputStringSize = StringLength(Str);
    //StringCopy(Str, "|:");
    StringCopy(DebugMessage.Message, Str);
    //StringConcat(Str, ":|");
    DebugMessage.TimeOfCreation = Inf.CurrentTime;
}

uint8_t SyncEditorMessage(){
    if(Inf.CurrentTime - DebugMessage.TimeOfCreation > EDITOR_MESSAGE_TIME * 1000){
        DebugMessage.TimeOfCreation = 0;
        DebugMessage.Message[0] = '\0';
        return 1U;
    }
    return 0U;
}

void FormatHeader(){
    //int X = ScreenBufferInfo.dwSize.X;
    int X = Inf.ConsoleColumns;

    char StrMain[256] = {0};
    char StrDebug[128] = {0};
    char StrDebug2[128] = {0};
    char StrEditor[128] = {0};

    uint32_t EditorMessageLength = StringLength(DebugMessage.Message);
    uint32_t ReturnStringLength = FormatInfoString(StrDebug);

    uint32_t DebugStringLength = FormatDebugInfoText(StrDebug2);

    uint32_t Limit = (X - ReturnStringLength) / 2 - FIRST_LINE_EMPTY_FIELDS;

    AddCharacters(StrMain, ' ', FIRST_LINE_EMPTY_FIELDS - 1);
    StringConcat(StrMain, INVERTED_TEXT_COLOR);
    CharConcat(StrMain, '|');

    

    if(EditorMessageLength <= Limit) 
        if(!EditorMessageLength){
            AddCharacters(StrMain, ' ', Limit);
        }
        else{
            uint32_t SmallLimit = (Limit - EditorMessageLength) / 2;
            
            AddCharacters(StrMain, ' ', SmallLimit);
            StringConcat(StrMain, DebugMessage.Message);
            AddCharacters(StrMain, ' ', SmallLimit);
        }

    StringConcat(StrMain, StrDebug);

    #ifdef DEBUG_INFO
        uint32_t SmallLimit = (Limit - DebugStringLength) / 2;

        AddCharacters(StrMain, ' ', SmallLimit);
        StringConcat(StrMain, StrDebug2);
        AddCharacters(StrMain, ' ', SmallLimit);
    #else
        AddCharacters(StrMain, ' ', Limit);
    #endif
    

    CharConcat(StrMain, '|');
    StringConcat(StrMain, RESET_TEXT_ATTRIBUTES);
    AddCharacters(StrMain, ' ', FIRST_LINE_EMPTY_FIELDS - 1);
    

    StringConcat(StrMain, "\n");

    PrintToBuffer(&Buffer, StrMain);
}

void FormatHeaderEx(){
    int X = Inf.ConsoleColumns;
    // FIRST_LINE_EMPTY_FIELDS + '|' --> 7


    char StrMain[256] = {0};
    char StrInfo[256] = {0};

    uint32_t EditorMessageLength = StringLength(DebugMessage.Message);
    uint32_t ReturnStringLength = FormatInfoString(StrInfo);
    int MessageSpace = X - 2 * FIRST_LINE_EMPTY_FIELDS - 8 - ReturnStringLength;

    AddCharacters(StrMain, ' ', FIRST_LINE_EMPTY_FIELDS);
    StringConcat(StrMain, INVERTED_TEXT_COLOR);
    // CharConcat(StrMain, '|');

    // AddCharacters(StrMain, ' ', 2);
    StringConcat(StrMain, StrInfo);
    AddCharacters(StrMain, ' ', 4);

    
    StringConcat(StrMain, DebugMessage.Message);

    if((uint32_t)MessageSpace > EditorMessageLength){
        MessageSpace -= EditorMessageLength;
        AddCharacters(StrMain, ' ', (uint32_t)MessageSpace);
    }

    CharConcat(StrMain, '|');
    StringConcat(StrMain, RESET_TEXT_ATTRIBUTES);
    AddCharacters(StrMain, ' ', FIRST_LINE_EMPTY_FIELDS);

    
    StringConcat(StrMain, "\n");
    PrintToBuffer(&Buffer, StrMain);
}

//---Main---//

void DrawRows(){
    char Str[64];
    for (uint32_t i = 0; (i + Inf.RowOffset) < Inf.RowArrayDisplay.NumberOfElements && i < (uint32_t)(Inf.ConsoleRows); i++) {
        uint32_t RowNumber = i + Inf.RowOffset;
        uint32_t DisplayNumber;

        // UintToString((RowNumber + 1) % 1000 + (RowNumber + 1) / 1000, Str, 3);
        UintToString((RowNumber + 1) % 1000, Str, 3);

        PrintToBuffer(&Buffer, Str);
        PrintToBuffer(&Buffer, "|");

        StringBuffer *temp = StringBufferGetElemenetAt(&(Inf.RowArrayDisplay), RowNumber);
        uint32_t StringSize = StringLength(temp->Memory);

        if(StringSize < Inf.ColumnOffset)
            AppendBufferEx(&Buffer, " ", 2, 0);
        else
            AppendBufferEx(&Buffer, temp->Memory, (Inf.ConsoleColumns - 3), Inf.ColumnOffset);

        if(i < (uint32_t)(Inf.ConsoleRows - 1)) PrintToBuffer(&Buffer, "\r\n");
    }
}

void RefreshScreen(){
    HideConsoleCursor();
    ResetCursorPossition;
    ScrollScreen();

    DeleteBuffer(&Buffer);
    Buffer = CreateBuffer(64);

    SyncEditorMessage();
    FormatHeaderEx();
    DrawRows();
    //pusi ga klintone <3
    //DrawFooter();

    Print(Buffer.Memory);

    SetCursorPossition(Inf.CursorX, Inf.CursorY);
    DisplayConsoleCursor();
}

// Replaces the next character in line with char C (behaves like Insert mode)
void InsertCharacter(char C){
    if(C == 0) return;
    Inf.EditorDirty = 1;
    uint32_t CurrentColumn = (uint32_t)(Inf.CursorX) + Inf.ColumnOffset - 4;
    uint32_t CurrentLine = (uint32_t)(Inf.CursorY) + Inf.RowOffset - 1;

    StringBuffer *target = StringBufferGetElemenetAt(&(Inf.RowArrayDisplay), CurrentLine);
    uint32_t CurrentLineLength = StringLength(target->Memory);

    uint32_t CurrentBufferSize = target->Length;

    if(CurrentLineLength + 1 > CurrentBufferSize) {
        DoubleSize(target);
        target = StringBufferGetElemenetAt(&(Inf.RowArrayDisplay), CurrentLine);

        ZeroBufferEx(target, CurrentBufferSize);

        PushEditorMessage("Doubled buffer size!");
    }

    if(!Inf.InsertMode) {
        uint32_t StringSize = StringLength(target->Memory);
        while(StringSize + 2 > target->Length) DoubleSize(target);
        StringShiftRight(target->Memory, CurrentColumn, 0);
    }
        
    *(target->Memory + CurrentColumn) = C;
    Inf.CursorX++;
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
    static int i = 0;
    char Str[64];
    do{
        i++;
        UintToString(i, Str, 0);
        ReadFile(hStdin, &C, 1, &read, NULL);
    } while(read != 1);
    return C;
}

char ProcessKeypress(){
    char c = ReadCharacterEx();

    uint32_t LinePossition = Inf.CursorX + Inf.ColumnOffset - LINE_NUMBER_WIDTH;
    StringBuffer *target = StringBufferGetElemenetAt(
        &(Inf.RowArrayDisplay),
        Inf.CursorY + Inf.RowOffset - 1
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
            uint32_t n;
            if(*(target->Memory + LinePossition) == ' ')
                n = CountForwardToWord(target->Memory, LinePossition);
            else
                n = CountForwardToBlank(target->Memory, LinePossition);
            Inf.CursorX = n + LINE_NUMBER_WIDTH;
            ArrowKeys = 0;
        } return 0;
        case CTRL_LEFT:{
            if(LinePossition == 0) return 0;
            uint32_t n;
            if(*(target->Memory + LinePossition - 1) == ' ')
                n = CountBackToWord(target->Memory, LinePossition);
            else
                n = CountBackToBlank(target->Memory, LinePossition);
            Inf.CursorX = n + LINE_NUMBER_WIDTH;
            ArrowKeys = 0;
        } return 0;
        case CTRL_UP:{
            if(Inf.RowOffset > 0)
                Inf.RowOffset--;
        } return 0;
        case CTRL_DOWN:{
            if(Inf.RowOffset < Inf.RowArrayDisplay.NumberOfElements)
                Inf.RowOffset++;
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
            Inf.CursorX = LINE_NUMBER_WIDTH;
            Inf.ColumnOffset = 0;
            ArrowKeys = 0;
        } return 0;
        case END_KEY:{
            uint32_t LineNumber = Inf.CursorY + Inf.RowOffset - 1;
            uint32_t LineSize = StringLength(target->Memory);
            
            uint32_t TotalColumns = LineSize + 3;

            if(TotalColumns <= (uint32_t)Inf.ConsoleColumns){
                Inf.ColumnOffset = 0;
                Inf.CursorX = TotalColumns;
            }
            else{
                Inf.ColumnOffset = TotalColumns - Inf.ConsoleColumns;
                Inf.CursorX = Inf.ConsoleColumns;
            }
            ArrowKeys = 0;
        } return 0;
        case DELETE_KEY:{
            StringBuffer *NextLine = StringBufferGetElemenetAt(&(Inf.RowArrayDisplay), Inf.CursorY + Inf.RowOffset);
            if(
                (LinePossition == StringLength(target->Memory) - 1)
                && Inf.CursorY + Inf.RowOffset < Inf.RowArrayDisplay.NumberOfElements
            ){

                StringBuffer temp = CreateBuffer(64);
                while(NextLine->Length > temp.Length) DoubleSize(&temp);
                RemoveLineEx(&(Inf.RowArrayDisplay), Inf.CursorY + Inf.RowOffset, temp.Memory);
                Inf.RowArrayDisplay.NumberOfElements--;

                uint32_t FullStrLength = StringLength(target->Memory) + StringLength(temp.Memory) - 1;

                while(FullStrLength > target->Length) DoubleSize(target);

                StringConcat(target->Memory, temp.Memory);
                DeleteBuffer(&temp);

                return 0;
            }

            StringShiftLeft(target->Memory, LinePossition, 0);
            
        } return 0;
        case CTRL_DELETE:{
            CTRL_DELETE_COMMANDS;
        }
    }
    switch (c) {
    case CTRL_KEY('q'):
        {
            if(Inf.EditorDirty) {
                PushEditorMessage("Exit without saving? (Yes, No, Save)");
                RefreshScreen();
                
                repeat:
                char c = _getch();
                if(c == 's') EditorSave(FileName);
                else if(c == 'n') {
                    DebugMessage.TimeOfCreation = 0;
                    DebugMessage.Message[0] = '\0';
                    return 0;
                }
                else if(c == 'y');
                else{
                    goto repeat;
                }
            }
            ResetCursorPossition;
            Running = 0;
        } return 0;
    case CTRL_KEY('c'):
        {
            char Str[64];
            UintToString(LinePossition, Str, 0);
            PushEditorMessage(Str);
        } return 0;
    case CTRL_KEY('s'):
        {
            uint8_t Failed = EditorSave(FileName);
            if(Failed)
                PushEditorMessage("Saving failed!");
            else
                PushEditorMessage("File saved");
        } return 0;

    case CTRL_KEY('d'):{
        CTRL_DELETE_COMMANDS;
    } return 0;

        // Ctrl + Backspace and Ctrl + W bound to same function
    case CTRL_KEY('w'):
    case 127:
    {
        if(LinePossition == 0) return 0;

        uint32_t n;
        if(*(target->Memory + LinePossition - 1) == ' ')
            n = CountBackToWord(target->Memory, LinePossition);
        else
            n = CountBackToBlank(target->Memory, LinePossition);
        
        uint32_t cnt = 0, limit = LinePossition - n;

        if(limit == 0) return 0;
        while(1) {
            StringShiftLeft(target->Memory, n, 0);
            cnt++;
            
            Inf.CursorX--;
            if(cnt == limit) {
                break;
            }
        }
    } return 0;

    case 8: // Backspace
        {
            if((int)LinePossition <= 0) {
                if((Inf.CursorY <= 1 && Inf.RowOffset == 0)) return 0;

                StringBuffer temp = CreateBuffer(64);
                while(target->Length > temp.Length) DoubleSize(&temp);
                RemoveLineEx(&(Inf.RowArrayDisplay), Inf.CursorY + Inf.RowOffset - 1, temp.Memory);
                Inf.RowArrayDisplay.NumberOfElements--;

                Inf.CursorY--;
                StringBuffer *NewLine = StringBufferGetElemenetAt(&(Inf.RowArrayDisplay), Inf.CursorY + Inf.RowOffset - 1);
                uint32_t StrLength = StringLength(NewLine->Memory);

                uint32_t FullStrLength = StringLength(temp.Memory) + StrLength - 1;

                while(FullStrLength > NewLine->Length) DoubleSize(NewLine);

                StringConcat(NewLine->Memory, temp.Memory);
                DeleteBuffer(&temp);
                
                Inf.CursorX = StrLength + 4;

                return 0;
            }

            if(Inf.InsertMode == 1) {
                target->Memory[LinePossition - 1] = '\0';
                Inf.CursorX--;
            }
            else {
                StringShiftLeft(target->Memory, LinePossition-1, 0);
                Inf.CursorX--;
            }
        } return 0;
    case '\r':{
        InsertLine(&(Inf.RowArrayDisplay), Inf.CursorY + Inf.RowOffset, target->Memory + LinePossition);
        target = StringBufferGetElemenetAt(
            &(Inf.RowArrayDisplay), 
            Inf.CursorY + Inf.RowOffset - 1
        );
        target->Memory[LinePossition] = '\0';
        //ZeroBufferEx(target, LinePossition);
        Inf.CursorY++;
        Inf.CursorX = 0;
    } return 0;
    case '\t':{
        //for(uint32_t i = 0; i < TAB_SPACE_COUNT; i++) InsertCharacter(' ');
        InsertCharacter(' ');
        while((Inf.CursorX + LINE_NUMBER_WIDTH) % TAB_SPACE_COUNT != 0) {
            InsertCharacter(' ');
        }
    } return 0;
    default:
        {
            //char Str[64];
            //CharToAnsi(c, StrExtr);
            //PrintToBuffer(&Buffer, Str);
            return c;
        }
    }
}

int main(int argc, char* argv[]){

    PrepareConsole();
    ScrollScreen();

    if(argc == 2)
        EditorOpen(argv[1]);
    else{
        Print("Error: You must provide an argument!\n");
        return 1;
    }

    //char Str[64];
    //char Str1[64];
    char C;

    //DWORD CharsRead;

    PushEditorMessage("Ctrl+q to quit");

    while(Running){
        GetConsoleSystemInfo();

        RefreshScreen();

        C = ProcessKeypress();

        InsertCharacter(C);
        
        HideConsoleCursor();
        FixCursorPossitionEx();
        DisplayConsoleCursor();

        //Sleep(TIMEOUT_MS);
    }

    ScrollScreen();
    //CloseHandle(hFile);

    //Print("Stopping program...");
    DisableRawMode();

    return 0;
}