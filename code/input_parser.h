#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

#include "types.h"
#include "res/command_types.h"
#include "system/io.h"

#define CTRL_KEY(k) ((k) & 0x1f)
#define UNCTRL_KEY(k) ((k) | 0x60)

typedef enum parser_result_type{
    RESULT_TYPE_COMMAND,
    RESULT_TYPE_CHAR
} parser_result_type;

typedef struct parser_result{
    parser_result_type type;
    union{
        command_key command;
        wchar32 character;
    };
} parser_result;

parser_result parser_ParseInput(read_result rr);

#endif /* INPUT_PARSER_H */
