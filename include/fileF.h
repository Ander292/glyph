#ifndef FILEF_H
    #include <windows.h>
    #include "buffer.h"

    typedef struct {
        int BufferIndex;   // index in the StringBufferArray
        int Offset;        // byte offset where the partial line ends
        int Reserved;      // optional, for future use
    } LineContinuationInfo;

    // Reads PortionSize bytes from hFile
    // Copies those bytes into data pointer
    void FileReadPortionS(
        HANDLE hFile, 
        DWORD PortionSize, 
        StringBufferArray *StrArray
    );

    // Separetes a file name and puts it into OutFileName
    void ReturnFileName(char *FullPath, char *OutFileName);

#define FILEF_H
#endif