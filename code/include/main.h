/**
 *  READ --bytes--> PARSER --utf32 characters--> INSERT_INTO_ROWS_LIST --utf8 for screen--> WRITE
 *                       | ------commands------> EXECUTE_COMMAND
 * 
 *  READ:
 *      ReadingInputEvents --sends escape sequences--> READ --bytes--> ...
 */

#define TEST_FLAG(flags, flag) (((flags) & (flag)) != 0)
#define SET_FLAG(flags, flag) ((flags) |= (flag))
#define CLEAR_FLAG(flags, flag) ((flags) &= (~(flag)))

#ifdef DO_DEBUG_CHECK
#define debug_check(expr) if(expr)
#else
#define debug_check(expr) if(false)
#endif