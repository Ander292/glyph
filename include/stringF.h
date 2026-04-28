#ifndef STRINGF_H
    #include <stdint.h>

    typedef unsigned short wchar;

    //-----FunctionDefinitions-----//

        // Returns the length of the string passed as an argument
        int StringLength(wchar *ptr);

        // Copies the contents of the 2nd string to the end of the first string
        void StringConcat(wchar *Destination, wchar *Source);

        // Copies "Size" number of bytes from "Source" to "Destination"
        // Disregards the previous content of Destination
        // Destination must be a pointer to a valid address
        void MemoryCopy(wchar *Destination, wchar *Source, int Size);

        // Compares 2 strings
        int StringCompare(wchar *Str1, wchar *Str2);

        // Converts the number to a string and writes the digits in ascii code inside pStr array
        void UintToString(int n, wchar * pStr, int digits);

        // Converts a 64bit integer into an asci string
        void LongToString(int64_t n, wchar * pStr, int digits);

        // Inverts the number (ex. 1277 becomes 7721)
        int ReverseOrder(int n, int cnt);

        // Invers a signed 64bit integer
        int64_t ReverseOrder64(int64_t n, int cnt);

        // Returns the power of a number (ex. 2,3 will return 2^3 (8))
        int Power(int base, int degree);

        // Returns the power of a number (ex. 2,3 will return 2^3 (8))
        int Power64(int64_t base, int degree);

        // Counts digits of number
        int DigitCount(int n);

        // Returns the smaller number
        int SmallerInteger(int a, int b);

        // Returns the smaller unsigned integer
        uint32_t SmallerUnsigned(uint32_t a, uint32_t b);

        // Return the larger unsigned integer
        uint32_t LargerUnsigned(uint32_t a, uint32_t b);

        // Dont ask me why this exists
        uint32_t AbsoluteUnsigned(uint32_t n);

        // Converts a character to ascii code string
        void CharToAnsi(wchar C, wchar * pStr);

        // Returns the length of the string until the first newline character
        int LineLength(wchar * ptr);

        // Returns the length of the string until the first newline character
        // It will not read more than MaxLength characters
        // FoundNewline will either be set to 1 or 0 depending on if the function actually found the newline
        int LineLengthEx(wchar * ptr, int MaxLength, uint8_t *FoundNewline);

        // Counts the ammount of chars C in the string
        // If the string is invalid (not null terminated) this function will return a segmentation fault
        int CharacterCount(wchar *String, wchar C);

        void StringShiftLeft(wchar *Str, int StartOffset, int EndOffset);

        void StringShiftRight(wchar *Str, int StartOffset, int EndOffset);
        
        
        int CountForwardToBlankEx(wchar *Str, int CurrentPossition);

        int CountBackToBlankEx(wchar *Str, int CurrentPossition);

        int CountBackToWordEx(wchar * Str, int CurrentPossition);

        int CountForwardToWordEx(wchar *Str, int CurrentPossition);


        int CountForwardToWord(wchar *Str, int CurrentPossition);

        int CountBackToWord(wchar * Str, int CurrentPossition);
        
        int CountForwardToBlank(wchar *Str, int CurrentPossition);

        int CountBackToBlank(wchar *Str, int CurrentPossition);
    //-----MacroFunctions-----//
        
        // The maximum size of a string array
        // Only works on strings created inside current function
        #define MaxStringSize(str) sizeof(str) / sizeof(char)

        //Returns 1 if a character is printable and 0 if its not
        #define IsPrintable(c) ((c) > 31 && (c) < 127)

        #define StringCopy(Destination, Source) \
            MemoryCopy(Destination, Source, StringLength(Source))

        //  Adds a character to the end of the string and moves the terminating sequence into the next one
        #define CharConcat(Destination, Character) \
            { \
                int DestLength = StringLength(Destination); \
                Destination[DestLength - 1] = Character; \
                Destination[DestLength] = '\0'; \
            }

        // Adds multiple characters to the end of the string
        #define AddCharacters(Destination, Character, Count) \
            { \
                for(int i = 0; i < Count; i++){ \
                CharConcat(Destination, Character); \
            }}


    //-----Constants-----//

       

#define STRINGF_H
#endif