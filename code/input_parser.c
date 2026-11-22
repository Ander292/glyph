#if 0
static void processInput(character_input ci){
    switch(rr.bytes[0]){
        case CTRL_KEY('c'): // Ctrl+c and ctrl+v is managed by the console host for now
        case CTRL_KEY('v'):
        //case CTRL_KEY('x'):
            break;
        case CTRL_KEY('q'):{
            if(HAS_FLAG(FLAG_DIRTY)){
                postEditorMessage(4*messageDurationDefault, "Unsaved changes. Quit? (Yes, No, Save)");
                while(1){
                    editorRefreshScreen();
                    character_input ci = pollInput();
                    switch(rr.bytes[0] & 0xdf){
                        case 'Y':
                            UNSET_FLAG(FLAG_RUNNING);
                            goto END_WHILE;
                        case 'N':
                            clearEditorMessage();
                            return;
                        case 'S':
                            switch(editorSave(E.File, DEFAULT_SAVE_SOURCE_ADDRESS)){
                                case 1: break;
                                case 0:
                                    goto END_WHILE;
                                case -1:
                                    postEditorMessage(messageDurationDefault, "Saving canceled");
                                    return;
                            }
                            break;
                    }
                }
                END_WHILE:
            }
            UNSET_FLAG(FLAG_RUNNING);
            break;
        }
        case CTRL_KEY('n'):
            TOGGLE_FLAG(FLAG_SHOWNUMBERS);
            break;
        case CTRL_KEY('o'):{ // File open
            if(HAS_FLAG(FLAG_DIRTY) && E.File[0] != 0){
                postEditorMessage(4*messageDurationDefault, "Unsaved changes. Proceed? (Yes, No, Save)");
                while(1){
                    editorRefreshScreen();
                    character_input ci = pollInput();
                    switch(rr.bytes[0] & 0xdf){
                        case 'Y':
                            goto END_WHILE2;
                        case 'N':
                            clearEditorMessage();
                            return;
                        case 'S':
                            if(editorSave(E.File, DEFAULT_SAVE_SOURCE_ADDRESS)) break;
                            goto END_WHILE2;
                    }
                }
                END_WHILE2:
            }
            string *feedback = editorPromtHeader("Enter path:");
            if(feedback != NULL) {
                if(getFileSize(feedback->data) <= 0) {
                    postEditorMessage(2*messageDurationDefault, "Invalid file: %s : %s", feedback->data, getInputOutputErrorString());
                    break;
                }
                freeList(E.Rows);
                *(E.Rows) = createListWithRows(1);
                E.Cursor = (int_pair){0, 0};
                strcpy(E.File, feedback->data);
                editorLoadFile(E.File);
                stringFreeHeap(feedback);
            }
            break;
        }
        case CTRL_KEY('g'):{
            string *Result = editorPromtHeader("Goto line:");
            if(Result){
                char *endPtr;
                int32 val = strtoul(Result->data, &endPtr, 0);
                stringFreeHeap(Result);
                if(*endPtr != 0){
                    postEditorMessage(messageDurationDefault, "Invalid input: Not a number");
                }else if(errno == ERANGE){
                    postEditorMessage(messageDurationDefault, "Invalid input: Overflow (number too large)");
                }else{
                    if(val > 0){
#if 0
                        if(val < E.Rows->size){
                            E.Cursor.Y = val-1;
                            clearEditorMessage();
                        }else{
                            postEditorMessage(messageDurationDefault, "Out of bounds: %d < %d", E.Rows->size, val);
                        }
#else
                        E.Cursor.Y = (val - 1 < E.Rows->size ? val - 1 : E.Rows->size - 1);
                        clearEditorMessage();
#endif
                    }else{
                        postEditorMessage(messageDurationDefault, "Out of bounds: %d is negative", val);
                    }
                }
            }else{
                postEditorMessage(messageDurationDefault, "Canceled");
            }
        }break;
        case CTRL_KEY('t'):
            TOGGLE_FLAG(FLAG_SHOWHEADER);
            break;
        case CTRL_KEY('f'):{
            SEARCH_REPEAT:
            // Search (Ctrl+f)
            string *Result = editorPromtHeader("Find:");
            if(Result){
                int32 matchCount = 0;
                int_pair oldCursor = E.Cursor;
                do{
                    list_node *p = E.Rows->head;
                    for(int32 i = 0; i < E.Rows->size; i++, p = p->next){
                        char *resultPtr = strstr(p->str->data, Result->data);
                        while(resultPtr != NULL){
                            matchCount++;
                            int32 xOffset = resultPtr - p->str->data;
                            E.Cursor.X = xOffset;
                            E.Cursor.Y = i;

                            FixCursorPossition();
                            if(HAS_FLAG(FLAG_SYNTAX))
                                syntaxHighlight(E.Rows);
                            else
                                resetSyntaxHighlight(E.Rows);

                            /* Highlighting the string */
                            int xOffsetInChars;
                            int charLen = stringByteToCharCount(p->str, xOffset, 0, Result->byteLen, &xOffsetInChars);
                            syntaxForce(p->str, xOffsetInChars, TOKEN_SEARCH_HIGHTLIGHT, charLen);

                            editorRefreshScreen();

                            character_input ci;
                            do{
                                ci = pollInput();
                            }while(ci.byteCount == 0);

                            switch(*rr.bytes){
                                case '\r':
                                    // Search further
                                    resultPtr = strstr(resultPtr + Result->byteLen, Result->data);
                                    break;
                                case 'r':
                                    // Return to old possition
                                    E.Cursor = oldCursor;
                                    ATR_FALLTROUGHT;
                                case CTRL_KEY('f'):
                                    stringFreeHeap(Result);
                                    goto SEARCH_REPEAT;
                                    break;
                                default:
                                    // Break search
                                    resultPtr = NULL;
                                    goto WHILE_END;
                                    break;
                            }
                        }
                    }

                }while(matchCount);
                WHILE_END:
                if(matchCount)
                    clearEditorMessage();
                else
                    postEditorMessage(messageDurationDefault, "No matches");
                    stringFreeHeap(Result);
            }else{
                postEditorMessage(messageDurationDefault, "Canceled");
            }
        }break;
        case CTRL_KEY('p'):
#if 1
            TOGGLE_FLAG(FLAG_SYNTAX);
            //SET_ALT_BUFFERID(0);
#else
            string *src = editorPromtHeader("Input string");
            if(src != NULL){
                string *dest = getStringAtIndex(E.Rows, E.Cursor.Y);
                for(int i = 0; i < src->len; i++){
                    character_input ci = getCharAtPos(src, i);
                    insertCharAtPossition(dest, ci, E.Cursor.X++, HAS_FLAG(FLAG_INSERT_MODE));
                }
                stringFreeHeap(src);
            }
#endif
            break;
        case CTRL_KEY('u'):
            // TODO: Switch between UTF16 and UTF8 output
            break;
        case CTRL_KEY('r'):
            TOGGLE_FLAG(FLAG_READONLY);
            break;
        //case CTRL_KEY('w'):
        case '\b': // Backspace (or ctrl + h)
            if(HAS_FLAG(FLAG_READONLY)) break;
            string *current = getStringAtIndex(E.Rows, E.Cursor.Y);
            int startOffset;
            int byteCountTillEnd = stringCharToByteCount(current, E.Cursor.X, 0, 0, &startOffset);
            char *start = current->data + startOffset;
            int removeCountLeft;
            if(((start - 1 < current->data) || start[-1] == ' ')){
                removeCountLeft = countBackToNotChar(current, E.Cursor.X, ' ');
            }else{
                removeCountLeft = countBackToChar(current, E.Cursor.X, ' ');
            }
            if(removeCountLeft != 0){
                SET_FLAG(FLAG_DIRTY);
                for(int i = 0; i < removeCountLeft; i++){
                    deleteCharFromPossition(current, E.Cursor.X-1);
                    E.Cursor.X--;
                }
            }
            break;
        case CTRL_KEY('e'):{
            // Help
            uint32 wasDirty = HAS_FLAG(FLAG_DIRTY);
            if(HAS_FLAG(FLAG_ALTVIEW) && GET_ALT_BUFFERID() == 0){
                switchToMainBuffer();
                E.Cursor = E.oldCursor;
                E.oldCursor = (int_pair){-1, -1};                
            }else{
                E.oldCursor = E.Cursor;
                switchToAltBuffer(0);

                freeList(E.Rows);
                *E.Rows = createListWithRows(1);
                E.Cursor = (int_pair){0, 0};
                stringIntoInput(helpFull, helpFullLength);
            }
            if(!wasDirty) CLEAR_FLAG(FLAG_DIRTY);
        } break;
        case CTRL_KEY('s'):
#if 0
            if(*E.File == 0){
                    string *str = editorPromtHeader("Save location");
                    if(getFileSize(str->data) > 0) {
                        strcpy(E.File, str->data);
                        free(str);
                        editorSave(E.File, DEFAULT_SAVE_SOURCE_ADDRESS);
                        break;
                    }else{
                        postEditorMessage(messageDurationDefault, "Invalid file: %s", str->data);
                    }
                    free(str);
            }
            else{
                editorSave(E.File, DEFAULT_SAVE_SOURCE_ADDRESS);
            }
#endif
            editorSave(E.File, DEFAULT_SAVE_SOURCE_ADDRESS);
            break;
        case 127:  // Delete key in ascii but its backspace for some reason
        {
            if(HAS_FLAG(FLAG_READONLY)) break;
            if(E.Cursor.X == 0){
                string *rowNext = getStringAtIndex(E.Rows, E.Cursor.Y);
                if(E.Cursor.Y != 0){
                    string *rowPrev = getStringAtIndex(E.Rows, E.Cursor.Y-1);
                    E.Cursor.X = rowPrev->len;
                    stringAppendEnd(rowPrev, rowNext->data, rowNext->byteLen);
                    listDeleteRow(E.Rows, E.Cursor.Y);
                    E.Cursor.Y--;
                    SET_FLAG(FLAG_DIRTY);
                }
            }else{
                deleteCharFromPosList(E.Rows, E.Cursor.Y, E.Cursor.X - 1);
                E.Cursor.X--;
                SET_FLAG(FLAG_DIRTY);
            }
            break;
        }
        case '\n':
            if(HAS_FLAG(FLAG_NEWLINE_ENTER)) goto NEWLINE;
            break;
        case '\r':{
            NEWLINE:
            if(HAS_FLAG(FLAG_READONLY)) break;
            SET_FLAG(FLAG_DIRTY);
            string *row = getStringAtIndex(E.Rows, E.Cursor.Y);
            string *str = stringCreateHeap(64);
            listInsertAtPossition(E.Rows, str, E.Cursor.Y + 1);
            
            if(E.Cursor.X != row->len){
                int startOffset;
                int bytesToCopy = stringCharToByteCount(row, E.Cursor.X, 0, 0, &startOffset);
                stringAppendEnd(str, row->data + startOffset, bytesToCopy);
                character_input ci = {0};
                rr.bytes[0] = 0;
                ci.byteCount = 1;
                //insertCharAtPossition(row, ci, E.Cursor.X, 0);
                terminateStringOnPos(row, E.Cursor.X);
            }

            E.Cursor.Y++;
            E.Cursor.X = 0;
            break;
        }
        case '\t':
            if(HAS_FLAG(FLAG_READONLY)) break;
            SET_FLAG(FLAG_DIRTY);
            character_input tabs = {0};
            tabs.arr[0] = ' ';
            tabs.byteCount = 1;
            insertCharAtPosList(tabs, E.Cursor.Y, E.Cursor.X);
            E.Cursor.X++;
            int oldCursorX = E.Cursor.X;
            for(;E.Cursor.X % TAB_SPACE_SIZE; E.Cursor.X++){
                insertCharAtPosList(tabs, E.Cursor.Y, oldCursorX);
            }
            break;
        // case 127: // Delete key
        //     postEditorMessage(messageDurationDefault, "Delete key ):");
        //     SET_FLAG(FLAG_DIRTY);
        //     break;
        case '\x1b':{
            switch(rr.bytes[2]){
                case 'A': // Up arrow
                case 'a':
                    UP_ARROW:
                    E.Cursor.Y--;
                    break;
                case 'B': // Down arrow
                case 'b':
                    DOWN_ARROW:
                    E.Cursor.Y++;
                    break;
                case 'C': // Right arrow
                case 'c':
                    RIGHT_ARROW:
                    E.Cursor.X++;
                    break;
                case 'D': // Left arrow
                case 'd':
                    LEFT_ARROW:
                    E.Cursor.X--;
                    break;
                case '5': // Pageup (followed by ~ but not checked)
                    //E.Cursor.Y -= (ConInfo.Rows - 2);
                    E.Cursor.Y -= (ConInfo.Rows - E.TextOffsetTop);
                    E.Offset.Y -= (ConInfo.Rows - E.TextOffsetTop);
                    break;
                case '6': // Pagedown (followed by ~ but not checked)
                    //E.Cursor.Y += (ConInfo.Rows - 2);
                    E.Cursor.Y += (ConInfo.Rows - E.TextOffsetTop);
                    E.Offset.Y += (ConInfo.Rows - E.TextOffsetTop);
                    // E.Cursor.Y = E.Offset.Y + ConInfo.Rows - 1;
                    // if(E.Cursor.Y > E.Rows->size - 1) E.Cursor.Y = E.Rows->size - 1;
                    break;
                case '2': // Insert key (followed by ~ but not checked)
                    TOGGLE_FLAG(FLAG_INSERT_MODE);
                    break;
                case 'O': // For the home and end escape sequences. They are diferent for different systems
                    if(rr.bytes[3] == 'H') goto HOME;
                    else if(rr.bytes[3] == 'F') goto END;
                    break;
                /* Home key. Goes to X index 0 - the begining to the line */
                case '1':
                    if(ci.byteCount == 6){
                        if(rr.bytes[3] == ';'){
                            switch(rr.bytes[4]){
                                case '5':{ // Ctrl modifier
                                    switch(rr.bytes[5]){
                                        case 'A': // up arrow
                                            E.Offset.Y--;
                                            break;
                                        case 'B': // down arrow
                                            E.Offset.Y++;
                                            break;
                                        case 'C':{ // right arrow
                                            string *current = getStringAtIndex(E.Rows, E.Cursor.Y);
                                            int startOffset;
                                            int byteCountTillEnd = stringCharToByteCount(current, E.Cursor.X, 0, 0, &startOffset);
                                            char *start = current->data + startOffset;
                                            if(*start == ' '){
                                                E.Cursor.X += countToNotChar(current, E.Cursor.X, ' ');
                                            }else{
                                                E.Cursor.X += countToChar(current, E.Cursor.X, ' ');
                                            }
                                            break;
                                        }
                                        case 'D':{ // left arrow
                                            string *current = getStringAtIndex(E.Rows, E.Cursor.Y);
                                            int startOffset;
                                            int byteCountTillEnd = stringCharToByteCount(current, E.Cursor.X, 0, 0, &startOffset);
                                            char *start = current->data + startOffset;
                                            if((start - 1 < current->data) || start[-1] == ' '){
                                                E.Cursor.X -= countBackToNotChar(current, E.Cursor.X, ' ');
                                            }else{
                                                E.Cursor.X -= countBackToChar(current, E.Cursor.X, ' ');
                                            }
                                            break;
                                        }
                                    }
                                    break;
                                }
                                case '3':{ // Alt keys
                                    switch(rr.bytes[5]){
                                        case 'A': // Up arrow
                                            if(E.Cursor.Y > 0 && E.Rows->size > 1) {
                                                //string *currentLine = getStringAtIndex(E.Rows, E.Cursor.Y);
                                                //string *prevLine = getStringAtIndex(E.Rows, E.Cursor.Y - 1);
                                                swapStringsForIndexes(E.Rows, E.Cursor.Y, E.Cursor.Y - 1);
                                                E.Cursor.Y--;
                                            }
                                            break;
                                        case 'B': // down arrow
                                            if(E.Cursor.Y + 1 < E.Rows->size && E.Rows->size > 1){
                                                swapStringsForIndexes(E.Rows, E.Cursor.Y, E.Cursor.Y + 1);
                                                E.Cursor.Y++;
                                            }
                                            break;
                                        case 'C': { // Right arrow
                                            if(HAS_FLAG(FLAG_ALTVIEW)){
                                                uint8 currentBufferId = GET_ALT_BUFFERID();

                                                SET_ALT_BUFFERID(((currentBufferId + 1U) % (ALT_BUFFER_COUNT)));
                                            }
                                            break;
                                        }
                                        case 'D': { // Left arrow
                                            if(HAS_FLAG(FLAG_ALTVIEW)){
                                                uint8 currentBufferId = GET_ALT_BUFFERID();
                                                currentBufferId -= 1U;
                                                if(currentBufferId > ALT_BUFFER_COUNT) currentBufferId = ALT_BUFFER_COUNT - 1;
                                                SET_ALT_BUFFERID(currentBufferId);
                                            }
                                            break;  
                                        } 
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case '7':
                case 'H':
                    HOME:
                    E.Cursor.X = 0;
                    break;
                /* End key. Goes to the end of the line (depending on its size) */
                case '4':
                case '8':
                case 'F':
                    END:
                    string *str = getStringAtIndex(E.Rows, E.Cursor.Y);
                    if(str != NULL){
                        E.Cursor.X = str->len;
                    }
                    break;
                case '3': // Delete key
                    //postEditorMessage(messageDurationDefault, "Delete key <3");
                    //SET_FLAG(FLAG_DIRTY);
                    if(HAS_FLAG(FLAG_READONLY)) break;
                    string *currentRow = getStringAtIndex(E.Rows, E.Cursor.Y);
                    if(E.Cursor.X == currentRow->len){
                        if(E.Cursor.Y + 1< E.Rows->size){
                            string *nextRow = getStringAtIndex(E.Rows, E.Cursor.Y + 1);
    
                            stringAppendEnd(currentRow, nextRow->data, nextRow->byteLen);
                            listDeleteRow(E.Rows, E.Cursor.Y + 1);
                            SET_FLAG(FLAG_DIRTY);
                        }
                    }else{
                        deleteCharFromPosList(E.Rows, E.Cursor.Y, E.Cursor.X);
                        SET_FLAG(FLAG_DIRTY);
                    }
                    break;
                
            }
            break;
        }
        default:{
            if(HAS_FLAG(FLAG_READONLY)) break;
            //stringAppendEnd(&OutputBuffer, rr.bytes, ci.byteCount);
            if(rr.bytes[0] != 0 && rr.bytes[0] != ESC_SEQ[0]){
                SET_FLAG(FLAG_DIRTY);
                insertCharAtPosList(rr, E.Cursor.Y, E.Cursor.X);
                E.Cursor.X++;
            }
            break;
        }
    }

    FixCursorPossition();
}
#endif

#include "input_parser.h"

void parser_ParseInput(read_result rr){
    if(rr.type == TYPE_ANSI){
        
    }
}