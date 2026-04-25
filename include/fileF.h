#ifndef FILEF_H
#define FILEF_H
    #include <windows.h>
    #include "buffer.h"

#if defined FILE_C || 1
    #include "bufferANSI.h"
#endif

    typedef struct {
        //int BufferIndex;   // index in the StringBufferArray
        int Offset;         // byte offset where the partial line ends
        int Reserved;       // optional, for future use
        uint8_t Shift;      //If the current line is finished
    } LineContinuationInfo;

    // Reads PortionSize bytes from hFile
    // Copies those bytes into data pointer
    void FileReadPortionS(
        HANDLE hFile, 
        DWORD PortionSize, 
        StringBufferArray *StrArray
    );

    // Separetes a file name and puts it into OutFileName
    void ReturnFileName(wchar_t* FullPath, wchar_t* OutFileName);

#endif