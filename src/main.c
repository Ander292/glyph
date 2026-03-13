#include "main.h"

//---Editor---//

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
    while(Inf.RowArray.MaxNumberOfElements < Inf.RowArrayOrigin.MaxNumberOfElements){
        DoubleArrayCapacity(&(Inf.RowArray));
    }
    for(uint32_t i = 0; i <= Inf.RowArrayOrigin.NumberOfElements; i++){
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
    if((uint32_t)Inf.CursorY > Inf.RowArray.NumberOfElements - 1) Inf.CursorY = Inf.RowArray.NumberOfElements - 1;

    StringBuffer *target = StringBufferGetElemenetAt(&(Inf.RowArray), Inf.CursorY);
    uint32_t LineLength = StringLength(target->Memory) - 1;
    
    if(Inf.CursorX < 0) Inf.CursorX = 0;
    if((uint32_t)Inf.CursorX > LineLength) Inf.CursorX = LineLength;
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

    uint32_t LineCount = Inf.RowArray.NumberOfElements;
    UintToString(LineCount, StrAux, 3);
    StringConcat(StrMain, StrAux);

    StringConcat(StrMain, " | L:");

    UintToString(Inf.CursorY + 1, StrAux, 3);
    StringConcat(StrMain, StrAux);

    StringConcat(StrMain, " | C:");

    UintToString(Inf.CursorX + 1, StrAux, 3);
    //uint32_t RowMaxSize = StringBufferGetElemenetAt(&(Inf.RowArray), Inf.CursorY + Inf.RowOffset - 1)->Length;

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
        StringConcat(DebugMessage.Message, "---");
        return 1U;
    }
    return 0U;
}

void FormatHeader(){
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

    StringConcat(StrMain, "|: ");
    StringConcat(StrMain, DebugMessage.Message);
    StringConcat(StrMain, " :|");
    if((uint32_t)MessageSpace > EditorMessageLength){
        MessageSpace = MessageSpace - EditorMessageLength - 6;
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
    /* This works. Better solution bellow
    Inf.ColumnOffset = 0;
    Inf.RowOffset = 0;
    if(Inf.ConsoleColumns - 5 < Inf.CursorX)
            Inf.ColumnOffset = Inf.CursorX - Inf.ConsoleColumns + 4;
    if(Inf.ConsoleRows - 1 < Inf.CursorY)
            Inf.RowOffset = Inf.CursorY - Inf.ConsoleRows + 1;
    */

    if(Inf.CursorX > (int)(Inf.ConsoleColumns - 4 + Inf.ColumnOffset))
        Inf.ColumnOffset = Inf.CursorX - Inf.ConsoleColumns + 4;
    if(Inf.CursorY > (int)(Inf.ConsoleRows + Inf.RowOffset - 1))
        Inf.RowOffset = Inf.CursorY - Inf.ConsoleRows + 1;
    
    if(Inf.CursorX < (int)(Inf.ColumnOffset))
        Inf.ColumnOffset = Inf.CursorX;
    if(Inf.CursorY < (int)(Inf.RowOffset))
        Inf.RowOffset = Inf.CursorY;

    for(uint32_t i = 0; (i + Inf.RowOffset) < Inf.RowArray.NumberOfElements && i < (uint32_t)(Inf.ConsoleRows); i++) {
        uint32_t RowNumber = i + Inf.RowOffset;

        UintToString((RowNumber + 1) % 1000, Str, 3);

        PrintToBuffer(&Buffer, Str);
        PrintToBuffer(&Buffer, "|");

        StringBuffer *temp = StringBufferGetElemenetAt(&(Inf.RowArray), RowNumber);
        uint32_t StringSize = StringLength(temp->Memory) - 1;

        

        if(StringSize < Inf.ColumnOffset)
            AppendBufferEx(&Buffer, "<--", 3, 0);
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

    Print(Buffer.Memory);

    SetCursorPossition(Inf.CursorX + LINE_NUMBER_WIDTH - Inf.ColumnOffset, Inf.CursorY + 1 - Inf.RowOffset);
    DisplayConsoleCursor();
}

void InsertCharacter(char C){
    if(C == 0) return;
    Inf.EditorDirty = 1;
    uint32_t CurrentColumn = (uint32_t)(Inf.CursorX);
    uint32_t CurrentLine = (uint32_t)(Inf.CursorY);

    StringBuffer *target = StringBufferGetElemenetAt(&(Inf.RowArray), CurrentLine);
    uint32_t CurrentLineLength = StringLength(target->Memory);

    uint32_t CurrentBufferSize = target->Length;

    if(CurrentLineLength + 1 > CurrentBufferSize) {
        DoubleSize(target);
        target = StringBufferGetElemenetAt(&(Inf.RowArray), CurrentLine);

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


char ProcessKeypress(){
    char c = ReadCharacterEx();

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
            uint32_t n;
            if(*(target->Memory + Inf.CursorX) == ' ')
                n = CountForwardToWordEx(target->Memory, Inf.CursorX);
            else
                n = CountForwardToBlankEx(target->Memory, Inf.CursorX);
            Inf.CursorX = Inf.CursorX + n;
            ArrowKeys = 0;
        } return 0;
        case CTRL_LEFT:{
            if(Inf.CursorX == 0) return 0;
            uint32_t n;
            if(*(target->Memory + Inf.CursorX - 1) == ' ')
                n = CountBackToWordEx(target->Memory, Inf.CursorX);
            else
                n = CountBackToBlankEx(target->Memory, Inf.CursorX);
            Inf.CursorX = Inf.CursorX - n;
            ArrowKeys = 0;
        } return 0;
        case CTRL_UP:{
            if(Inf.RowOffset > 0){
                Inf.RowOffset--;
                Inf.CursorY--;
            } 
        } return 0;
        case CTRL_DOWN:{
            if(Inf.RowOffset < Inf.RowArray.NumberOfElements - Inf.CursorY - Inf.ConsoleColumns){
                Inf.RowOffset++;
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
            uint32_t LineNumber = Inf.CursorY;
            uint32_t LineSize = StringLength(target->Memory) - 1;
            
            Inf.CursorX = LineSize;
            ArrowKeys = 0;
        } return 0;
        case DELETE_KEY:{
            Inf.EditorDirty = 1;
            if(
                ((uint32_t)Inf.CursorX == StringLength(target->Memory) - 1)
                && (uint32_t)Inf.CursorY < Inf.RowArray.NumberOfElements - 1
            ){
                StringBuffer *NextLine = StringBufferGetElemenetAt(&(Inf.RowArray), Inf.CursorY);
                if(NextLine == NULL) return 0;

                StringBuffer temp = CreateBuffer(64);
                while(NextLine->Length > temp.Length) DoubleSize(&temp);
                RemoveLineEx(&(Inf.RowArray), Inf.CursorY + 1, temp.Memory);
                Inf.RowArray.NumberOfElements--;

                uint32_t FullStrLength = StringLength(target->Memory) + StringLength(temp.Memory) - 1;

                while(FullStrLength > target->Length) DoubleSize(target);

                StringConcat(target->Memory, temp.Memory);
                DeleteBuffer(&temp);

                return 0;
            }

            StringShiftLeft(target->Memory, Inf.CursorX, 0);
            
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
            UintToString(Inf.CursorX, Str, 0);
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
        Inf.EditorDirty = 1;
        if(Inf.CursorX == 0) return 0;

        uint32_t n;
        if(*(target->Memory + Inf.CursorX - 1) == ' ')
            n = CountBackToWordEx(target->Memory, Inf.CursorX);
        else
            n = CountBackToBlankEx(target->Memory, Inf.CursorX);
        
        uint32_t counter = n;
        int OldCursor = Inf.CursorX;

        while(1){
            if(counter == 0) break;
            counter--;
            StringShiftLeft(target->Memory, Inf.CursorX - 1, 0);
            Inf.CursorX--;
        }

    } return 0;

    case 8: // Backspace
        {
            Inf.EditorDirty = 1;
            if((int)Inf.CursorX == 0) {
                if((Inf.CursorY == 0 && Inf.RowOffset == 0)) return 0;

                StringBuffer temp = CreateBuffer(64);
                while(target->Length > temp.Length) DoubleSize(&temp);
                RemoveLineEx(&(Inf.RowArray), Inf.CursorY, temp.Memory);
                Inf.RowArray.NumberOfElements--;

                Inf.CursorY--;
                StringBuffer *NewLine = StringBufferGetElemenetAt(&(Inf.RowArray), Inf.CursorY);
                uint32_t StrLength = StringLength(NewLine->Memory) - 1;

                uint32_t FullStrLength = StringLength(temp.Memory) + StrLength;

                while(FullStrLength > NewLine->Length) DoubleSize(NewLine);

                StringConcat(NewLine->Memory, temp.Memory);
                DeleteBuffer(&temp);
                
                Inf.CursorX = StrLength;

                return 0;
            }

            if(Inf.InsertMode == 1) { // Unsuported
                target->Memory[Inf.CursorX - 1] = '\0';
                Inf.CursorX--;
            }
            else {
                StringShiftLeft(target->Memory, Inf.CursorX-1, 0);
                Inf.CursorX--;
            }
        } return 0;
    case '\r':{
        Inf.EditorDirty = 1;
        InsertLine(&(Inf.RowArray), Inf.CursorY + 1, target->Memory + Inf.CursorX);
        target = StringBufferGetElemenetAt(
            &(Inf.RowArray), 
            Inf.CursorY
        );
        target->Memory[Inf.CursorX] = '\0';
        Inf.CursorY++;
        Inf.CursorX = 0;
    } return 0;
    case '\t':{
        InsertCharacter(' ');
        while((Inf.CursorX) % TAB_SPACE_COUNT != 0) {
            InsertCharacter(' ');
        }
    } return 0;
    default:
        {
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

    char C;

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
    DisableRawMode();

    return 0;
}