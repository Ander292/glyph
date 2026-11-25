#include "res.h"
#include "command_types.h"

const char * const Formats[] = {
    "UTF8",
    "UTF16",
    "BIN",
    "UNUSED"
};

const char * const EndLine[] = {
    "CRLF", // \r\n
    "LF"    // \n only
};

const char const helpFull[] =
    "Ctrl key combinations: \r"

        "\te -> enter/leave help\r"
        "\tq -> quit\r"
        "\to -> open new file\r"
        "\ts -> save current file\r"
        "\tg -> goto line\r"

        "\th -> same as ctrl+backspace, delete whole word\r"
        
        "\tt -> show header\r"
        "\tp -> toggle syntax highlighting\r"
        "\tn -> toggle numbers\r"
        "\tu -> switch between utf16 and utf8 mode"

        //"\tr -> read-only mode\r"
;

const u64 helpFullLength = sizeof(helpFull);

const command_type default_keybind[COUNT_OF_KEYBINDS] = {
    [KEY_CTRL_Q] = COMMAND_QUIT,
    [KEY_CTRL_E] = COMMAND_OPEN_HELP,
    [KEY_CTRL_S] = COMMAND_SAVE,
    [KEY_CTRL_O] = COMMAND_LOAD,
    [KEY_CTRL_N] = COMMAND_SHOWNUMBER_TOGGLE,
    [KEY_CTRL_T] = COMMAND_SHOWHEADER_TOGGLE,
    [KEY_CTRL_P] = COMMAND_SYNTAX,
    [KEY_CTRL_G] = COMMAND_GOTO_LINE,
    [KEY_CTRL_F] = COMMAND_FIND,
    [KEY_CTRL_U] = COMMAND_SWITCH_CHARSET,
    [KEY_CTRL_R] = COMMAND_TOGGLE_READONLY,

    [KEY_CTRL_W] = COMMAND_REMOVE_WORD_LEFT,
    [KEY_CTRL_H] = COMMAND_REMOVE_WORD_LEFT,

    [KEY_BACKSPACE] = COMMAND_BACKSPACE,
    
    [KEY_NEWLINE] = COMMAND_NEWLINE,
    [KEY_CARRIAGE] = COMMAND_CARRIAGE,
    [KEY_TAB] = COMMAND_TAB,

    [KEY_UP_ARROW] = COMMAND_UP,
    [KEY_LEFT_ARROW] = COMMAND_LEFT,
    [KEY_RIGHT_ARROW] = COMMAND_RIGHT,
    [KEY_DOWN_ARROW] = COMMAND_DOWN,
    [KEY_PAGEUP] = COMMAND_PAGEUP,
    [KEY_PAGEDOWN] = COMMAND_PAGEDOWN,
    [KEY_INSERT] = COMMAND_INSERT,
    [KEY_HOME] = COMMAND_HOME,
    [KEY_END] = COMMAND_END,

    [KEY_DELETE] = COMMAND_DELETE,
    [KEY_CTRL_DELETE] = COMMAND_REMOVE_WORLD_RIGHT,
    //[KEY_ALT_DELETE] = COMMAND_ALT_DELETE,
    
    [KEY_CTRL_UP_ARROW] = COMMAND_OFFSET_UP,
    [KEY_CTRL_DOWN_ARROW] = COMMAND_OFFSET_DOWN,
    [KEY_CTRL_RIGHT_ARROW] = COMMAND_SKIP_WORD_RIGHT,
    [KEY_CTRL_LEFT_ARROW] = COMMAND_SKIP_WORD_LEFT,
    //[KEY_ALT_UP_ARROW] = COMMAND_ALT_UP,
    //[KEY_ALT_DOWN_ARROW] = COMMAND_ALT_DOWN,
    [KEY_ALT_RIGHT_ARROW] = COMMAND_NEXT_BUFFER,
    [KEY_ALT_LEFT_ARROW] = COMMAND_PREV_BUFFER
};

command_type keybind[COUNT_OF_KEYBINDS];