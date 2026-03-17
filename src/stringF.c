#include "stringF.h"

// String functions

int StringLength(char* ptr){
    int Length = 0;
    for(Length = 0; *(ptr + Length) != '\0'; Length++); // This will just count the characters
    
    
    return Length + 1;
    //return Length;
}

void StringConcat(char *Destination, char *Source){
    int DestinationLength = StringLength(Destination);
    int SourceLength = StringLength(Source);

    //uint32_t Pos1 = SourceLength - 1;
    //uint32_t Pos2 = 0;

    /*while(Pos2 < SourceLength){
        Destination[Pos1++] = Source[Pos2++];
    }*/
    for(int i = 0; i < SourceLength; i++)
        Destination[DestinationLength + i - 1] = Source[i];
}

void MemoryCopy(char *Destination, char *Source, int Size){
    for(int i = 0; i < Size; i++)
        Destination[i] = Source[i];
}

void UintToString(int n, char *pStr, int digits){
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
    UintToString(c, pStr, 0);
}

// Special Functions

int LineLength(char* ptr){
    int Length = 0;
    for(Length = 0; *(ptr + Length) != '\n'; Length++); // This will just count the characters
    
    return Length;
}

int LineLengthEx(char* ptr, int MaxLength, uint8_t *FoundNewline){
    int Length = 0;
    *FoundNewline = 0U;
    for(Length = 0; Length < MaxLength; Length++){
        if(*(ptr + Length) == '\r' || *(ptr + Length) == '\n') {
            *FoundNewline = 1U;
            break;
        }      
    }
    
    return Length;
}

int CharacterCount(char *String, char C){
    int CharCount = 0;
    int Pos = 0;
    while(String[Pos] != '\0')
        if(String[Pos++] == C) CharCount++;

    return CharCount;
}

void StringShiftLeft(char *Str, int StartOffset, int EndOffset){
    int StringSize = StringLength(Str) - 1;

    if(StringSize < 2) Str[0] = '\0';

    for(int i = StartOffset; i < StringSize - EndOffset; i++)
        Str[i] = Str[i+1];
}

void StringShiftRight(char *Str, int StartOffset, int EndOffset){
    int StringSize = StringLength(Str);

    for(int i = StringSize - EndOffset; i > StartOffset; i--){
        Str[i] = Str[i - 1];
    }
}

int CountForwardToWordEx(char *Str, int CurrentPossition){
    int StrLength = StringLength(Str);
    int StartPossition = CurrentPossition;
    while(Str[CurrentPossition] == ' ' && CurrentPossition < StrLength) CurrentPossition++;

    return CurrentPossition - StartPossition;
}

int CountBackToWordEx(char* Str,int CurrentPossition){
    int StartPossition = CurrentPossition;
    CurrentPossition--;
    //uint32_t StrLength = StringLength(Str);
    while(Str[CurrentPossition] == ' ' && CurrentPossition > 0) CurrentPossition--;

    if(Str[CurrentPossition] == ' ' && CurrentPossition < 1) return StartPossition - CurrentPossition;

    return StartPossition - CurrentPossition - 1;
}

int CountForwardToBlankEx(char *Str, int CurrentPossition){
    int StrLength = StringLength(Str);
    int StartPossition = CurrentPossition;

    if((Str[CurrentPossition] < 48) ||
        (Str[CurrentPossition] > 57 && Str[CurrentPossition] < 65) ||
        (Str[CurrentPossition] > 90 && Str[CurrentPossition] < 97) ||
        (Str[CurrentPossition] > 122))
        return CurrentPossition + 1 - StartPossition;

    while(CurrentPossition < StrLength){
        if((Str[CurrentPossition] < 48) ||
        (Str[CurrentPossition] > 57 && Str[CurrentPossition] < 65) ||
        (Str[CurrentPossition] > 90 && Str[CurrentPossition] < 97) ||
        (Str[CurrentPossition] > 122)) break;

        CurrentPossition++;
    }
    return CurrentPossition - StartPossition;
}

int CountBackToBlankEx(char *Str, int CurrentPossition){
    int StartPossition = CurrentPossition;
    CurrentPossition--;
    //uint32_t StrLength = StringLength(Str);

    if((Str[CurrentPossition] < 48) ||
        (Str[CurrentPossition] > 57 && Str[CurrentPossition] < 65) ||
        (Str[CurrentPossition] > 90 && Str[CurrentPossition] < 97) ||
        (Str[CurrentPossition] > 122))
        return StartPossition - CurrentPossition;

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
        return StartPossition - CurrentPossition;

    return StartPossition - CurrentPossition - 1;
}

int CountForwardToWord(char *Str, int CurrentPossition){
    int StrLength = StringLength(Str);
    while(Str[CurrentPossition] == ' ' && CurrentPossition < StrLength) CurrentPossition++;

    return CurrentPossition;
}

int CountBackToWord(char* Str, int CurrentPossition){
    CurrentPossition--;
    //uint32_t StrLength = StringLength(Str);
    while(Str[CurrentPossition] == ' ' && CurrentPossition > 0) CurrentPossition--;

    if(Str[CurrentPossition] == ' ' && CurrentPossition < 1) return CurrentPossition;

    return CurrentPossition + 1;
}

int CountForwardToBlank(char *Str, int CurrentPossition){
    int StrLength = StringLength(Str);

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

int CountBackToBlank(char *Str, int CurrentPossition){
    CurrentPossition--;
    int StrLength = StringLength(Str);

    if((Str[CurrentPossition] < 48) ||
        (Str[CurrentPossition] > 57 && Str[CurrentPossition] < 65) ||
        (Str[CurrentPossition] > 90 && Str[CurrentPossition] < 97) ||
        (Str[CurrentPossition] > 122))
        return CurrentPossition + 1;

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

int ReverseOrder(int n, int cnt){
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

//261 368

uint32_t LargerUnsigned(uint32_t a, uint32_t b){
    if(a > b) return a;
    else return b;
}

uint32_t AbsoluteUnsigned(uint32_t n){
    if((int)n > 0) return n;
    else return (uint32_t)((int)n * -1);
}