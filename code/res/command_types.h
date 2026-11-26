#ifndef COMMAND_TYPES_H
#define COMMAND_TYPES_H

#include <memory.h>

typedef enum command_key{
    KEY_CTRL_Q,
    KEY_CTRL_E,
    KEY_CTRL_S,
    KEY_CTRL_O,

    KEY_CTRL_N,
    KEY_CTRL_T,
    KEY_CTRL_P,
    
    KEY_CTRL_G,
    KEY_CTRL_F,
    KEY_CTRL_U,
    KEY_CTRL_R,

    KEY_CTRL_W,
    KEY_CTRL_H, // KEY_BACKSPACE, 127

    KEY_DELETE,
    KEY_NEWLINE,
    KEY_CARRIAGE,
    KEY_TAB,
    KEY_BACKSPACE, //127
    KEY_CTRL_BACKSPACE,
    KEY_ALT_BACKSPACE,

    KEY_UP_ARROW,
    KEY_LEFT_ARROW,
    KEY_RIGHT_ARROW,
    KEY_DOWN_ARROW,

    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_HOME,
    KEY_END,

    KEY_INSERT,
    KEY_CTRL_DELETE,
    KEY_ALT_DELETE,    

    KEY_CTRL_UP_ARROW,
    KEY_CTRL_DOWN_ARROW,
    KEY_CTRL_RIGHT_ARROW,
    KEY_CTRL_LEFT_ARROW,

    
    KEY_ALT_UP_ARROW,
    KEY_ALT_DOWN_ARROW,
    KEY_ALT_RIGHT_ARROW,
    KEY_ALT_LEFT_ARROW,

    COUNT_OF_KEYBINDS
} command_key;

typedef enum command_type{
    COMMAND_NEUTRAL,
    COMMAND_QUIT = 1,
    COMMAND_OPEN_HELP,
    COMMAND_SAVE,
    COMMAND_LOAD,

    COMMAND_SHOWNUMBER_TOGGLE,
    COMMAND_SHOWHEADER_TOGGLE,
    COMMAND_SYNTAX,
    COMMAND_INSERT,

    COMMAND_GOTO_LINE,
    COMMAND_FIND,

    COMMAND_SWITCH_CHARSET,
    COMMAND_TOGGLE_READONLY,

    COMMAND_BACKSPACE,
    COMMAND_REMOVE_WORD_LEFT,
    COMMAND_DELETE,
    COMMAND_REMOVE_WORLD_RIGHT,

    //COMMAND_NEWLINE,
    COMMAND_NEWLINE,
    COMMAND_TAB,

    COMMAND_UP,
    COMMAND_LEFT,
    COMMAND_RIGHT,
    COMMAND_DOWN,
    COMMAND_PAGEUP,
    COMMAND_PAGEDOWN,
    COMMAND_HOME,
    COMMAND_END,

    COMMAND_OFFSET_UP,
    COMMAND_OFFSET_DOWN,
    COMMAND_SKIP_WORD_RIGHT,
    COMMAND_SKIP_WORD_LEFT,

    
    COMMAND_MOVE_ROW_UP,
    COMMAND_MOVE_ROW_DOWN,
    COMMAND_NEXT_BUFFER,
    COMMAND_PREV_BUFFER
} command_type;

extern const command_type default_keybind[COUNT_OF_KEYBINDS];
extern command_type keybind[COUNT_OF_KEYBINDS];

static inline void resetKeybinds(){
    memcpy(keybind, default_keybind, COUNT_OF_KEYBINDS * sizeof(command_type));
}

static inline void resetKeybind(command_key key){
    keybind[key] = default_keybind[key];
}

static inline command_type translateCommandKey(command_key key){
    if(key >= COUNT_OF_KEYBINDS) return (command_type)0;
    return keybind[key];
}

static inline void assignKeybind(command_key key, command_type command){
    if(key >= COUNT_OF_KEYBINDS) return;
    keybind[key] = command;
}

#define COMMAND_OPEN COMMAND_LOAD
#define COMMAND_SEARCH COMMAND_FIND

#endif /* COMMAND_TYPES_H */
