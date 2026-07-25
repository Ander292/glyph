#ifndef UTF8_H
#define UTF8_H

typedef struct string{
    char *data;         /* The string itself */
    char *byteCount;    /* The sizes in bytes of each character */
    int len;            /* The length of the string in bytes */
    int byteLen;        /* The ammount of codepoints currently in data */
    int maxSize;        /* The size of 'byteCount' array. The 'data' array is always 4 times larger than byte size */
} string;

string stringCreate(int size);
void doubleSize(string *str);
void stringAppendEnd(string *str, const char *c, int size);
void clearBuffer(string *str);

#define PrintBuffer(buffer) writeOutput((buffer).data, (buffer).byteLen)
#define WriteToBuffer(buffer, str) stringAppendEnd((buffer), (str), strlen(str))
#endif