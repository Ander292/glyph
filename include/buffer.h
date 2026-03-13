#ifndef BUFFER_H
    #include "stringF.h"
    #define WINDOWS
    #include <windows.h>

    //-----StructDefinitions-----//
    
        typedef struct{
            char *Memory;
            uint32_t Length;
        } StringBuffer;

        typedef struct{
            StringBuffer *Data;
            size_t MemorySize;
            uint32_t NumberOfElements;
            uint32_t MaxNumberOfElements;
        } StringBufferArray;

    //-----Global Variables-----//
        #if defined WINDOWS
            extern HANDLE hHeap;
            extern HANDLE hStdout;
        #elif defined LINUX

        #endif
    //-----FunctionDefinitions-----//

        // Creates a back buffer
        StringBuffer CreateBuffer(uint32_t size);

        // Doubles the size of the back buffer
        void DoubleSize(StringBuffer *Buffer);

        // Delets the back buffer
        void DeleteBuffer(StringBuffer *Buffer);

        // Zeroes the buffer to clear its content
        void ZeroBuffer(StringBuffer *Buffer);

        // Zeroes the buffer starting at a certain possition
        void ZeroBufferEx(StringBuffer *Buffer, uint32_t StartIndex);

        // Append to the end of the buffer (at the termination character)
        void AppendBuffer(StringBuffer *Buffer, char *Str);

        // Apped to the end of the buffer (maximum of MaxLength characters)
        uint32_t AppendBufferEx(
            StringBuffer *Buffer,
            char *Str,
            uint32_t MaxLength,
            uint32_t Offset
        );

        // Creates a new array of string buffers
        StringBufferArray CreateBufferArray(uint32_t InitialMaxElements);

        // Doubles the capacity of a StringArrayBuffer 
        // Alocates sufficient space on the heap for all the new elements
        void DoubleArrayCapacity(StringBufferArray *Array);

        // Deletes the whole buffer array including its individual elements
        // and frees the heap used by them
        void DeleteBufferArray(StringBufferArray *Array);

        // Returns a pointer to the selected element of the array
        StringBuffer *StringBufferGetElemenetAt(
            StringBufferArray *Array, 
            uint32_t ElementIndex
        );

        // Inserts a line at given index. Pushes all other lines one place forward.
        void InsertLine(
            StringBufferArray *Array, 
            uint32_t ElementIndex, 
            char *Str
        );

        // Removes a line at given index. Pulls all other lines one place backwards
        void RemoveLine(StringBufferArray *Array, uint32_t ElementIndex);

        // Removes a line at a given index and copies the string that was inside it to OutString parameter. (Can be null)
        void RemoveLineEx(StringBufferArray *Array, uint32_t ElementIndex, char *OutStr);

        // Returns the longest line length
        uint32_t MaxLineLength(StringBufferArray *Array);


    //-----MacroFunctions-----//

        #define PrintToBuffer(Buffer, str) \
            AppendBuffer(Buffer, str)

        #define ClearLineBuffer(Buffer) \
            PrintToBuffer(Buffer, "\x1b[K")

    
#define BUFFER_H
#endif