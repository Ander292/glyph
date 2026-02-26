#include "stringF.h"

// String functions

uint32_t StringLength(char* ptr){
    uint32_t Length = 0;
    for(Length = 0; *(ptr + Length) != '\0'; Length++); // This will just count the characters
    
    
    return Length + 1;
    //return Length;
}

void StringConcat(char *Destination, char *Source){
    uint32_t DestinationLength = StringLength(Destination);
    uint32_t SourceLength = StringLength(Source);

    //uint32_t Pos1 = SourceLength - 1;
    //uint32_t Pos2 = 0;

    /*while(Pos2 < SourceLength){
        Destination[Pos1++] = Source[Pos2++];
    }*/
    for(uint32_t i = 0; i < SourceLength; i++)
        Destination[DestinationLength + i - 1] = Source[i];
}

void MemoryCopy(char *Destination, char *Source, uint32_t Size){
    for(uint32_t i = 0; i < Size; i++)
        Destination[i] = Source[i];
}

void UintToString(uint32_t n, char *pStr, int digits){
    int cnt = DigitCount(n);
    int invert = ReverseOrder(n, cnt);
    int pos = 0;
    for(int i = 0; i < digits - cnt; i++){
        pStr[pos++] = ' ';
    }
    for(int i = 0; i < cnt; i++){
        pStr[pos++] = invert % 10 + 0x30; 
        // ASCII for 1 is 0x31, for 2 its 0x32 and so on...
        invert /= 10;
    }
    *(pStr + pos) = '\0';
}

void LongToString(int64_t n, char *pStr, int digits){
    int cnt = DigitCount(n);
    int64_t invert = ReverseOrder64(n, cnt);
    int pos = 0;
    for(int i = 0; i < digits - cnt; i++){
        pStr[pos++] = ' ';
    }
    for(int i = 0; i < cnt; i++){
        pStr[pos++] = invert % 10 + 0x30; 
        // ASCII for 1 is 0x31, for 2 its 0x32 and so on...
        invert /= 10;
    }
    *(pStr + pos) = '\0';
}

//Char

void CharToAnsi(char c, char *pStr){
    UintToString((uint32_t)c, pStr, 0);
}

// Special Functions

uint32_t LineLength(char* ptr){
    uint32_t Length = 0;
    for(Length = 0; *(ptr + Length) != '\n'; Length++); // This will just count the characters
    
    return Length;
}

uint32_t LineLengthEx(char* ptr, uint32_t MaxLength, uint8_t *FoundNewline){
    uint32_t Length = 0;
    *FoundNewline = 0U;
    for(Length = 0; Length < MaxLength; Length++){
        if(*(ptr + Length) == '\r' || *(ptr + Length) == '\n') {
            *FoundNewline = 1U;
            break;
        }      
    }
    
    return Length;
}

uint32_t CharacterCount(char *String, char C){
    uint32_t CharCount = 0;
    uint32_t Pos = 0;
    while(String[Pos] != '\0')
        if(String[Pos++] == C) CharCount++;

    return CharCount;
}

void StringShiftLeft(char *Str, uint32_t StartOffset, uint32_t EndOffset){
    uint32_t StringSize = StringLength(Str) - 1;

    if(StringSize < 2) Str[0] = '\0';

    for(uint32_t i = StartOffset; i < StringSize - EndOffset; i++)
        Str[i] = Str[i+1];
}

void StringShiftRight(char *Str, uint32_t StartOffset, uint32_t EndOffset){
    uint32_t StringSize = StringLength(Str);

    for(uint32_t i = StringSize - EndOffset; i > StartOffset; i--){
        Str[i] = Str[i - 1];
    }
}

uint32_t CountForwardToWord(char *Str, uint32_t CurrentPossition){
    uint32_t StrLength = StringLength(Str);
    while(Str[CurrentPossition] == ' ' && CurrentPossition < StrLength) CurrentPossition++;

    return CurrentPossition;
}

uint32_t CountBackToWord(char* Str, uint32_t CurrentPossition){
    CurrentPossition--;
    uint32_t StrLength = StringLength(Str);
    while(Str[CurrentPossition] == ' ' && CurrentPossition > 0) CurrentPossition--;

    if(Str[CurrentPossition] == ' ' && CurrentPossition < 1) return CurrentPossition;

    return CurrentPossition + 1;
}

uint32_t CountForwardToBlank(char *Str, uint32_t CurrentPossition){
    uint32_t StrLength = StringLength(Str);

    if((Str[CurrentPossition] < 48) ||
        (Str[CurrentPossition] > 57 && Str[CurrentPossition] < 65) ||
        (Str[CurrentPossition] > 90 && Str[CurrentPossition] < 97) ||
        (Str[CurrentPossition] > 122))
        return CurrentPossition + 1;

    while(CurrentPossition < StrLength){
        if((Str[CurrentPossition] < 48) ||
        (Str[CurrentPossition] > 57 && Str[CurrentPossition] < 65) ||
        (Str[CurrentPossition] > 90 && Str[CurrentPossition] < 97) ||
        (Str[CurrentPossition] > 122)) break;

        CurrentPossition++;
    }
    return CurrentPossition;
}

uint32_t CountBackToBlank(char *Str, uint32_t CurrentPossition){
    CurrentPossition--;
    uint32_t StrLength = StringLength(Str);

    if((Str[CurrentPossition] < 48) ||
        (Str[CurrentPossition] > 57 && Str[CurrentPossition] < 65) ||
        (Str[CurrentPossition] > 90 && Str[CurrentPossition] < 97) ||
        (Str[CurrentPossition] > 122))
        return CurrentPossition - 1;

    while(CurrentPossition != 0){
        if((Str[CurrentPossition] < 48) ||
        (Str[CurrentPossition] > 57 && Str[CurrentPossition] < 65) ||
        (Str[CurrentPossition] > 90 && Str[CurrentPossition] < 97) ||
        (Str[CurrentPossition] > 122)) break;

        CurrentPossition--;
    }

    if(CurrentPossition < 1 && !((Str[CurrentPossition] < 48) ||
        (Str[CurrentPossition] > 57 && Str[CurrentPossition] < 65) ||
        (Str[CurrentPossition] > 90 && Str[CurrentPossition] < 97) ||
        (Str[CurrentPossition] > 122)))
        return CurrentPossition;

    return CurrentPossition + 1;
}

// Number functions

uint32_t ReverseOrder(uint32_t n, int cnt){
    int result = 0;

    for(int i = 0; i < cnt; i++){
        int cif = n % 10; 
        n /= 10;
        result = result + cif * Power(10, cnt - i - 1);
    }

    return result;
}

int64_t ReverseOrder64(int64_t n, int cnt){
    int64_t result = 0;
    
    for(int i = 0; i < cnt; i++){
        int cif = n % 10;
        n /= 10;
        result = result + cif * Power64(10, cnt - i - 1);
    }

    return result;
}

int DigitCount(int n){
    if(!n) return 1;
    int i;
    for(i = 0; n != 0; i++) n /= 10;
    return i;
}

int Power(int base, int degree){
    if(!degree) return 1;
    int result = base;
    for(int i = 1; i < degree; i++){
        result *= base;
    }

    return result;
}

int Power64(int64_t base, int degree){
    if(!degree) return 1;
    int64_t result = base;
    for(int i = 1; i < degree; i++){
        result *= base;
    }

    return result;
}

int SmallerInteger(int a, int b){
    if(a > b) return b;
    else return a;
}

uint32_t SmallerUnsigned(uint32_t a, uint32_t b){
    if(a > b) return b;
    else return a;
}

uint32_t LargerUnsigned(uint32_t a, uint32_t b){
    if(a > b) return a;
    else return b;
}

uint32_t AbsoluteUnsigned(uint32_t n){
    if((int)n > 0) return n;
    else return (uint32_t)((int)n * -1);
}