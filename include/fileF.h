#ifndef FILEF_H
    #include <windows.h>
    #include "buffer.h"

    typedef struct {
        uint32_t BufferIndex;   // index in the StringBufferArray
        uint32_t Offset;        // byte offset where the partial line ends
        uint32_t Reserved;      // optional, for future use
    } LineContinuationInfo;

    // Reads PortionSize bytes from hFile
    // Copies those bytes into data pointer
    void FileReadPortionS(
        HANDLE hFile, 
        uint32_t PortionSize, 
        StringBufferArray *StrArray
    );

    // Separetes a file name and puts it into OutFileName
    void ReturnFileName(char *FullPath, char *OutFileName);

#define FILEF_H
#endif