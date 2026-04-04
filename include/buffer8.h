#ifndef BUFFER_UTF8_H
#define BUFFER_UTF8_H

    #include "utf8F.h"
    #include "stringF.h" // For evaluating literal byte size
    #define WINDOWS
    #include <windows.h>

    //-----StructDefinitions-----//

        typedef struct{
            char *Memory;
            int Size;
            int MaxSize;
            int Length;
            uint8_t *Inf;
        } StringBuffer8;

    //-----Global Variables-----//
        
        #if defined WINDOWS
            extern HANDLE hHeap;
            extern HANDLE hStdout;
        #elif defined LINUX

        #endif


    //-----FunctionDefinitions-----//

        StringBuffer8 CreateBuffer8(int size);

        void DoubleSize8(StringBuffer8 *Buffer);

        void DeleteBuffer8(StringBuffer8 *Buffer);

        void ZeroBuffer8(StringBuffer8 *Buffer);

        void ZeroBuffer8Ex(StringBuffer8 *Buffer, int StartIndex);

        void AppendBuffer8(StringBuffer8 *Buffer, char *Str);

        uint32_t AppendBuffer8Ex(StringBuffer8 *Buffer, char *Str, int MaxLength, int Offset);

        /*
        StringBufferArray8 CreateBufferArray8(int InitialMaxElements);

        void DoubleArrayCapacity8(StringBufferArray8 *Array);

        void DeleteBufferArray8(StringBufferArray8 *Array);

        StringBuffer8 *StringBufferGetElemenetAt8(StringBufferArray8 *Array, int ElementIndex);

        void InsertLine8(StringBufferArray8 *Array, int ElementIndex, char *Str);

        void RemoveLine8(StringBufferArray8 *Array, int ElementIndex);

        void RemoveLine8Ex(StringBufferArray8 *Array, int ElementIndex, char *OutStr);

        int MaxLineLength8(StringBufferArray8 *Array);
        */

    //-----MacroFunctions-----//

        #define PrintToBuffer8(Buffer, str) \
            AppendBuffer8(Buffer, str)

        #define ClearLineBuffer8(Buffer) \
            PrintToBuffer8(Buffer, "\x1b[K")

#endif