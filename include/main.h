#ifndef MAIN_H
#define MAIN_H

#define ATR_FALLTROUGHT __attribute__((fallthrough));
#define TAB_SPACE_SIZE 4

typedef struct editor_message{
    char Data[256];
    time_t PostTime;
    time_t Duration;
} editor_message;

#define FILE_PATH_LENGTH 512

#define GLOBAL_STRUCT_NAME E

#define postEditorMessage(duration, text, ...) messageCreate(&GLOBAL_STRUCT_NAME.Message, duration, text, ##__VA_ARGS__)
#define clearEditorMessage() messageCreate(&GLOBAL_STRUCT_NAME.Message, 0, "")
/**
 * FLAGS structure (bit by bit):
 * 
 * Rxxxxxxx xxxxxxrn Aaaaaaaa xxTODIHN
 * ┃              ┃┃ ┃          ┃┃┃┃┃┃
 * ┃              ┃┃ ┃          ┃┃┃┃┃┗> SHOWNUMBERS flag
 * ┃              ┃┃ ┃          ┃┃┃┃┗━> SHOWHEADER flag
 * ┃              ┃┃ ┃          ┃┃┃┗━━> INSERT_MODE flag (if character inserts cause shifting of others after it)
 * ┃              ┃┃ ┃          ┃┃┗━━━> DIRTY flag (set if the file has been edited since last save/load
 * ┃              ┃┃ ┃          ┃┗━━━━> READONLY flag (if true then insertion into the rows is prohibited)
 * ┃              ┃┃ ┃          ┗━━━━━> TO_RENDER flag (will not rerender the scren if its zero)
 * ┃              ┃┃ ┗━━━━━━━━━━━━━━━━> ALTVIEW flag (7 bytes to its right 
 * ┃              ┃┃                    are used for alt buffer number. Extracted using GET_ALT_BUFFERID())
 * ┃              ┃┗━━━━━━━━━━━━━━━━━━> NEWLINE_ENTER flag (\n is interpreted as enter used for reading linux files)
 * ┃              ┗━━━━━━━━━━━━━━━━━━━> RETURN_ENTER flag (unused)
 * ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━> RUNNING flag
 */


#define GLOBAL_FLAGS GLOBAL_STRUCT_NAME.Flags

#define DEFAULT_SAVE_SOURCE_ADDRESS (&GLOBAL_STRUCT_NAME.MainBuffer)

#define HAS_FLAG(f)         ((GLOBAL_FLAGS) & (f))
#define SET_FLAG(f)         ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) | (f))
#define UNSET_FLAG(f)       ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) & ~(f))
#define CLEAR_FLAG UNSET_FLAG
#define TOGGLE_FLAG(f)      ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) ^ (f))
#define GET_ALT_BUFFERID()  (((GLOBAL_FLAGS) >> ALTVIEW_SHIFT) & ALTVIEW_MASK)
#define SET_ALT_BUFFERID(f) ((GLOBAL_FLAGS) = (((GLOBAL_FLAGS) & ~(ALTVIEW_MASK << ALTVIEW_SHIFT)) | (((f) & ALTVIEW_MASK) << ALTVIEW_SHIFT)))

#define FLAG_RUNNING        (1 << 31)
#define FLAG_SHOWNUMBERS    (1)
#define FLAG_SHOWHEADER     (2)
#define FLAG_INSERT_MODE    (4)
#define FLAG_DIRTY          (8)
#define FLAG_READONLY       (16)
#define FLAG_RENDER         (32)

#define FLAG_NEWLINE_ENTER  (1 << 16)

/* Multiple buffer flags */
#define FLAG_ALTVIEW        (1 << 15)
#define ALTVIEW_SHIFT       (8)
#define ALTVIEW_MASK        (0x7f)
#define ALT_BUFFER_COUNT    (5)

#endif