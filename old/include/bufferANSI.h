#ifndef BUFFER_ANSI_H
#define BUFFER_ANSI_H

    #define WINDOWS
    #include <windows.h>
    #include <stringF.h>

    //-----StructDefinitions-----//
    
        typedef struct{
            char *Memory;
            int Length;
        } StringBufferA;

    //-----Global Variables-----//
        #if defined WINDOWS
            extern HANDLE hHeap;
            extern HANDLE hStdout;
        #elif defined LINUX

        #endif
    //-----FunctionDefinitions-----//

        int StringLengthA(char* ptr);

        void StringConcatA(char *Destination, char *Source);

        void MemoryCopyA(char *Destination, char *Source, int Size);

        void UintToStringA(int n, char *pStr, int digits);

        int StringCompareA(char *Str1, char *Str2);

        // Creates a back buffer
        StringBufferA CreateBufferA(int size);

        // Doubles the size of the back buffer
        void DoubleSizeA(StringBufferA *Buffer);

        // Delets the back buffer
        void DeleteBufferA(StringBufferA *Buffer);

        // Zeroes the buffer to clear its content
        void ZeroBufferA(StringBufferA *Buffer);

        // Zeroes the buffer starting at a certain possition
        void ZeroBufferExA(StringBufferA *Buffer, int StartIndex);

        // Append to the end of the buffer (at the termination character)
        void AppendBufferA(StringBufferA *Buffer, char *Str);

        // Apped to the end of the buffer (maximum of MaxLength characters)
        uint32_t AppendBufferExA(
            StringBufferA *Buffer,
            char *Str,
            int MaxLength,
            int Offset
        );

    //-----MacroFunctions-----//

        #define PrintToBufferA(Buffer, str) \
            AppendBufferA(Buffer, str)

        #define ClearLineBufferA(Buffer) \
            PrintToBufferA(Buffer, "\x1b[K")


#endif