#include "buffer8.h"


StringBuffer8 CreateBuffer8(int size){
    StringBuffer8 temp;
    temp.Size = 0;
    temp.Length = 0;
    temp.MaxSize = size;

    #if defined WINDOWS
        temp.Memory = HeapAlloc(hHeap, 0, size);
        temp.Inf = HeapAlloc(hHeap, 0, size);
    #elif define LINUX

    #endif
    ZeroBuffer8(&temp);
}

void DoubleSize8(StringBuffer8 *Buffer){
    Buffer->MaxSize *= 2;
    #if defined WINDOWS
        Buffer->Memory = HeapReAlloc(hHeap, 0, (LPVOID)Buffer->Memory, Buffer->Length);
        Buffer->Inf = HeapReAlloc(hHeap, 0, (LPVOID)Buffer->Inf, Buffer->MaxSize);
    #elif defined LINUX

    #endif
}

void DeleteBuffer8(StringBuffer8 *Buffer){
    #if defined WINDOWS
        HeapFree(hHeap, 0, Buffer->Memory);
        HeapFree(hHeap, 0, Buffer->Inf);
    #elif defined LINUX

    #endif

    Buffer->Memory = NULL;
    Buffer->Inf = NULL;
    Buffer->Length = 0;
    Buffer->Size = 0;
    Buffer->MaxSize = 0;
}

void ZeroBuffer8(StringBuffer8 *Buffer){
    for(int i = 0; i < Buffer->Size; i++){
        Buffer->Memory[i] = '\0';
        Buffer->Inf[i] = '\0';
    }
    Buffer->Length = 0;
    Buffer->Size = 0;
}

void ZeroBuffer8Ex(StringBuffer8 *Buffer, int StartIndex){
    int StartByte = StartIndex; // Placeholder
    for(int i = StartByte; i < Buffer->Size; i++) {
        Buffer->Memory[i] = '\0';
        Buffer->Inf[i] = '\0';
    }

    Buffer->Size -= StartByte;
    Buffer->Length -= StartIndex;
}

void AppendBuffer8(StringBuffer8 *Buffer, char *Str){
    //int BufferStringLength = StringLength(Buffer->Memory);
    int BufferStringSize = Buffer->Size;
    int StringSize = StringLength(Str);

    while(BufferStringSize + StringSize > Buffer->MaxSize)
        DoubleSize8(Buffer);
    
    StringConcat(Buffer->Memory, Str);

    Buffer->Size += StringSize;
}

uint32_t AppendBuffer8Ex(StringBuffer8 *Buffer, char *Str, int MaxLength, int Offset){
    int BufferStringSize = Buffer->Size; //StringLength(Buffer->Memory);
    int StringSize = StringLength(Str);

    int ActualSize = SmallerUnsigned(StringSize, MaxLength);

    while(BufferStringSize + ActualSize > Buffer->MaxSize)
        DoubleSize8(Buffer);

    MemoryCopy((Buffer->Memory) + BufferStringSize - 1, Str + Offset, ActualSize);
    *((Buffer->Memory) + BufferStringSize + ActualSize - 1) = '\0';

    return BufferStringSize + ActualSize;
}