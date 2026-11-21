#include "../system/io.h"
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]){  
    term_PrepareConsole();

    read_result r = io_ReadConsoleInput();
    
    u32 writeResult = writeOutput(r.bytes, r.size);
    
    io_ReadConsoleInput();

    return 0;
}