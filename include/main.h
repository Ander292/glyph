#ifndef MAIN_H
#define MAIN_H

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
 * Rxxxxxxx xxxxxxxx Aaaaaaaa xxxxDIHN
 * ┃                 ┃            ┃┃┃┃
 * ┃                 ┃            ┃┃┃┗> SHOWNUMBERS flag
 * ┃                 ┃            ┃┃┗━> SHOWHEADER flag
 * ┃                 ┃            ┃┗━━> INSERT_MODE flag (if character inserts cause shifting of others after it)
 * ┃                 ┃            ┗━━━> DIRTY flag (set if the file has been edited since last save/load
 * ┃                 ┗━━━━━━━━━━━━━━━━> ALTVIEW flag (7 bytes to its right 
 * ┃                                    are used for alt buffer number. Extracted using GET_ALT_BUFFERID())
 * ┃                                    
 * ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━> RUNNING flag
 */


#define GLOBAL_FLAGS GLOBAL_STRUCT_NAME.Flags

#define HAS_FLAG(f)         ((GLOBAL_FLAGS) & (f))
#define SET_FLAG(f)         ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) | (f))
#define UNSET_FLAG(f)       ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) & ~(f))
#define TOGGLE_FLAG(f)      ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) ^ (f))
#define GET_ALT_BUFFERID()  (((GLOBAL_FLAGS) >> ALTVIEW_SHIFT) & ALTVIEW_MASK)
#define SET_ALT_BUFFERID(f) (((GLOBAL_FLAGS) & (ALTVIEW_MASK << ALTVIEW_SHIFT)) | (f))

#define FLAG_RUNNING        (1 << 31)
#define FLAG_SHOWNUMBERS    (1)
#define FLAG_SHOWHEADER     (2)
#define FLAG_INSERT_MODE    (4)
#define FLAG_DIRTY          (8)

/* Multiple buffer flags */
#define FLAG_ALTVIEW        (1 << 15)
#define ALTVIEW_SHIFT       (8)
#define ALTVIEW_MASK        (0x7f)
#define ALT_BUFFER_COUNT    (1)

#endif