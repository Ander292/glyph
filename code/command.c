#ifndef COMMAND_C
#define COMMAND_C

#include "res/command_types.h"
#include "types.h"
#include "main.h"
#include "system/io.h"

static void executeCommand(command_type ct){
    switch(ct){
        case COMMAND_QUIT: {
            for(u32 BufferIndex = 0; BufferIndex < EDITOR_STATE_COUNT; BufferIndex++){
                editor_state *focusedState = &GLOBAL_STRUCT_NAME.states[BufferIndex];
                if(TEST_FLAG(focusedState->Flags, FLAG_INITIALIZED)){
                    if(TEST_FLAG_LOCAL(FLAG_DIRTY)){
                        SET_EDITOR_ID(BufferIndex);

                        postEditorMessage(4*messageDurationDefault, "Unsaved changes (%u). Quit? (Yes, No, Save)", BufferIndex);
                        while(1){
                            editorRefreshScreen();
                            read_result rr = io_ReadConsoleInput();
                            if(rr.type != TYPE_ANSI) continue;
                            switch(rr.bytes[0] & 0xdf){
                                case 'Y':
                                    //CLEAR_FLAG_GLOBAL(GLOBAL_FLAG_RUNNING);
                                    goto END_WHILE;
                                case 'N':
                                    clearEditorMessage();
                                    return;
                                case 'S':
                                    switch(editorSave(focusedState->File, &focusedState->List)){
                                        case 1: break;
                                        case 0:
                                            goto END_WHILE;
                                        case -1:
                                            postEditorMessage(messageDurationDefault, "Saving canceled on buffer %d", BufferIndex);
                                            return;
                                    }
                                    break;
                            }
                        }
                        END_WHILE:
                    }
                    CLEAR_FLAG(focusedState->Flags, FLAG_INITIALIZED);
                }
            }
            CLEAR_FLAG_GLOBAL(GLOBAL_FLAG_RUNNING);
            break;
        }
        case COMMAND_OPEN_HELP: break; // TODO: Implement help
        case COMMAND_SAVE: {
            editorSave(EC.File, &EC.List);
            break;
        }break;
        case COMMAND_LOAD: { // File open
            if(TEST_FLAG_LOCAL(FLAG_DIRTY) && EC.File[0] != 0){
                postEditorMessage(4*messageDurationDefault, "Unsaved changes (%u). Proceed? (Yes, No, Save)", GET_EDITOR_ID());
                while(1){
                    editorRefreshScreen();
                    read_result rr = io_ReadConsoleInput();
                    if(rr.type != TYPE_ANSI) continue;
                    switch(rr.bytes[0] & 0xdf){
                        case 'Y':
                            goto END_WHILE2;
                        case 'N':
                            clearEditorMessage();
                            return;
                        case 'S':
                            if(editorSave(EC.File, &EC.List)) break;
                            goto END_WHILE2;
                    }
                }
                END_WHILE2:
            }
            string_a *feedback = editorPromtHeader("Enter path:");
            if(feedback != NULL) {
                if(io_GetFileSize(feedback->data) <= 0) {
                    postEditorMessage(2*messageDurationDefault, "Invalid file: %s : %s", feedback->data, io_GetInputOutputErrorString());
                    break;
                }
#if 0
                FreeList(&EC.List);
                
                EC.List = CreateListWithRows(1, LIST_TYPE_UTF32);

                E.Cursor = (int_pair){0, 0};
#else
                resetLocalEditorState(GET_EDITOR_ID());
#endif
                //strncpy(EC.File, feedback->data, sizeof(EC.File));
                u32 copySize = MIN_VAL(feedback->len, sizeof(EC.File) - 1);
                memcpy(EC.File, feedback->data, copySize);
                EC.File[copySize] = 0;

                editorLoadFile(GET_EDITOR_ID());
                StringFreeHeap(feedback);
            }
        }break;
        case COMMAND_SHOWNUMBER_TOGGLE: TOGGLE_FLAG_GLOBAL(GLOBAL_FLAG_SHOWNUMBERS); break;
        case COMMAND_SHOWHEADER_TOGGLE: TOGGLE_FLAG_GLOBAL(GLOBAL_FLAG_SHOWHEADER); break;
        case COMMAND_SYNTAX: TOGGLE_FLAG_LOCAL(FLAG_SYNTAX); break;
        case COMMAND_INSERT: TOGGLE_FLAG_GLOBAL(GLOBAL_FLAG_INSERT); break;
        case COMMAND_GOTO_LINE:{
            string_a *result = editorPromtHeader("Goto line:");
            if(result){
                char *endPtr;
                i32 val = strtoul(result->data, &endPtr, 0);
                StringFreeHeap(result);
                
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
                        EC.Cursor.Y = (val - 1 < EC.List.size ? val - 1 : EC.List.size - 1);
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
        case COMMAND_FIND: { // Search
            SEARCH_REPEAT:
            string_a *result = editorPromtHeader("Find:");
            
            if(result){
                string_u32 *result32 = BufferCreateFromStringU32(result->data, result->len);
                NullTerminateStringU32(result32);

                i32 matchCount = 0;
                int_pair oldCursor = EC.Cursor;
                do{
                    list_node *p = EC.List.head;
                    for(i32 i = 0; i < EC.List.size; i++, p = p->next){
                        string_u32 *focusedStr = (string_u32 *)p->str;
                        wchar32 *resultPtr = StrstrU32(focusedStr->data, result32->data);
                        
                        while(resultPtr != NULL){
                            matchCount++;
                            i32 xOffset = resultPtr - focusedStr->data;
                            EC.Cursor.X = xOffset;
                            EC.Cursor.Y = i;

                            fixCursorPossition();
                            if(TEST_FLAG_LOCAL(FLAG_SYNTAX))
                                syntaxHighlight(&EC.List);
                            else
                                resetSyntaxHighlight(&EC.List);

                            /* Highlighting the string */
                            
                            SyntaxForceU32(p->str, xOffset, TOKEN_SEARCH_HIGHTLIGHT, result32->len);

                            editorRefreshScreen();

                            read_result rr;
                            do{
                                rr = io_ReadConsoleInput();
                            }while(rr.size == 0);

                            switch(*rr.bytes){
                                case '\r':
                                    // Search further
                                    resultPtr = StrstrU32(resultPtr + result32->len, result32->data);
                                    break;
                                case 'r':
                                    // Return to old possition
                                    EC.Cursor = oldCursor;
                                    ATR_FALLTROUGHT;
                                case CTRL_KEY('f'):
                                    StringFreeHeap(result);
                                    goto SEARCH_REPEAT;
                                    break;
                                default:
                                    // Break search
                                    SyntaxForceU32(p->str, xOffset, TOKEN_NEUTRAL, result32->len);
                                    resultPtr = NULL;
                                    goto WHILE_END;
                                    break;
                            }
                        }
                    }

                }while(matchCount);
                WHILE_END:
                if(matchCount){
                    clearEditorMessage();
                }else{
                    postEditorMessage(messageDurationDefault, "No matches");

                    StringFreeHeap((string*)result32);
                }
            }else{
                postEditorMessage(messageDurationDefault, "Canceled");
            }
            StringFreeHeap((string*)result);
        }break;
        case COMMAND_SWITCH_CHARSET: break;
        case COMMAND_TOGGLE_READONLY: TOGGLE_FLAG_LOCAL(FLAG_READONLY); break;
        case COMMAND_BACKSPACE: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;

            if(EC.Cursor.X == 0){
                string_u32 *rowNext = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);

                if(EC.Cursor.Y != 0){
                    string_u32 *rowPrev = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y-1);
                    EC.Cursor.X = rowPrev->len;
                    ConcatStringsU32(rowPrev, rowNext);
                    ListDeleteRow(&EC.List, EC.Cursor.Y);

                    EC.Cursor.Y--;
                    SET_FLAG_LOCAL(FLAG_DIRTY);
                }
            }else{
                deleteCharFromPosList(&EC.List, EC.Cursor.Y, EC.Cursor.X - 1);
                EC.Cursor.X--;
                SET_FLAG_LOCAL(FLAG_DIRTY);
            }
        }break;
        case COMMAND_REMOVE_WORD_LEFT: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;
            string_u32 *current = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);

            i32 startCharOffset = EC.Cursor.X;
            wchar32 *start = current->data + startCharOffset;
            i32 removeCountLeft;

            if(((start - 1 < current->data) || start[-1] == U' ')){
                removeCountLeft = CountBackToNotChar32(current, EC.Cursor.X, U' ');
            }else{
                removeCountLeft = CountBackToChar32(current, EC.Cursor.X, U' ');
            }
            if(removeCountLeft != 0){
                SET_FLAG_LOCAL(FLAG_DIRTY);
                for(i32 i = 0; i < removeCountLeft; i++){
                    DeleteCharFromPosU32(current, EC.Cursor.X-1);
                    EC.Cursor.X--;
                }
            }
        } break;
        case COMMAND_DELETE: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;

            string_u32 *currentRow = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);
            if(EC.Cursor.X == currentRow->len){
                if(EC.Cursor.Y + 1 < EC.List.size){
                    string_u32 *nextRow = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y + 1);

                    ConcatStringsU32(currentRow, nextRow);
                    ListDeleteRow(&EC.List, EC.Cursor.Y);
                    SET_FLAG_LOCAL(FLAG_DIRTY);
                }

            }else{
                deleteCharFromPosList(&EC.List, EC.Cursor.Y, EC.Cursor.X);
                SET_FLAG_LOCAL(FLAG_DIRTY);
            }
        }break;
        case COMMAND_REMOVE_WORLD_RIGHT: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;
            string_u32 *current = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);

            i32 startCharOffset = EC.Cursor.X;
            wchar32 *start = current->data + startCharOffset;
            i32 removeCountLeft;


            if(start[0] == U' '){
                removeCountLeft = CountToNotChar32(current, EC.Cursor.X, U' ');
            }else{
                removeCountLeft = CountToChar32(current, EC.Cursor.X, U' ');
            }

            if(removeCountLeft != 0){
                SET_FLAG_LOCAL(FLAG_DIRTY);
                for(i32 i = 0; i < removeCountLeft; i++){
                    DeleteCharFromPosU32(current, EC.Cursor.X);
                }
            }
        } break;
        case COMMAND_NEWLINE: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;

            SET_FLAG_LOCAL(FLAG_DIRTY);
            string_u32 *row = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);
            string_u32 *str = StringCreateHeapU32(64);
            ListInsertAtPossition(&EC.List, (string*)str, EC.Cursor.Y + 1);
            
            if(EC.Cursor.X != row->len){
                i32 startCharOffset = EC.Cursor.X;
                i32 charsToCopy = row->len - startCharOffset;

                ConcatStringsExU32(str, row, startCharOffset, 0);
                TerminateStringOnPosU32(row, startCharOffset);
            }

            EC.Cursor.Y++;
            EC.Cursor.X = 0;
        }break;
        case COMMAND_TAB: {
            if(TEST_FLAG_LOCAL(FLAG_READONLY)) break;
            SET_FLAG_LOCAL(FLAG_DIRTY);

            wchar32 whitespace = U' ';
            insertCharAtPosList(&EC.List, whitespace, EC.Cursor.Y, EC.Cursor.X);
            EC.Cursor.X++;
            for(; EC.Cursor.X % TAB_SPACE_SIZE; ++EC.Cursor.X){
                insertCharAtPosList(&EC.List, whitespace, EC.Cursor.Y, EC.Cursor.X);
            }
        }break;

        case COMMAND_UP: EC.Cursor.Y--; break;
        case COMMAND_LEFT: EC.Cursor.X--; break;
        case COMMAND_RIGHT: EC.Cursor.X++; break;
        case COMMAND_DOWN: EC.Cursor.Y++; break;
        case COMMAND_PAGEUP: 
            EC.Cursor.Y -= (GlobalConsoleInfo.Rows - E.TextOffsetTop);
            EC.Offset.Y -= (GlobalConsoleInfo.Rows - E.TextOffsetTop);
            break;
        case COMMAND_PAGEDOWN: 
            EC.Cursor.Y += (GlobalConsoleInfo.Rows - E.TextOffsetTop);
            EC.Offset.Y += (GlobalConsoleInfo.Rows - E.TextOffsetTop);
            break;
        case COMMAND_HOME: EC.Cursor.X = 0; break;
        case COMMAND_END: {
            string *str = GetStringAtIndex(&EC.List, EC.Cursor.Y);
            if(str != NULL){
                EC.Cursor.X = str->len;
            }
        }break;

        case COMMAND_OFFSET_UP: EC.Offset.Y--; break;
        case COMMAND_OFFSET_DOWN: EC.Offset.Y++; break;
        case COMMAND_SKIP_WORD_RIGHT: {
            string_u32 *current = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);

            i32 startCharOffset = EC.Cursor.X;
            wchar32 *start = current->data + startCharOffset;
            i32 skipCount;

            if(start[0] == U' '){
                skipCount = CountToNotChar32(current, EC.Cursor.X, U' ');
            }else{
                skipCount = CountToChar32(current, EC.Cursor.X, U' ');
            }

            EC.Cursor.X += skipCount;
        } break;
        case COMMAND_SKIP_WORD_LEFT: {
            string_u32 *current = (string_u32*)GetStringAtIndex(&EC.List, EC.Cursor.Y);

            i32 startCharOffset = EC.Cursor.X;
            wchar32 *start = current->data + startCharOffset;
            i32 skipCount;

            if((start - 1 < current->data) || (start[-1] == U' ')){
                skipCount = CountBackToNotChar32(current, EC.Cursor.X, ' ');
            }else{
                skipCount = CountBackToChar32(current, EC.Cursor.X, ' ');
            }

            EC.Cursor.X -= skipCount;
        } break;

        case COMMAND_MOVE_ROW_UP: {
            if(EC.Cursor.Y > 0 && EC.List.size > 1){
                SwapStringsForIndexes(&EC.List, EC.Cursor.Y, EC.Cursor.Y - 1);
                EC.Cursor.Y--;
            }
        }break;
        case COMMAND_MOVE_ROW_DOWN: {
            if(EC.Cursor.Y + 1 < EC.List.size && EC.List.size > 1){
                SwapStringsForIndexes(&EC.List, EC.Cursor.Y, EC.Cursor.Y + 1);
                EC.Cursor.Y++;
            }
        }break;
        case COMMAND_NEXT_BUFFER: {
            u8 contextIndex = GET_EDITOR_ID();

            switchEditorState((contextIndex + 1U) % EDITOR_STATE_COUNT);
        }break;
        case COMMAND_PREV_BUFFER: {
            u8 currentIndex = GET_EDITOR_ID();
            currentIndex -= 1U;
            
            if(currentIndex > EDITOR_STATE_COUNT) currentIndex = EDITOR_STATE_COUNT - 1;
            switchEditorState(currentIndex);
        }break;

        case COMMAND_NEUTRAL:
            postEditorMessage(messageDurationDefault, "COMMAND_NEUTRAL recieved");
            break;
        default:
            postEditorMessage(messageDurationDefault, "Invalid command type : %x", ct);
            break;
    }    
}

#endif