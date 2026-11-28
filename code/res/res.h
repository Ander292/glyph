#ifndef RES_H
#define RES_H

#include "types.h"

extern const char *const token_escape[];
extern const u32 token_escape_len;

extern const char *const keyword1_a[];
extern const char *const keyword2_a[];
extern const char *const preprocesorKeyword_a[];

extern const u32 keyword1_a_count;
extern const u32 keyword2_a_count;
extern const u32 preprocesorKeyword_a_count;

extern const char32_t *const keyword1_u32[];
extern const char32_t *const keyword2_u32[];
extern const char32_t *const preprocesorKeyword_u32[];

extern const u32 keyword1_u32_count;
extern const u32 keyword2_u32_count;
extern const u32 preprocesorKeyword_u32_count;

extern const char * const Formats[];
extern const char * const EndLine[];
extern const char const helpFull[];
extern const u64 helpFullLength;

#endif