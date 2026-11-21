#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define ATR_FALLTROUGHT __attribute__((fallthrough));
#define MIN_VAL(a, b) (a < b ? a : b)
#define MAX_VAL(a, b) (a > b ? a : b)

#define OUT
#define IN
#define VALUE_RESULT

typedef int8_t      int8,   i8;
typedef int16_t     int16,  i16;
typedef int32_t     int32,  i32;
typedef int64_t     int64,  i64;

typedef uint8_t     uint8,  u8,     byte,   bool8;
typedef uint16_t    uint16, u16,    word;
typedef uint32_t    uint32, u32,    dword,  bool32;
typedef uint64_t    uint64, u64,    qword;

typedef float       real32;
typedef double      real64;

//typedef uint16      wchar16;
//typedef uint32      wchar32;

typedef union wchar32{
    u8 bytes[4];
    u32 full;
} wchar32;

typedef union wchar16{
    u8 bytes[2];
    u16 full;
} wchar16;

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