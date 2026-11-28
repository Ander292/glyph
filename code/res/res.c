#include "res.h"
#include "command_types.h"
#include "escape.h"

const char *const token_escape[] = {
    ESC_RESET_TEXT_ATTRIBUTES,
    ESC_TEXT_COLOR_BLUE,
    ESC_TEXT_COLOR_MAGENTA,
    ESC_TEXT_COLOR_YELLOW_BRIGHT,
    ESC_TEXT_COLOR_GREEN_BRIGHT,
    ESC_TEXT_COLOR_GREEN,
    ESC_TEXT_COLOR_CYAN,
    ESC_TEXT_COLOR_MAGENTA_BRIGHT,
    ESC_BACKGROUND_COLOR_GREEN_BRIGHT,
    ESC_TEXT_COLOR_RED_BRIGHT,
    ESC_BACKGROUND_COLOR_WHITE_BRIGHT,
};
const u32 token_escape_len = sizeof(token_escape) / sizeof(char*);

const char *const keyword1_a[] = {
    "while",
    "for",
    "if",
    "else",
    "switch",
    "case",
    "default",
    "return",
    "break",
    "continue"
};
const u32 keyword1_a_count = sizeof(keyword1_a) / sizeof(char*);

const char *const keyword2_a[] = {
    "int",
    "short",
    "char",
    "long",
    "unsigned",
    "signed",
    "static",
    "void",
    "float",
    "double",
    "inline",
    "auto",
    "const",

    "int8",
    "int16",
    "int32",
    "int64",
    "uint8",
    "uint16",
    "uint32",
    "uint64",

    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "size_t",

    "i8",
    "i16",
    "i32",
    "i64",
    "u8",
    "u16",
    "u32",
    "u64"
};

const u32 keyword2_a_count = sizeof(keyword2_a) / sizeof(char*);

const char *const preprocesorKeyword_a[] = {
    "#define",
    "#include",
    "#undef",
    "#if",
    "#ifdef",
    "#ifndef",
    "#else",
    "#elif",
    "#region",
    "#endregion",
    "#pragma",
    "#endif",
    "#warning",
    "#error"
};

const u32 preprocesorKeyword_a_count = sizeof(preprocesorKeyword_a) / sizeof(char*);

const char32_t *const keyword1_u32[] = {
    U"while",
    U"for",
    U"if",
    U"else",
    U"switch",
    U"case",
    U"default",
    U"return",
    U"break",
    U"continue"
};

const u32 keyword1_u32_count = sizeof(keyword1_u32) / sizeof(char32_t*);

const char32_t *const keyword2_u32[] = {
    U"int",
    U"short",
    U"char",
    U"long",
    U"unsigned",
    U"signed",
    U"static",
    U"void",
    U"float",
    U"double",
    U"inline",
    U"auto",
    U"const",

    U"int8",
    U"int16",
    U"int32",
    U"int64",
    U"uint8",
    U"uint16",
    U"uint32",
    U"uint64",

    U"int8_t",
    U"int16_t",
    U"int32_t",
    U"int64_t",
    U"uint8_t",
    U"uint16_t",
    U"uint32_t",
    U"uint64_t",
    U"size_t",

    U"i8",
    U"i16",
    U"i32",
    U"i64",
    U"u8",
    U"u16",
    U"u32",
    U"u64"
};
const u32 keyword2_u32_count = sizeof(keyword2_u32) / sizeof(char32_t*);

const char32_t *const preprocesorKeyword_u32[] = {
    U"#define",
    U"#include",
    U"#undef",
    U"#if",
    U"#ifdef",
    U"#ifndef",
    U"#else",
    U"#elif",
    U"#region",
    U"#endregion",
    U"#pragma",
    U"#endif",
    U"#warning",
    U"#error"
};
const u32 preprocesorKeyword_u32_count = sizeof(preprocesorKeyword_u32) / sizeof(char32_t*);


const char *const Formats[] = {
    "UTF8",
    "UTF16",
    "BIN",
    "UNUSED"
};

const char *const EndLine[] = {
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

const command_type const default_keybind[COUNT_OF_KEYBINDS] = {
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
    
    //[KEY_NEWLINE] = COMMAND_NEWLINE,
    [KEY_CARRIAGE] = COMMAND_NEWLINE,
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