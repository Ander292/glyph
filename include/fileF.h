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
    void ReturnFileName(wchar *FullPath, wchar *OutFileName);

    // Translates an UTF-8 or ansi string into an UTF-16 string
    StringBuffer TranslateToUtf16Ex(char *Src);

    // Translates an UTF-16 string into an UTF-8 string
    StringBufferA TranslateToUtf8Ex(wchar *Src);

    //------Macros------//

        /* 
            wchar *Dest, 
            int DestSize, 
            char *Src,
            int SrcSize // Can be -1 if the string is null terminated
        */
        #define TranslateToUtf16(DestWide, DestSize, SrcMulti, SrcSize) \
            !MultiByteToWideChar(CP_UTF8, 0, (SrcMulti), (SrcSize), (DestWide), (DestSize))

        // Gets how large would the UTF8 string be in UTF16
        #define GetConvertedSize16(Src) \
            MultiByteToWideChar(CP_UTF8, 0, (Src), -1, NULL, 0)

        /* 
            char *Dest, 
            int DestSize, 
            wchar *Src,
            int SrcSize // Can be -1 if the string is null terminated
        */
        #define TranslateToUtf8(DestMulti, DestSize, SrcWide, SrcSize) \
            !WideCharToMultiByte(CP_UTF8, 0, (SrcWide), (SrcSize), (DestMulti), (DestSize), NULL, NULL);

        // Gets how large would the UTF16 string be in UTF8
        #define GetConvertedSize8(Src) \
            WideCharToMultiByte(CP_UTF8, 0, (Src), -1, NULL, 0, NULL, NULL)
#endif