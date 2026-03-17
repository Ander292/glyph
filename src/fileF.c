#include "fileF.h"

void ReturnFileName(char *FullPath, char *OutFileName){
    int FullPathLength = StringLength(FullPath);
    int i;

    for(i = FullPathLength - 1; i < FullPathLength; i--)
        if(FullPath[i] == '\\' || FullPath[i] == '/')
            break;

    if(i >= FullPathLength) {
        StringCopy(OutFileName, FullPath);
        return;
    }

    MemoryCopy(OutFileName, FullPath + i + 1, FullPathLength - i - 1);
}

void SeparateIntoLines(StringBufferArray *StrArray, StringBuffer *PrimaryBuffer, int *LastLineIndex, LineContinuationInfo *Continuation){
    
    char *PtrToCurrentPos = PrimaryBuffer->Memory; // Current possition inside the main buffer
    size_t RemainingSize = PrimaryBuffer->Length;

    int LineNumber = *LastLineIndex;

    if(Continuation->BufferIndex != 0xFFFFFFF){
        StringBuffer *target = StringBufferGetElemenetAt(StrArray, Continuation->BufferIndex);
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

        if (RemainingSize > 0 && *PtrToCurrentPos == '\r') { PtrToCurrentPos++; RemainingSize--; }
        if (RemainingSize > 0 && *PtrToCurrentPos == '\n') { PtrToCurrentPos++; RemainingSize--; }

        if (FoundNewline) {
            Continuation->BufferIndex = 0xFFFFFFFF;
            LineNumber++;   
        } else {
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

        if (RemainingSize > 0 && *PtrToCurrentPos == '\r') { PtrToCurrentPos++; RemainingSize--; }
        if (RemainingSize > 0 && *PtrToCurrentPos == '\n') { PtrToCurrentPos++; RemainingSize--; }

        if (!FoundNewline) {
            Continuation->BufferIndex = LineNumber;
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

    LineContinuationInfo Continuation = { 0xFFFFFFF, 0, 0 };

    do{
        ReadFile(hFile, PrimaryBuffer.Memory, PortionSize, &ReadFeedback, NULL);

        PrimaryBuffer.Memory[ReadFeedback] = '\0';
        PrimaryBuffer.Length = ReadFeedback;


        SeparateIntoLines(StrArray, &PrimaryBuffer, &LastLineIndex, &Continuation);
    }while(ReadFeedback >= PortionSize);

    if(StrArray->NumberOfElements == 0) StrArray->NumberOfElements = 1;

    DeleteBuffer(&PrimaryBuffer);
}