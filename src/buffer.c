// String buffer which will be displayed to the screen 
#include "buffer.h"

StringBuffer CreateBuffer(uint32_t size){
    StringBuffer temp;
    temp.Memory = HeapAlloc(hHeap, 0, size);
    temp.Length = size;

    //*(temp.Memory) = '\0';
    ZeroBuffer(&temp);

    return temp;
}

void DoubleSize(StringBuffer *Buffer){
    LPVOID PtrM = Buffer->Memory;
    Buffer->Length *= 2;
    Buffer->Memory = HeapReAlloc(hHeap, 0, PtrM, Buffer->Length);
}

void DeleteBuffer(StringBuffer *Buffer){
    HeapFree(hHeap, 0, Buffer->Memory);
    Buffer->Memory = NULL;
    Buffer->Length = 0;
}

void ZeroBuffer(StringBuffer *Buffer){
    for(uint32_t i = 0; i < Buffer->Length; i++)
        Buffer->Memory[i] = '\0';
}

void ZeroBufferEx(StringBuffer *Buffer, uint32_t StartIndex){
    for(uint32_t i = StartIndex; i < Buffer->Length; i++)
        Buffer->Memory[i] = '\0';
}

void AppendBuffer(StringBuffer *Buffer, char *Str){
    uint32_t BufferStringLength = StringLength(Buffer->Memory);
    uint32_t StringSize = StringLength(Str);

    while(BufferStringLength + StringSize > Buffer->Length)
        DoubleSize(Buffer);
    
    StringConcat(Buffer->Memory, Str);
}

uint32_t AppendBufferEx(StringBuffer *Buffer, char *Str, uint32_t MaxLength, uint32_t Offset){
    uint32_t BufferStringLength = StringLength(Buffer->Memory);
    uint32_t StringSize = StringLength(Str);

    uint32_t ActualSize = SmallerUnsigned(StringSize, MaxLength);

    while(BufferStringLength + ActualSize > Buffer->Length)
        DoubleSize(Buffer);

    MemoryCopy((Buffer->Memory) + BufferStringLength - (uint32_t)1, Str + Offset, ActualSize);
    *((Buffer->Memory) + BufferStringLength + ActualSize - 1) = '\0';

    return BufferStringLength + ActualSize;
}

// StringBufferArray

StringBufferArray CreateBufferArray(uint32_t InitialMaxElements){
    StringBufferArray temp;
    temp.NumberOfElements = 0;
    temp.MaxNumberOfElements = InitialMaxElements;
    temp.MemorySize = sizeof(StringBuffer) * InitialMaxElements;

    temp.Data = HeapAlloc(hHeap, 0, temp.MemorySize);

    for(uint32_t i = 0; i < InitialMaxElements; i++)
        *StringBufferGetElemenetAt(&temp, i) = CreateBuffer(64);

    return temp;
}

void DoubleArrayCapacity(StringBufferArray *Array){
    uint32_t OldCapacity = Array->MaxNumberOfElements;
    Array->MaxNumberOfElements *= 2;
    Array->MemorySize = sizeof(StringBuffer) * Array->MaxNumberOfElements;

    StringBuffer *temp = HeapReAlloc(hHeap, 0, Array->Data, Array->MemorySize);
    if(temp == NULL){
        exit(1);
    }
    Array->Data = temp;


    for(uint32_t i = OldCapacity; i < Array->MaxNumberOfElements; i++)
        *StringBufferGetElemenetAt(Array, i) = CreateBuffer(64);
}

void DeleteBufferArray(StringBufferArray *Array){
    for (uint32_t i = 0; i < Array->NumberOfElements; i++) {
        StringBuffer *target = StringBufferGetElemenetAt(Array, i);
        HeapFree(hHeap, 0, target->Memory);
    }

    HeapFree(hHeap, 0, Array->Data);

    Array->MaxNumberOfElements = 0;
    Array->MemorySize = 0;
    Array->Data = NULL;
    Array->NumberOfElements = 0;
}

StringBuffer *StringBufferGetElemenetAt(StringBufferArray *Array, uint32_t ElementIndex){
    
    if(ElementIndex >= Array->MaxNumberOfElements)
        return NULL;

    return &(Array->Data[ElementIndex]);
}

void InsertLine(StringBufferArray *Array, uint32_t ElementIndex, char *Str){

    while(Array->NumberOfElements + 1 >= Array->MaxNumberOfElements) DoubleArrayCapacity(Array);

    for(uint32_t i = Array->NumberOfElements; i > ElementIndex; i--) {
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

void RemoveLine(StringBufferArray *Array, uint32_t ElementIndex){
    
    DeleteBuffer(StringBufferGetElemenetAt(Array, ElementIndex));

    for(uint32_t i = ElementIndex; i < Array->NumberOfElements; i++)
        *StringBufferGetElemenetAt(Array, i) = *StringBufferGetElemenetAt(Array, i + 1);

}

void RemoveLineEx(StringBufferArray *Array, uint32_t ElementIndex, char *OutStr){
    if(OutStr)
        StringCopy(OutStr, StringBufferGetElemenetAt(Array, ElementIndex)->Memory);
    
    DeleteBuffer(StringBufferGetElemenetAt(Array, ElementIndex));

    for(uint32_t i = ElementIndex; i < Array->NumberOfElements - 1; i++)
        *StringBufferGetElemenetAt(Array, i) = *StringBufferGetElemenetAt(Array, i + 1);

    StringBuffer *last = StringBufferGetElemenetAt(Array, Array->NumberOfElements - 1);
    last->Memory = NULL; 
    last->Length = 0;
}

uint32_t MaxLineLength(StringBufferArray *Array){
    uint32_t MaxLength = 0;
    for(uint32_t i = 0; i < Array->NumberOfElements; i++){
        StringBuffer *temp = StringBufferGetElemenetAt(Array, i);
        if(StringLength(temp->Memory)> MaxLength) MaxLength = StringLength(temp->Memory);
    }

    return MaxLength;
}