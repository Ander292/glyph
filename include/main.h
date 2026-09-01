#ifndef MAIN_H
#define MAIN_H


#define TAB_SPACE_SIZE 4

typedef struct editor_message{
    char Data[256];
    time_t PostTime;
    time_t Duration;
} editor_message;

#define FILE_PATH_LENGTH 512
#define ROW_OVERHIGHLIGHT_COUNT 40

#define DEFAULT_CURSOR_MAXIMUM_X INT_MAX
#define DEFAULT_CURSOR_MAXIMUM_Y INT_MAX

#define GLOBAL_STRUCT_NAME E

#define postEditorMessage(duration, text, ...) messageCreate(&GLOBAL_STRUCT_NAME.Message, duration, text, ##__VA_ARGS__)
#define clearEditorMessage() messageCreate(&GLOBAL_STRUCT_NAME.Message, 0, "")

/**
 * FLAGS structure (bit by bit):
 *  global    file    altbuff  editor 
 * RFxxxxxx xxxheern Aaaaaaaa fSTODIHN
 * ┃┃          ┃┃┃┃┃ ┃        ┃┃┃┃┃┃┃┃
 * ┃┃          ┃┃┃┃┃ ┃        ┃┃┃┃┃┃┃┗> SHOWNUMBERS flag
 * ┃┃          ┃┃┃┃┃ ┃        ┃┃┃┃┃┃┗━> SHOWHEADER flag
 * ┃┃          ┃┃┃┃┃ ┃        ┃┃┃┃┃┗━━> INSERT_MODE flag (if character inserts cause shifting of others after it)
 * ┃┃          ┃┃┃┃┃ ┃        ┃┃┃┃┗━━━> DIRTY flag (set if the file has been edited since last save/load
 * ┃┃          ┃┃┃┃┃ ┃        ┃┃┃┗━━━━> READONLY flag (if true then insertion into the rows is prohibited)
 * ┃┃          ┃┃┃┃┃ ┃        ┃┃┗━━━━━> TO_RENDER flag (will not rerender the scren if its zero)
 * ┃┃          ┃┃┃┃┃ ┃        ┃┗━━━━━━> SYNTAX flag (will highlight if set)
 * ┃┃          ┃┃┃┃┃ ┃        ┗━━━━━━━> TO_FORMAT flag (will format syntax before next render)
 * ┃┃          ┃┃┃┃┃ ┗━━━━━━━━━━━━━━━━> ALTVIEW flag (7 bytes to its right 
 * ┃┃          ┃┃┃┃┃                    are used for alt buffer number. Extracted using GET_ALT_BUFFERID())
 * ┃┃          ┃┃┃┃┗━━━━━━━━━━━━━━━━━━> NEWLINE_ENTER flag (\n is interpreted as enter used when reading LF files)
 * ┃┃          ┃┃┃┗━━━━━━━━━━━━━━━━━━━> UNUSED
 * ┃┃          ┃┗┻━━━━━━━━━━━━━━━━━━━━> FILE_FORMAT
 * ┃┃          ┗━━━━━━━━━━━━━━━━━━━━━━> UNUSED
 * ┃┃
 * ┃┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━> FORCE flag (will not check file type on reload)
 * ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━> RUNNING flag
 */


#define GLOBAL_FLAGS GLOBAL_STRUCT_NAME.Flags

#define DEFAULT_SAVE_SOURCE_ADDRESS (&GLOBAL_STRUCT_NAME.MainBuffer)

#define HAS_FLAG(f)             ((GLOBAL_FLAGS) & (f))
#define SET_FLAG(f)             ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) | (f))
#define UNSET_FLAG(f)           ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) & ~(f))
#define CLEAR_FLAG              UNSET_FLAG
#define TOGGLE_FLAG(f)          ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) ^ (f))
#define GET_ALT_BUFFERID()      (((GLOBAL_FLAGS) >> ALTVIEW_SHIFT) & ALTVIEW_MASK)
#define SET_ALT_BUFFERID(f)     ((GLOBAL_FLAGS) = (((GLOBAL_FLAGS) & ~(ALTVIEW_MASK << ALTVIEW_SHIFT)) | (((f) & ALTVIEW_MASK) << ALTVIEW_SHIFT)))
#define GET_FORMAT_NUMBER()     (((GLOBAL_FLAGS) >> FORMAT_SHIFT) & FORMAT_MASK)
#define SET_FORMAT_NUMBER(f)    ((GLOBAL_FLAGS) = (((GLOBAL_FLAGS) & ~(FORMAT_MASK << FORMAT_SHIFT)) | (((f) & FORMAT_MASK) << FORMAT_SHIFT)))
#define GET_FORMAT_STRING(f)    (Formats[f])

#define FLAG_RUNNING        (1 << 31)
#define FLAG_FORCE          (1 << 30)

#define FLAG_SHOWNUMBERS    (1)
#define FLAG_SHOWHEADER     (2)
#define FLAG_INSERT_MODE    (4)
#define FLAG_DIRTY          (8)
#define FLAG_READONLY       (16)
#define FLAG_RDONLY         FLAG_READONLY
#define FLAG_RENDER         (32)
#define FLAG_SYNTAX         (64)
#define FLAG_FORMAT         (128)

#define FLAG_NEWLINE_ENTER  (1 << 16)

/* Multiple buffer flags */
#define FLAG_ALTVIEW        (1 << 15)
#define ALTVIEW_SHIFT       (8)
#define ALTVIEW_MASK        (0x7f)
#define ALT_BUFFER_COUNT    (5)
#define ALT_RESERVED_COUNT  (2)

/* File format flags */
#define FORMAT_SHIFT    (18)
#define FORMAT_MASK     (0x3)
#define FORMAT_UTF8     (0)
#define FORMAT_UTF16    (1)
#define FORMAT_BIN      (2)
#define FORMAT_UNUSED   (3)

#define IS_PRINTABLE_CHAR(c) ((c) >= 32 && c != 127)

static char *Formats[FORMAT_MASK + 1] = {
    "UTF8",
    "UTF16",
    "BIN",
    "UNUSED"
};

#endif