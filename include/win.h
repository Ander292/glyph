#ifndef WIN_H

    #include <conio.h>
    #include <windows.h>
    #include "buffer.h"

    //-----ExternVariables-----//
        
        extern 


    //-----MacroFunctions-----//


    //-----PreprocessorDefines-----//

        #define UP_ARROW    (char)0x01  //0000 0001
        #define DOWN_ARROW  (char)0x02  //0000 0010
        #define LEFT_ARROW  (char)0x04  //0000 0100
        #define RIGHT_ARROW (char)0x08  //0000 1000

        #define CTRL_UP     (char)0xfe  //1111 1110
        #define CTRL_DOWN   (char)0xfd  //1111 1101
        #define CTRL_LEFT   (char)0xfb  //1111 1011
        #define CTRL_RIGHT  (char)0xf7  //1111 0111

        #define PAGE_DOWN   (char)0x10  //0001 0000
        #define PAGE_UP     (char)0x20  //0010 0000
        #define HOME_KEY    (char)0x40  //0100 0000
        #define END_KEY     (char)0x80  //1000 0000

        #define DELETE_KEY  (char)0x03  //0000 0011
        #define INSERT_KEY  (char)0x05  //0000 0101

        #define CTRL_DELETE (char)0x06  //0000 0110


#define WIN_H
#endif