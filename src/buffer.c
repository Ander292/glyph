// String buffer which will be displayed to the screen 
#include "buffer.h"

StringBuffer CreateBuffer(uint32_t size){
    StringBuffer temp;
    #if defined WINDOWS
        temp.Memory = HeapAlloc(hHeap, 0, size);
    #elif define LINUX

    #endif
    temp.Length = size;

    //*(temp.Memory) = '\0';
    ZeroBuffer(&temp);

    return temp;
}

void DoubleSize(StringBuffer *Buffer){
    LPVOID PtrM = Buffer->Memory;
    Buffer->Length *= 2;
    #if defined WINDOWS
        Buffer->Memory = HeapReAlloc(hHeap, 0, PtrM, Buffer->Length);
    #elif defined LINUX

    #endif
}

void DeleteBuffer(StringBuffer *Buffer){
    #if defined WINDOWS
        HeapFree(hHeap, 0, Buffer->Memory);
    #elif defined LINUX

    #endif
    Buffer->Memory = NULL;
    Buffer->Length = 0;
}

void ZeroBuffer(StringBuffer *Buffer){
    for(int i = 0; i < Buffer->Length; i++)
        Buffer->Memory[i] = '\0';
}

void ZeroBufferEx(StringBuffer *Buffer, int StartIndex){
    for(int i = StartIndex; i < Buffer->Length; i++)
        Buffer->Memory[i] = '\0';
}

void AppendBuffer(StringBuffer *Buffer, char *Str){
    int BufferStringLength = StringLength(Buffer->Memory);
    int StringSize = StringLength(Str);

    while(BufferStringLength + StringSize > Buffer->Length)
        DoubleSize(Buffer);
    
    StringConcat(Buffer->Memory, Str);
}

uint32_t AppendBufferEx(StringBuffer *Buffer, char *Str, int MaxLength, int Offset){
    int BufferStringLength = StringLength(Buffer->Memory);
    int StringSize = StringLength(Str);

    int ActualSize = SmallerUnsigned(StringSize, MaxLength);

    while(BufferStringLength + ActualSize > Buffer->Length)
        DoubleSize(Buffer);

    MemoryCopy((Buffer->Memory) + BufferStringLength - 1, Str + Offset, ActualSize);
    *((Buffer->Memory) + BufferStringLength + ActualSize - 1) = '\0';

    return BufferStringLength + ActualSize;
}

// StringBufferArray

StringBufferArray CreateBufferArray(int InitialMaxElements){
    StringBufferArray temp;
    temp.NumberOfElements = 0;
    temp.MaxNumberOfElements = InitialMaxElements;
    temp.MemorySize = sizeof(StringBuffer) * InitialMaxElements;

    temp.Data = HeapAlloc(hHeap, 0, temp.MemorySize);

    for(int i = 0; i < InitialMaxElements; i++)
        *StringBufferGetElemenetAt(&temp, i) = CreateBuffer(64);

    return temp;
}

void DoubleArrayCapacity(StringBufferArray *Array){
    int OldCapacity = Array->MaxNumberOfElements;
    Array->MaxNumberOfElements *= 2;
    Array->MemorySize = sizeof(StringBuffer) * Array->MaxNumberOfElements;

    StringBuffer *temp = HeapReAlloc(hHeap, 0, Array->Data, Array->MemorySize);
    if(temp == NULL){
        exit(1);
    }
    Array->Data = temp;


    for(int i = OldCapacity; i < Array->MaxNumberOfElements; i++)
        *StringBufferGetElemenetAt(Array, i) = CreateBuffer(64);
}

void DeleteBufferArray(StringBufferArray *Array){
    for (int i = 0; i < Array->NumberOfElements; i++) {
        StringBuffer *target = StringBufferGetElemenetAt(Array, i);
        HeapFree(hHeap, 0, target->Memory);
    }

    HeapFree(hHeap, 0, Array->Data);

    Array->MaxNumberOfElements = 0;
    Array->MemorySize = 0;
    Array->Data = NULL;
    Array->NumberOfElements = 0;
}

StringBuffer *StringBufferGetElemenetAt(StringBufferArray *Array, int ElementIndex){
    
    if(ElementIndex >= Array->MaxNumberOfElements)
        return NULL;

    return &(Array->Data[ElementIndex]);
}

void InsertLine(StringBufferArray *Array, int ElementIndex, char *Str){

    while(Array->NumberOfElements + 1 >= Array->MaxNumberOfElements) DoubleArrayCapacity(Array);

    for(int i = Array->NumberOfElements; i > ElementIndex; i--) {
        /*StringBuffer *dest = StringBufferGetElemenetAt(Array, i + 1);
        StringBuffer *src = StringBufferGetElemenetAt(Array, i);
        *dest = *src;*/
        *StringBufferGetElemenetAt(Array, i) = *StringBufferGetElemenetAt(Array, i - 1);
    }

    Array->NumberOfElements++;

    StringBuffer *target = StringBufferGetElemenetAt(Array, ElementIndex);

    target->Memory = NULL;
    target->Length = 0;

    *target = CreateBuffer(64);
    while(target->Length < StringLength(Str)) DoubleSize(target);
    StringCopy(target->Memory, Str);
}

void RemoveLine(StringBufferArray *Array, int ElementIndex){
    
    DeleteBuffer(StringBufferGetElemenetAt(Array, ElementIndex));

    for(int i = ElementIndex; i < Array->NumberOfElements; i++)
        *StringBufferGetElemenetAt(Array, i) = *StringBufferGetElemenetAt(Array, i + 1);

}

void RemoveLineEx(StringBufferArray *Array, int ElementIndex, char *OutStr){
    if(OutStr)
        StringCopy(OutStr, StringBufferGetElemenetAt(Array, ElementIndex)->Memory);
    
    DeleteBuffer(StringBufferGetElemenetAt(Array, ElementIndex));

    for(int i = ElementIndex; i < Array->NumberOfElements - 1; i++)
        *StringBufferGetElemenetAt(Array, i) = *StringBufferGetElemenetAt(Array, i + 1);

    StringBuffer *last = StringBufferGetElemenetAt(Array, Array->NumberOfElements - 1);
    last->Memory = NULL; 
    last->Length = 0;
}

int MaxLineLength(StringBufferArray *Array){
    int MaxLength = 0;
    for(int i = 0; i < Array->NumberOfElements; i++){
        StringBuffer *temp = StringBufferGetElemenetAt(Array, i);
        if(StringLength(temp->Memory)> MaxLength) MaxLength = StringLength(temp->Memory);
    }

    return MaxLength;
}