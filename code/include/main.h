#ifndef MAIN_H
#define MAIN_H

/**
 *  READ --bytes--> PARSER --utf32 characters--> INSERT_INTO_ROWS_LIST --utf8 for screen--> WRITE
 *                       | ------commands------> EXECUTE_COMMAND
 * 
 *  READ:
 *      ReadingInputEvents --sends escape sequences--> READ --bytes--> ...
 */

#include <time.h>

typedef struct editor_message{
    char Data[256];
    time_t PostTime;
    time_t Duration;
} editor_message;

#define FILE_PATH_LENGTH 512
#define ROW_OVERHIGHLIGHT_COUNT 40

#define DEFAULT_CURSOR_MAXIMUM_X INT_MAX
#define DEFAULT_CURSOR_MAXIMUM_Y INT_MAX

#define EDITOR_STATE_COUNT 32

typedef struct global_editor_state{
    uint32 GlobalFlags;

    editor_state states[EDITOR_STATE_COUNT];

    i32 TextOffsetTop, TextOffsetLeft;
} global_editor_state;

typedef struct editor_state{
    uint32 Flags;

    int_pair Cursor; /* The possition of the cursor inside the internal buffer */
    int_pair oldCursor; /* The possitions in the main buffer, saved here during altbuffer switches */
    int_pair Offset;
    int_pair maxCursor;

    char File[FILE_PATH_LENGTH];
    string_list List;

    editor_message Message;
} editor_state;

#define GLOBAL_STRUCT_NAME E

#define messageDurationDefault 5
#define messageDurationInfinite 0
#define postEditorMessage(duration, text, ...) messageCreate(&GLOBAL_STRUCT_NAME.Message, duration, text, ##__VA_ARGS__)
#define clearEditorMessage() messageCreate(&GLOBAL_STRUCT_NAME.Message, 0, "")

#define TEST_FLAG(flags, flag) (((flags) & (flag)) != 0)
#define SET_FLAG(flags, flag) ((flags) |= (flag))
#define CLEAR_FLAG(flags, flag) ((flags) &= (~(flag)))
#define TOGGLE_FLAG(flags, flag) ((flags) = (flags) ^ (flag))

#ifdef DO_DEBUG_CHECK
#define debug_check(expr) if(expr)
#else
#define debug_check(expr) if(false)
#endif /* DO_DEBUG_CHECK */

#define IS_PRINTABLE_CHAR(c) ((c) >= 32 && c != 127)

/**
 * GLOBAL FLAGS structure (bit by bit):
 *   altbuff  global 
 *  aaaaaaaa RxxxxIHN
 *  ┃┃┃┃┃┃┃┃ ┃    ┃┃┃
 *  ┗┻┻╋┻┻┻┛ ┃    ┃┃┗> SHOWNUMBERS flag
 *     ┃     ┃    ┃┗━> SHOWHEADER flag
 *     ┃     ┃    ┗━━> INSERT_MODE flag (if character inserts cause shifting of others after it)
 *     ┃     ┗━━━━━━━> RUNNING flag
 *     ┃
 *     ┗━━━━━━━━━━━━━> Current buffer id ()
 */

#define GLOBAL_FLAGS GLOBAL_STRUCT_NAME.Flags
#define TEST_FLAG_GLOBAL(flag) TEST_FLAG(GLOBAL_FLAGS, flag)
#define SET_FLAG_GLOBAL(flag) SET_FLAG(GLOBAL_FLAGS, flag)
#define CLEAR_FLAG_GLOBAL(flag) CLEAR_FLAG(GLOBAL_FLAGS, flag)
#define TOGGLE_FLAG_GLOBAL(flag) TOGGLE_FLAG(GLOBAL_FLAGS, flag)

#define FLAG_SHOWNUMBERS    (1 << 0)
#define FLAG_SHOWHEADER     (1 << 1)
#define FLAG_INSERT         (1 << 2)
#define FLAG_RUNNING        (1 << 7)

#define ALTVIEW_SHIFT       (8)
#define ALTVIEW_MASK        (0xff)

#define GET_ALT_BUFFERID()      (((GLOBAL_FLAGS) >> ALTVIEW_SHIFT) & ALTVIEW_MASK)
#define SET_ALT_BUFFERID(f)     ((GLOBAL_FLAGS) = (((GLOBAL_FLAGS) & ~(ALTVIEW_MASK << ALTVIEW_SHIFT)) | (((f) & ALTVIEW_MASK) << ALTVIEW_SHIFT)))

/**
 * FLAGS structure (bit by bit):
 *   file    editor 
 *  xxxheern xxFfSTOD
 *     ┃┃┃┃┃   ┃┃┃┃┃┃  
 *     ┃┃┃┃┃   ┃┃┃┃┃┃  
 *     ┃┃┃┃┃   ┃┃┃┃┃┃ 
 *     ┃┃┃┃┃   ┃┃┃┃┃┃
 *     ┃┃┃┃┃   ┃┃┃┃┃┗━━━> DIRTY flag (set if the file has been edited since last save/load
 *     ┃┃┃┃┃   ┃┃┃┃┗━━━━> READONLY flag (if true then insertion into the rows is prohibited)
 *     ┃┃┃┃┃   ┃┃┃┗━━━━━> TO_RENDER flag (will not rerender the scren if its zero)
 *     ┃┃┃┃┃   ┃┃┗━━━━━━> SYNTAX flag (will highlight if set)
 *     ┃┃┃┃┃   ┃┗━━━━━━━> TO_FORMAT flag (will format syntax before next render)
 *     ┃┃┃┃┃   ┗━━━━━━━━> FORCE flag (will not check file type on reload)
 *     ┃┃┃┃┃
 *     ┃┃┃┃┗━━━━━━━━━━━━━━━━━━> NEWLINE_ENTER flag (\n is interpreted as enter. Used when reading LF files)
 *     ┃┃┃┗━━━━━━━━━━━━━━━━━━━> UNUSED
 *     ┃┗┻━━━━━━━━━━━━━━━━━━━━> FILE_FORMAT
 *     ┗━━━━━━━━━━━━━━━━━━━━━━> UNUSED
 */

/* File format flags */
#define FORMAT_SHIFT    (10)
#define FORMAT_MASK     (0x3)
#define FORMAT_UTF8     (0)
#define FORMAT_UTF16    (1)
#define FORMAT_BIN      (2)
#define FORMAT_UNUSED   (3)

#define GET_FORMAT_NUMBER()     (((GLOBAL_FLAGS) >> FORMAT_SHIFT) & FORMAT_MASK)
#define SET_FORMAT_NUMBER(f)    ((GLOBAL_FLAGS) = (((GLOBAL_FLAGS) & ~(FORMAT_MASK << FORMAT_SHIFT)) | (((f) & FORMAT_MASK) << FORMAT_SHIFT)))
#define GET_FORMAT_STRING(f)    (Formats[f])

#define FLAG_DIRTY          (1 << 0)
#define FLAG_READONLY       (1 << 1)
#define FLAG_RDONLY         FLAG_READONLY
#define FLAG_RENDER         (1 << 2)
#define FLAG_SYNTAX         (1 << 3)
#define FLAG_FORMAT         (1 << 4)
#define FLAG_FORCE          (1 << 5)

#define FLAG_NEWLINE_ENTER  (1 << 8)


static char *Formats[FORMAT_MASK + 1] = {
    "UTF8",
    "UTF16",
    "BIN",
    "UNUSED"
};

static char *EndLine[] = {
    "CRLF", // \r\n
    "LF"    // \n only
};

#endif /* MAIN_H */
