// String buffer which will be displayed to the screen 
#include "bufferANSI.h"

// String.h functions for ANSI

int StringLengthA(char* ptr){
    int Length = 0;
    for(Length = 0; *(ptr + Length) != '\0'; Length++); // This will just count the characters
    
    
    return Length + 1;
}

void StringConcatA(char *Destination, char *Source){
    int DestinationLength = StringLengthA(Destination);
    int SourceLength = StringLengthA(Source);

    //uint32_t Pos1 = SourceLength - 1;
    //uint32_t Pos2 = 0;

    /*while(Pos2 < SourceLength){
        Destination[Pos1++] = Source[Pos2++];
    }*/
    for(int i = 0; i < SourceLength; i++)
        Destination[DestinationLength + i - 1] = Source[i];
}

void MemoryCopyA(char *Destination, char *Source, int Size){
    for(int i = 0; i < Size; i++)
        Destination[i] = Source[i];
}

void ZeroBufferA(StringBufferA *Buffer){
    for(int i = 0; i < Buffer->Length; i++)
        Buffer->Memory[i] = '\0';
}

StringBufferA CreateBufferA(int size){
    StringBufferA temp;
    #if defined WINDOWS
        temp.Memory = HeapAlloc(hHeap, 0, size);
    #elif define LINUX

    #endif
    temp.Length = size;

    //*(temp.Memory) = '\0';
    ZeroBufferA(&temp);

    return temp;
}

void DoubleSizeA(StringBufferA *Buffer){
    LPVOID PtrM = Buffer->Memory;
    Buffer->Length *= 2;
    #if defined WINDOWS
        Buffer->Memory = HeapReAlloc(hHeap, 0, PtrM, Buffer->Length);
    #elif defined LINUX

    #endif
}

void DeleteBufferA(StringBufferA *Buffer){
    #if defined WINDOWS
        HeapFree(hHeap, 0, Buffer->Memory);
    #elif defined LINUX

    #endif
    Buffer->Memory = NULL;
    Buffer->Length = 0;
}

void ZeroBufferExA(StringBufferA *Buffer, int StartIndex){
    for(int i = StartIndex; i < Buffer->Length; i++)
        Buffer->Memory[i] = '\0';
}

void AppendBufferA(StringBufferA *Buffer, char *Str){
    int BufferStringLength = StringLengthA(Buffer->Memory);
    int StringSize = StringLengthA(Str);

    while(BufferStringLength + StringSize > Buffer->Length)
        DoubleSizeA(Buffer);
    
    StringConcatA(Buffer->Memory, Str);
}

uint32_t AppendBufferExA(StringBufferA *Buffer, char *Str, int MaxLength, int Offset){
    int BufferStringLength = StringLengthA(Buffer->Memory);
    int StringSize = StringLengthA(Str);

    int ActualSize = SmallerUnsigned(StringSize, MaxLength);

    while(BufferStringLength + ActualSize > Buffer->Length)
        DoubleSizeA(Buffer);

    MemoryCopyA((Buffer->Memory) + BufferStringLength - 1, Str + Offset, ActualSize);
    *((Buffer->Memory) + BufferStringLength + ActualSize - 1) = '\0';

    return BufferStringLength + ActualSize;
}