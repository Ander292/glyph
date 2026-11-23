#include "../system/io.h"
#include "string/string_buffer.h"

int main(int argc, char *argv[], char *envp[]){  
    term_PrepareConsole();

    string_a *str = StringCreateHeapA(64);
    string_u32 *str32 = StringCreateHeapU32(64);

    for(;;){
        io_SetCursorPossition(0, 0);
        read_result r = io_ReadConsoleInput();
        if(r.bytes[0] == '\r') break;
        if(r.type == TYPE_ANSI || r.type == TYPE_UTF32) 
            StringAppendEndU32(str32, r.bytes, r.size);
        
        TranslateToUtf8(str, str32);
        PrintA(ESC_CLEAR_LINE);
        PrintBufferA(*str);
    }

    return 0;
}