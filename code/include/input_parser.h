#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

#include "types.h"
#include "../system/io.h"

#define CTRL_KEY(k) ((k) & 0x1f)
#define UNCTRL_KEY(k) ((k) | 0x60)

void parser_ParseInput(read_result rr);

#endif /* INPUT_PARSER_H */
