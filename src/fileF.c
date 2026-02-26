#include "fileF.h"

void ReturnFileName(char *FullPath, char *OutFileName){
    uint32_t FullPathLength = StringLength(FullPath);
    uint32_t i;

    for(i = FullPathLength - 1; i < FullPathLength; i--)
        if(FullPath[i] == '\\' || FullPath[i] == '/')
            break;

    if(i >= FullPathLength) {
        StringCopy(OutFileName, FullPath);
        return;
    }

    MemoryCopy(OutFileName, FullPath + i + 1, FullPathLength - i - 1);
}

void SeparateIntoLines(StringBufferArray *StrArray, StringBuffer *PrimaryBuffer, uint32_t *LastLineIndex, LineContinuationInfo *Continuation){
    
    char *PtrToCurrentPos = PrimaryBuffer->Memory; // Current possition inside the main buffer
    size_t RemainingSize = PrimaryBuffer->Length;

    uint32_t LineNumber = *LastLineIndex;

    if(Continuation->BufferIndex != 0xFFFFFFFF){
        StringBuffer *target = StringBufferGetElemenetAt(StrArray, Continuation->BufferIndex);
        uint32_t CurrentLineLength = StringLength(target->Memory) - 1;
        uint32_t Offset = Continuation->Offset;

        uint8_t FoundNewline = 0;
        uint32_t CopyLength = LineLengthEx(PtrToCurrentPos, RemainingSize, &FoundNewline);

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
        uint32_t CopyLength = LineLengthEx(PtrToCurrentPos, RemainingSize, &FoundNewline);

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

void FileReadPortionS(HANDLE hFile, uint32_t PortionSize, StringBufferArray *StrArray){

    StringBuffer PrimaryBuffer = CreateBuffer(PortionSize + 1);

    DWORD ReadFeedback;
    uint32_t LastLineIndex = 0;

    LineContinuationInfo Continuation = { 0xFFFFFFFF, 0, 0 };

    do{
        ReadFile(hFile, PrimaryBuffer.Memory, PortionSize, &ReadFeedback, NULL);

        PrimaryBuffer.Memory[ReadFeedback] = '\0';
        PrimaryBuffer.Length = ReadFeedback;


        SeparateIntoLines(StrArray, &PrimaryBuffer, &LastLineIndex, &Continuation);
    }while(ReadFeedback >= PortionSize);

    if(StrArray->NumberOfElements == 0) StrArray->NumberOfElements = 1;

    DeleteBuffer(&PrimaryBuffer);
}