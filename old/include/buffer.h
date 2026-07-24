#ifndef BUFFER_H
    #include "stringF.h"
    #define WINDOWS
    #include <stddef.h>
    #include <windows.h>

    //-----StructDefinitions-----//
    
        typedef struct{
            wchar *Memory;
            int Length;
        } StringBuffer;

        typedef struct{
            StringBuffer *Data;
            size_t MemorySize;
            int NumberOfElements;
            int MaxNumberOfElements;
        } StringBufferArray;

    //-----Global Variables-----//
        #if defined WINDOWS
            extern HANDLE hHeap;
            extern HANDLE hStdout;
        #elif defined LINUX

        #endif
    //-----FunctionDefinitions-----//

        // Creates a back buffer
        StringBuffer CreateBuffer(int size);

        // Doubles the size of the back buffer
        void DoubleSize(StringBuffer *Buffer);

        // Delets the back buffer
        void DeleteBuffer(StringBuffer *Buffer);

        // Zeroes the buffer to clear its content
        void ZeroBuffer(StringBuffer *Buffer);

        // Zeroes the buffer starting at a certain possition
        void ZeroBufferEx(StringBuffer *Buffer, int StartIndex);

        // Append to the end of the buffer (at the termination character)
        void AppendBuffer(StringBuffer *Buffer, wchar *Str);

        // Apped to the end of the buffer (maximum of MaxLength characters)
        uint32_t AppendBufferEx(
            StringBuffer *Buffer,
            wchar *Str,
            int MaxLength,
            int Offset
        );

        // Creates a new array of string buffers
        StringBufferArray CreateBufferArray(int InitialMaxElements);

        // Doubles the capacity of a StringArrayBuffer 
        // Alocates sufficient space on the heap for all the new elements
        void DoubleArrayCapacity(StringBufferArray *Array);

        // Deletes the whole buffer array including its individual elements
        // and frees the heap used by them
        void DeleteBufferArray(StringBufferArray *Array);

        // Returns a pointer to the selected element of the array
        StringBuffer *StringBufferGetElemenetAt(
            StringBufferArray *Array, 
            int ElementIndex
        );

        // Inserts a line at given index. Pushes all other lines one place forward.
        void InsertLine(
            StringBufferArray *Array, 
            int ElementIndex, 
            wchar *Str
        );

        // Removes a line at given index. Pulls all other lines one place backwards
        void RemoveLine(StringBufferArray *Array, int ElementIndex);

        // Removes a line at a given index and copies the string that was inside it to OutString parameter. (Can be null)
        void RemoveLineEx(StringBufferArray *Array, int ElementIndex, wchar *OutStr);

        // Returns the longest line length
        int MaxLineLength(StringBufferArray *Array);


    //-----MacroFunctions-----//

        #define PrintToBuffer(Buffer, str) \
            AppendBuffer(Buffer, str)

        #define ClearLineBuffer(Buffer) \
            PrintToBuffer(Buffer, "\x1b[K")

    
#define BUFFER_H
#endif