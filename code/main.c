#include "../system/io.h"
#include "string/string_buffer.h"
#include <stdio.h>

#define TEST_REFR \
    io_SetCursorPossition(0, 0); \
    PrintA(ESC_CLEAR_LINE); \
    PrintBufferA(*str); \
    io_ReadConsoleInput();

#define TEST_REFR2 \
    TranslateToUtf8(str, str32); \
    io_SetCursorPossition(0, 0); \
    PrintA(ESC_CLEAR_LINE); \
    PrintBufferA(*str); \
    io_ReadConsoleInput();

int main(int argc, char *argv[], char *envp[]){  
    term_PrepareConsole();

    string_a *str = StringCreateHeapA(64);
    string_u32 *str32 = StringCreateHeapU32(64);

#if 1
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
#endif

#if 0
    char *bla = "Indifandozerapanzero";
    int blalen = 20;
    
    StringAppendEndA(str, bla, blalen);
    TEST_REFR

    InsertCharAtPosA(str, 'H', 67, INS_MODE_OVERWRITE | INS_MODE_INCREASE_SIZE);
    TEST_REFR

    DeleteCharFromPosA(str, 14);
    TEST_REFR

    // TerminateStringOnPosA(str, 6);
    // TEST_REFR

    string_a *old = str;
    str = CreateCopyA(old, 4, 1);
    TEST_REFR

    StringFreeHeapA(old);
    StringFreeHeapA(str);
#endif
#if 0
    const char *bla = "1234ččč5678ććć9f";
    int blalen = 22;

    string_u32 *str32 = BufferCreateFromStringU32(bla, blalen);
    TEST_REFR2

    StringAppendEndU32(str32, bla, blalen);
    TEST_REFR2

    // wchar32 wc = GetCharAtPosU32(str32, 12);

    // OverrunBufferWithA(str, wc.bytes, charGetByteCount(*wc.bytes));
    // TEST_REFR

    InsertCharAtPosU32(str32, WCHAR32(U' '), 12, 0);
    TEST_REFR2

    InsertCharAtPosU32(str32, WCHAR32(U'F'), 12, INS_MODE_OVERWRITE);
    TEST_REFR2

    InsertCharAtPosU32(str32, WCHAR32(U'R'), 72, INS_MODE_OVERWRITE | INS_MODE_INCREASE_SIZE);
    TEST_REFR2
    InsertCharAtPosU32(str32, WCHAR32(U'R'), 73, INS_MODE_INCREASE_SIZE);
    TEST_REFR2

    InsertCharAtPosU32(str32, WCHAR32(0x88D1), 72, INS_MODE_OVERWRITE | INS_MODE_INCREASE_SIZE);
    TEST_REFR2

#endif
    return 0;
}