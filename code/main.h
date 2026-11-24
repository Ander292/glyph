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
#include "string/string_buffer.h"
#include "string/string_list.h"

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

typedef struct editor_state{
    uint32 Flags;

    int_pair Cursor; /* The possition of the cursor inside the internal buffer */
    int_pair oldCursor; /* The possitions in the main buffer, saved here during altbuffer switches */
    int_pair Offset;

    char File[FILE_PATH_LENGTH];
    string_list List;
} editor_state;

typedef struct global_editor_state{
    uint32 Flags;

    editor_state states[EDITOR_STATE_COUNT];
    editor_message Message;

    int_pair maxCursor;
    i32 TextOffsetTop, TextOffsetLeft;
} global_editor_state;

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


#define GLOBAL_FLAGS GLOBAL_STRUCT_NAME.Flags

#define CURRENT_LOCAL_STRUCT GLOBAL_STRUCT_NAME.states[GET_EDITOR_ID()]
#define CURRENT_LOCAL_FLAGS CURRENT_LOCAL_STRUCT.Flags

#define TEST_FLAG_GLOBAL(flag) TEST_FLAG(GLOBAL_FLAGS, flag)
#define SET_FLAG_GLOBAL(flag) SET_FLAG(GLOBAL_FLAGS, flag)
#define CLEAR_FLAG_GLOBAL(flag) CLEAR_FLAG(GLOBAL_FLAGS, flag)
#define TOGGLE_FLAG_GLOBAL(flag) TOGGLE_FLAG(GLOBAL_FLAGS, flag)

#define TEST_FLAG_LOCAL(flag) TEST_FLAG(CURRENT_LOCAL_FLAGS, flag)
#define SET_FLAG_LOCAL(flag) SET_FLAG(CURRENT_LOCAL_FLAGS, flag)
#define CLEAR_FLAG_LOCAL(flag) CLEAR_FLAG(CURRENT_LOCAL_FLAGS, flag)
#define TOGGLE_FLAG_LOCAL(flag) TOGGLE_FLAG(CURRENT_LOCAL_FLAGS, flag)


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
 *     ┗━━━━━━━━━━━━━> Current editor id (use to get GET_EDITOR_ID())
 */

#define GLOBAL_FLAG_SHOWNUMBERS    (1 << 0)
#define GLOBAL_FLAG_SHOWHEADER     (1 << 1)
#define GLOBAL_FLAG_INSERT         (1 << 2)
#define GLOBAL_FLAG_RUNNING        (1 << 7)

#define ALTVIEW_SHIFT       (8)
#define ALTVIEW_MASK        (0xff)

#define GET_EDITOR_ID()      (((GLOBAL_FLAGS) >> ALTVIEW_SHIFT) & ALTVIEW_MASK)
#define SET_EDITOR_ID(f)     ((GLOBAL_FLAGS) = (((GLOBAL_FLAGS) & ~(ALTVIEW_MASK << ALTVIEW_SHIFT)) | (((f) & ALTVIEW_MASK) << ALTVIEW_SHIFT)))

#define DEFAULT_GLOBAL_FLAGS (GLOBAL_FLAG_RUNNING | GLOBAL_FLAG_SHOWHEADER | GLOBAL_FLAG_SHOWNUMBERS | GLOBAL_FLAG_INSERT)

/**
 * FLAGS structure (bit by bit):
 *   file    editor 
 *  xxxheern IxFfSTOD
 *     ┃┃┃┃┃ ┃ ┃┃┃┃┃┃  
 *     ┃┃┃┃┃ ┃ ┃┃┃┃┃┃  
 *     ┃┃┃┃┃ ┃ ┃┃┃┃┃┃ 
 *     ┃┃┃┃┃ ┃ ┃┃┃┃┃┃
 *     ┃┃┃┃┃ ┃ ┃┃┃┃┃┗━━━> DIRTY flag (set if the file has been edited since last save/load
 *     ┃┃┃┃┃ ┃ ┃┃┃┃┗━━━━> READONLY flag (if true then insertion into the rows is prohibited)
 *     ┃┃┃┃┃ ┃ ┃┃┃┗━━━━━> TO_RENDER flag (will not rerender the scren if its zero)
 *     ┃┃┃┃┃ ┃ ┃┃┗━━━━━━> SYNTAX flag (will highlight if set)
 *     ┃┃┃┃┃ ┃ ┃┗━━━━━━━> TO_FORMAT flag (will format syntax before next render)
 *     ┃┃┃┃┃ ┃ ┗━━━━━━━━> FORCE flag (will not check file type on reload)
 *     ┃┃┃┃┃ ┗━━━━━━━━━━> INITIALIZED flag (set if the buffer is used)
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

#define GET_LOCAL_FORMAT_NUMBER()     (((CURRENT_LOCAL_FLAGS) >> FORMAT_SHIFT) & FORMAT_MASK)
#define SET_LOCAL_FORMAT_NUMBER(f)    ((CURRENT_LOCAL_FLAGS) = (((CURRENT_LOCAL_FLAGS) & ~(FORMAT_MASK << FORMAT_SHIFT)) | (((f) & FORMAT_MASK) << FORMAT_SHIFT)))
#define GET_FORMAT_STRING(f)    (Formats[f])

#define FLAG_DIRTY          (1 << 0)
#define FLAG_READONLY       (1 << 1)
#define FLAG_RDONLY         FLAG_READONLY
#define FLAG_RENDER         (1 << 2)
#define FLAG_SYNTAX         (1 << 3)
#define FLAG_FORMAT         (1 << 4)
#define FLAG_FORCE          (1 << 5)

#define FLAG_INITIALIZED    (1 << 7)
#define FLAG_NEWLINE_ENTER  (1 << 8)

#ifdef UNIX
#define DEFAULT_LOCAL_FLAGS (FLAG_NEWLINE_ENTER | FLAG_INITIALIZED)
#else
#define DEFAULT_LOCAL_FLAGS (FLAG_INITIALIZED)
#endif
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
