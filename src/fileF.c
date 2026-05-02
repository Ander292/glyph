#define FILE_C
#include "fileF.h"

// ---------BufferTranslation--------- //

StringBufferA TranslateToUtf8Ex(wchar *Src, uint32_t *WriteSize){
    // uint32_t DestSize = (uint32_t)WideCharToMultiByte(CP_UTF8, 0, Src, -1, 
    //     NULL, 0, NULL, NULL);
    uint32_t DestSize = GetConvertedSize8(Src);
    StringBufferA DestBuffer = CreateBufferA(DestSize + 1);
    // WideCharToMultiByte(CP_UTF8, 0, temp->Memory, -1, 
    //     LoopConvertBuffer.Memory, LoopConvertBuffer.Length, NULL, NULL);
    TranslateToUtf8(DestBuffer.Memory, DestSize, Src, -1);

    if(WriteSize != NULL){
        *WriteSize = DestSize;
    }
    return DestBuffer;
}

StringBuffer TranslateToUtf16Ex(char *Src, uint32_t *WriteSize){
    DWORD DestinationSize = GetConvertedSize16(Src);
    StringBuffer DestBuffer = CreateBuffer(DestinationSize);
    TranslateToUtf16(DestBuffer.Memory, DestinationSize, Src, -1);
    //MultiByteToWideChar(CP_UTF8, 0, Src, -1, DestBuffer.Memory, DestinationSize);

    if(WriteSize != NULL){
        *WriteSize = DestinationSize;
    }
    return DestBuffer;
}

void ReturnFileName(wchar *FullPath, wchar *OutFileName){
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

int SeparateIntoLines(StringBufferArray *StrArray, StringBuffer *PrimaryBuffer){
    
    wchar *PtrToCurrentPos = PrimaryBuffer->Memory; // Current possition inside the main buffer

    int RemainingSize = PrimaryBuffer->Length - 1;

    #ifndef LINUX
        if(*PtrToCurrentPos == '\n') {
            PtrToCurrentPos++;
            RemainingSize--;
        }
    #endif

    int LineNumber = 0;

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
                    RemainingSize -= 2;
                    continue;
                }
        if(RemainingSize > 0 && *PtrToCurrentPos == '\r') { PtrToCurrentPos++; RemainingSize--; }
        if(RemainingSize > 0 && *PtrToCurrentPos == '\n') { PtrToCurrentPos++; RemainingSize--; }
        // while(RemainingSize > 0 && (*PtrToCurrentPos == '\r' || *PtrToCurrentPos == '\n')) 
        //     { PtrToCurrentPos++; RemainingSize--; }

        LineNumber++;
    }
    return LineNumber;
}

int FileReadPortionS8(HANDLE hFile, DWORD PortionSize, StringBufferArray *StrArray){
    StringBufferA PrimaryBuffer = CreateBufferA(PortionSize + 1);

    DWORD ReadFeedback;

    ReadFile(hFile, PrimaryBuffer.Memory, PortionSize, &ReadFeedback, NULL);

    PrimaryBuffer.Memory[ReadFeedback] = '\0';
    PrimaryBuffer.Length = ReadFeedback;
    
    StringBuffer DestBuffer = TranslateToUtf16Ex(PrimaryBuffer.Memory, NULL);
    int Result = SeparateIntoLines(StrArray, &DestBuffer);
    DeleteBuffer(&DestBuffer);

    if(StrArray->NumberOfElements == 0) StrArray->NumberOfElements = 1;

    DeleteBufferA(&PrimaryBuffer);
    return Result;
}

int FileReadPortionS16(HANDLE hFile, DWORD PortionSize, StringBufferArray *StrArray){
    StringBuffer PrimaryBuffer = CreateBuffer(PortionSize + 1);

    DWORD ReadFeedback;
    
    ReadFile(hFile, PrimaryBuffer.Memory, PortionSize, &ReadFeedback, NULL);

    PrimaryBuffer.Memory[ReadFeedback] = '\0';
    PrimaryBuffer.Length = (ReadFeedback + 1)/ 2 + 1; // +1 because I remove one later, dont ask

    int Result = SeparateIntoLines(StrArray, &PrimaryBuffer);

    if(StrArray->NumberOfElements == 0) StrArray->NumberOfElements = 1;

    DeleteBuffer(&PrimaryBuffer);
    return Result;
}

int FileReadPortionS(HANDLE hFile, DWORD PortionSize, StringBufferArray *StrArray, uint8_t *FileMode){
    //DWORD Result;
    // BOM check
    wchar BOM = 0xFEFF;
    wchar Check;
    ReadFile(hFile, &Check, 2, NULL, NULL);
    if(Check == BOM){
        *FileMode = MODE_UTF16;
    }
    else{
        SetFilePointerEx(hFile, (LARGE_INTEGER)0LL, NULL, FILE_BEGIN);
    }

    int Result;

    if(*FileMode == MODE_UTF8)
        Result = FileReadPortionS8(hFile, PortionSize, StrArray);
    else
        Result = FileReadPortionS16(hFile, PortionSize, StrArray);

    return Result;
}