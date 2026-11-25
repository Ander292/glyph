#include "res.h"

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