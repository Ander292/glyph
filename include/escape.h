#ifndef ESCAPE_H
#define ESCAPE_H

#define TO_STR_H(str) #str
#define TO_STR(str) TO_STR_H(str)

#define ESC_SEQ "\x1b["
#define ESC(c) ESC_SEQ c

#define ESC_HIDE_CURSOR ESC("?25l")
#define ESC_SHOW_CURSOR ESC("?25h")

#define ESC_CLEAR_SCREEN ESC("2J")
#define ESC_CLEAR_LINE ESC("2K")

#define ESC_FULL_CLEAR ESC("c")

#define MOVE_TO_AUX_BUFFER  ESC("?1049h")
#define MOVE_TO_MAIN_BUFFER ESC("?1049l")
#define ENABLE_MOUSE_TRACKING ESC("?1000h")
#define DISABLE_MOUSE_TRACKING ESC("?1000l")

#define INVERTED_TEXT_COLOR ESC("7m")
#define RESET_TEXT_ATTRIBUTES ESC("m")

#define BACKGROUND_COLOR_RED ESC("41m")
#define RESET_BACKGROUND_ATTRIBUTES ESC("0m")

#define ESC_RESET_CURSOR_POSSITION ESC("0;0H")

#endif