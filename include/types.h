#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define ATR_FALLTROUGHT __attribute__((fallthrough));

typedef int8_t      int8;
typedef int16_t     int16;
typedef int32_t     int32;
typedef int64_t     int64;

typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef uint64_t    uint64;

typedef float       real32;
typedef double      real64;

#define MIN_VAL(a, b) (a < b ? a : b)
#define MAX_VAL(a, b) (a > b ? a : b)

static inline uint32 powerOfTwoRoundUp(uint32 num){
    if(num == 1) return 2;

    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num++;

    return num;
}

#endif