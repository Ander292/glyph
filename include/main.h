/**
 *  READ --bytes--> PARSER --utf32 characters--> INSERT_INTO_ROWS_LIST --utf8 for screen--> WRITE
 *                       | ------commands------> EXECUTE_COMMAND
 * 
 *  READ:
 *      ReadingInputEvents --sends escape sequences--> READ --bytes--> ...
 */