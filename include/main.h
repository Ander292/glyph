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

#if 0
#define ESCAPE_UPARROW      (1)
#define ESCAPE_DOWNARROW    (2)
#define ESCAPE_RIGHTARROW   (4)
#define ESCAPE_LEFTARROW    (8)
#endif

#endif