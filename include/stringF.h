#ifndef STRINGF_H
    #include <stdint.h>

    //-----FunctionDefinitions-----//

        // Returns the length of the string passed as an argument
        uint32_t StringLength(char* ptr);

        // Copies the contents of the 2nd string to the end of the first string
        void StringConcat(char *Destination, char *Source);

        // Copies "Size" number of bytes from "Source" to "Destination"
        // Disregards the previous content of Destination
        // Destination must be a pointer to a valid address
        void MemoryCopy(char *Destination, char *Source, uint32_t Size);

        // Converts the number to a string and writes the digits in ascii code inside pStr array
        void UintToString(uint32_t n, char *pStr, int digits);

        // Converts a 64bit integer into an asci string
        void LongToString(int64_t n, char *pStr, int digits);

        // Inverts the number (ex. 1277 becomes 7721)
        uint32_t ReverseOrder(uint32_t n, int cnt);

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
        void CharToAnsi(char c, char *pStr);

        // Returns the length of the string until the first newline character
        uint32_t LineLength(char* ptr);

        // Returns the length of the string until the first newline character
        // It will not read more than MaxLength characters
        // FoundNewline will either be set to 1 or 0 depending on if the function actually found the newline
        uint32_t LineLengthEx(char* ptr, uint32_t MaxLength, uint8_t *FoundNewline);

        // Counts the ammount of chars C in the string
        // If the string is invalid (not null terminated) this function will return a segmentation fault
        uint32_t CharacterCount(char *String, char C);

        void StringShiftLeft(char *Str, uint32_t StartOffset, uint32_t EndOffset);

        void StringShiftRight(char *Str, uint32_t StartOffset, uint32_t EndOffset);
        

        uint32_t CountForwardToBlankEx(char *Str, uint32_t CurrentPossition);

        uint32_t CountBackToBlankEx(char *Str, uint32_t CurrentPossition);

        uint32_t CountBackToWordEx(char* Str, uint32_t CurrentPossition);

        uint32_t CountForwardToWordEx(char *Str, uint32_t CurrentPossition);


        uint32_t CountForwardToWord(char *Str, uint32_t CurrentPossition);

        uint32_t CountBackToWord(char* Str, uint32_t CurrentPossition);
        
        uint32_t CountForwardToBlank(char *Str, uint32_t CurrentPossition);

        uint32_t CountBackToBlank(char *Str, uint32_t CurrentPossition);
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
                uint32_t DestLength = StringLength(Destination); \
                Destination[DestLength - 1] = Character; \
                Destination[DestLength] = '\0'; \
            }

        // Adds multiple characters to the end of the string
        #define AddCharacters(Destination, Character, Count) \
            { for(uint32_t i = 0; i < Count; i++){ \
                CharConcat(Destination, Character); \
            }}


    //-----Constants-----//

       

#define STRINGF_H
#endif