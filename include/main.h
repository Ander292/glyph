#ifndef MAIN_H
#define MAIN_H

#define GLOBAL_FLAGS E.Flags

#define HAS_FLAG(f) ((GLOBAL_FLAGS) & (f))
#define SET_FLAG(f) ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) | (f))
#define UNSET_FLAG(f) ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) & ~(f))
#define TOGGLE_FLAG(f) ((GLOBAL_FLAGS) = (GLOBAL_FLAGS) ^ (f))

#define FLAG_RUNNING        (1 << 31)
#define FLAG_SHOWNUMBERS    (1)
#define FLAG_SHOWHEADER     (2)

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