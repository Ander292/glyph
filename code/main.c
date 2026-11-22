#include "../system/io.h"
#include "string/string_buffer.h"
#include <stdio.h>

#define TEST_REFR \
    io_SetCursorPossition(0, 0); \
    PrintA(ESC_CLEAR_LINE); \
    PrintBufferA(*str); \
    io_ReadConsoleInput();

int main(int argc, char *argv[], char *envp[]){  
    term_PrepareConsole();

    string_a *str = StringCreateHeapA(64);

#if 0
    for(;;){
        io_SetCursorPossition(0, 0);
        read_result r = io_ReadConsoleInput();
        if(r.bytes[0] == '\r') break;
        if(r.type == TYPE_ANSI) StringAppendEndA(&str, r.bytes, 1);
        
        PrintA(ESC_CLEAR_LINE);
        PrintBufferA(str);
    }
#endif

    char *bla = "Indifandozerapanzero";
    int blalen = 20;
    
    StringAppendEndA(str, bla, blalen);
    TEST_REFR

    InsertCharAtPosA(str, 'H', 67, INS_MODE_OVERWRITE | INS_MODE_INCREASE_SIZE);
    TEST_REFR

    DeleteCharFromPosA(str, 14);
    TEST_REFR

    // TerminateStringOnPos(str, 6);
    // TEST_REFR

    string_a *old = str;
    str = CreateCopyA(old, 4, 1);
    TEST_REFR

    StringFreeHeapA(old);
    StringFreeHeapA(str);

    return 0;
}