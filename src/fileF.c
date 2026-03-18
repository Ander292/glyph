#include "fileF.h"

void ReturnFileName(char *FullPath, char *OutFileName){
    int FullPathLength = StringLength(FullPath);
    int i;

    for(i = FullPathLength - 1; i > 0; i--)
        if(FullPath[i] == '\\' || FullPath[i] == '/')
            break;

    if(i == 0) {
        StringCopy(OutFileName, FullPath);
        return;
    }

    MemoryCopy(OutFileName, FullPath + i + 1, FullPathLength - i - 1);
}

void SeparateIntoLines(StringBufferArray *StrArray, StringBuffer *PrimaryBuffer, int *LastLineIndex, LineContinuationInfo *Continuation){
    
    char *PtrToCurrentPos = PrimaryBuffer->Memory; // Current possition inside the main buffer

    int RemainingSize = PrimaryBuffer->Length;

    #ifndef LINUX
        if(*PtrToCurrentPos == '\n') {
            PtrToCurrentPos++;
            RemainingSize--;
        }
    #endif

    int LineNumber = *LastLineIndex;

    if(Continuation->Shift == 1){
        StringBuffer *target = StringBufferGetElemenetAt(StrArray, LineNumber);
        int CurrentLineLength = StringLength(target->Memory) - 1;
        int Offset = Continuation->Offset;

        uint8_t FoundNewline = 0;
        int CopyLength = LineLengthEx(PtrToCurrentPos, RemainingSize, &FoundNewline);

        while(CurrentLineLength + CopyLength + 1 > target->Length)
            DoubleSize(target);

        MemoryCopy(target->Memory + CurrentLineLength, PtrToCurrentPos, CopyLength);

        CurrentLineLength += CopyLength;
        target->Memory[CurrentLineLength] = '\0';

        PtrToCurrentPos += CopyLength;
        RemainingSize -= CopyLength;

        if(RemainingSize > 0 && *PtrToCurrentPos == '\r') { PtrToCurrentPos++; RemainingSize--; }
        if(RemainingSize > 0 && *PtrToCurrentPos == '\n') { PtrToCurrentPos++; RemainingSize--; }

        if(FoundNewline){
            LineNumber++;
            Continuation->Shift = 0;
        }else{
            Continuation->Shift = 1;
            //Continuation->BufferIndex = LineNumber;
            Continuation->Offset = CurrentLineLength;
            *LastLineIndex = LineNumber;
            return;
        }
    }

    while(RemainingSize > 0){
        uint8_t FoundNewline = 0;
        int CopyLength = LineLengthEx(PtrToCurrentPos, RemainingSize, &FoundNewline);

        if(LineNumber >= StrArray->MaxNumberOfElements) 
            DoubleArrayCapacity(StrArray);

        StringBuffer *target = StringBufferGetElemenetAt(StrArray, LineNumber);
        while(target->Length <= CopyLength + 1) DoubleSize(target);

        MemoryCopy(target->Memory, PtrToCurrentPos, CopyLength);
        target->Memory[CopyLength] = '\0';

        StrArray->NumberOfElements = LineNumber + 1;

        PtrToCurrentPos += CopyLength;
        RemainingSize -= CopyLength;

        if(RemainingSize > 0 && *PtrToCurrentPos == '\r' 
            && *(PtrToCurrentPos + 1) == '\n' && *(PtrToCurrentPos + 2) == '\000'){
                    LineNumber++;
                    StrArray->NumberOfElements++;
                    RemainingSize-=2;
                    continue;
                }
        if(RemainingSize > 0 && *PtrToCurrentPos == '\r') { PtrToCurrentPos++; RemainingSize--; }
        if(RemainingSize > 0 && *PtrToCurrentPos == '\n') { PtrToCurrentPos++; RemainingSize--; }
        // while(RemainingSize > 0 && (*PtrToCurrentPos == '\r' || *PtrToCurrentPos == '\n')) 
        //     { PtrToCurrentPos++; RemainingSize--; }

        if(!FoundNewline){
            *LastLineIndex = LineNumber;
            Continuation->Shift = 1;
            Continuation->Offset = CopyLength;
            break;
        }

        LineNumber++;
    }

    *LastLineIndex = LineNumber;

}

void FileReadPortionS(HANDLE hFile, DWORD PortionSize, StringBufferArray *StrArray){

    StringBuffer PrimaryBuffer = CreateBuffer(PortionSize + 1);

    DWORD ReadFeedback;
    int LastLineIndex = 0;

    LineContinuationInfo Continuation = { -1, 0, 0 };

    do{
        ReadFile(hFile, PrimaryBuffer.Memory, PortionSize, &ReadFeedback, NULL);

        PrimaryBuffer.Memory[ReadFeedback] = '\0';
        PrimaryBuffer.Length = ReadFeedback;

        SeparateIntoLines(StrArray, &PrimaryBuffer, &LastLineIndex, &Continuation);
    }while(ReadFeedback >= PortionSize);

    if(StrArray->NumberOfElements == 0) StrArray->NumberOfElements = 1;
    //StrArray->NumberOfElements++;


    DeleteBuffer(&PrimaryBuffer);
}