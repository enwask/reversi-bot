#include <stdio.h>
#include <stdlib.h>

#include "reversi_functions.h"

const int DX[] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int DY[] = {-1, 0, 1, -1, 1, -1, 0, 1};
const char PIECES[] = {'_', 'O', 'X'};

void initBoard(enum piece board[][SIZE]) {
    
    // Make everything empty.
    int i, j;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            board[i][j] = EMPTY;
    
    // Starting board has this pattern.
    board[SIZE / 2 - 1][SIZE / 2 - 1] = WHITE;
    board[SIZE / 2][SIZE / 2] = WHITE;
    board[SIZE / 2 - 1][SIZE / 2] = BLACK;
    board[SIZE / 2][SIZE / 2 - 1] = BLACK;
}

#ifndef ANSI_YELLOW
#   define ANSI_YELLOW "\x1b[33m"
#   define ANSI_PURPLE "\x1b[35m"
#   define ANSI_RESET "\x1b[0m"
#endif

// Prints board.
void printBoard(enum piece board[][SIZE]) {
    
    int i, j;
    
    // Column headers.
    printf(" ");
    for (i = 0; i < SIZE; i++)
        printf(" %d", i);
    printf("\n");
    
    for (i = 0; i < SIZE; i++) {
        
        // Row header, row, then new line.
        printf("%d ", i);
        for (j = 0; j < SIZE; j++) {
            switch (board[i][j]) {
                case BLACK:printf(ANSI_PURPLE "X " ANSI_RESET);
                    break;
                case WHITE:printf(ANSI_YELLOW "O " ANSI_RESET);
                    break;
                default:printf(". ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

// Returns TRUE iff ptrPos points to a position on the board.
enum boolean inbounds(const position *ptrPos) {
    return ptrPos->x >= 0 && ptrPos->x < SIZE && ptrPos->y >= 0 && ptrPos->y < SIZE;
}

enum piece opposite(enum piece mine) {
    if (mine == WHITE) return BLACK;
    if (mine == BLACK) return WHITE;
    return EMPTY;
}

// Allocates a new position storing the position starting at ptrPos, one move in
// the direction of direction.
position *getPosInDir(const position *ptrPos, int direction) {
    position *tmp = malloc(sizeof(position));
    tmp->x = ptrPos->x + DX[direction];
    tmp->y = ptrPos->y + DY[direction];
    return tmp;
}

// Returns TRUE iff ptrP1 and ptrP2 are pointing to the same position values.
enum boolean equal(const position *ptrP1, const position *ptrP2) {
    return ptrP1->x == ptrP2->x && ptrP1->y == ptrP2->y;
}

// Used for playing with a human player - asks the user to enter a move
// and continues to do so until a valid move is returned.
position *getUserMove(const enum piece board[][SIZE], enum piece mine) {
    
    // Go until you get a user move.
    while (TRUE) {
        
        int myx, myy;
        
        // Ask for a move.
        printf("Player %c, Please enter the row(0-7) and column(0-7) of your move.\n", PIECES[mine]);
        scanf("%d%d", &myx, &myy);
        
        // Create the struct.
        position *tmp = malloc(sizeof(position));
        tmp->x = myx;
        tmp->y = myy;
        
        // Error message.
        if (!isValidMove(board, tmp, mine)) {
            printf("Sorry that is not a valid move.\n");
            free(tmp);
            continue;
        }
        
        // Or return it.
        return tmp;
    }
}

// Wrapper function to execute a move - returns false if no move was done. Otherwise, does the
// move and returns true.
enum boolean moveWrapper(enum piece board[][SIZE], const position *ptrPos, enum piece mine) {
    if (!isValidMove(board, ptrPos, mine)) return FALSE;
    executeMove(board, ptrPos, mine);
    return TRUE;
}

// Returns TRUE iff this move specified is valid.
enum boolean isValidMove(const enum piece board[][SIZE], const position *ptrPos, enum piece mine) {
    
    // Must be inbounds.
    if (!inbounds(ptrPos)) return FALSE;
    
    // Not allowing empty moves.
    if (mine == EMPTY) return FALSE;
    
    // Must be open.
    if (board[ptrPos->x][ptrPos->y] != EMPTY) return FALSE;
    
    // Try each direction.
    int dir;
    for (dir = 0; dir < NUMDIR; dir++) {
        position *tmp = getStreakEnd(board, ptrPos, dir, mine);
        if (tmp != NULL) {
            free(tmp);
            return TRUE;
        }
    }
    
    // Can't do it if we make it here.
    return FALSE;
}

// Returns an dynamically allocated array storing the possible moves and changes the integer
// pointed to by numMovesPtr to the length of the array.
position *getPossibleMoves(const enum piece board[][SIZE], enum piece mine, int *numMovesPtr) {
    
    position *moves = malloc(sizeof(position) * SIZE * SIZE);
    int i, j, posIndex = 0;
    
    // Go through all possible moves.
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            
            // Store this move in the next valid index of the array.
            moves[posIndex].x = i;
            moves[posIndex].y = j;
            
            // If it's a valid move, update posIndex so it doesn't get written over.
            if (isValidMove(board, &moves[posIndex], mine))
                posIndex++;
        }
    }
    
    // Reallocate the array to be the right size.
    moves = realloc(moves, sizeof(position) * posIndex);
    
    // Store this.
    *numMovesPtr = posIndex;
    
    // This is what we return.
    return moves;
}

// Returns true iff mine has at least one valid move on board.
enum boolean canMove(const enum piece board[][SIZE], enum piece mine) {
    
    // Generate and free the move list, storing the number of moves.
    int moveCnt;
    position *list = getPossibleMoves(board, mine, &moveCnt);
    if (moveCnt > 0) free(list);
    
    // This is when we can move.
    return moveCnt > 0;
}

// Returns FALSE iff at least one team can move, TRUE otherwise.
enum boolean gameOver(const enum piece board[][SIZE]) {
    if (canMove(board, WHITE)) return FALSE;
    return !canMove(board, BLACK);
}

// Returns the number of pieces on board equal to mine.
int score(const enum piece board[][SIZE], enum piece mine) {
    
    // Go through and add up all the pieces equal to mine.
    int i, j, res = 0;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            if (board[i][j] == mine)
                res++;
    
    return res;
}

// Retuns NULL if there is no valid move for mine starting from ptrPos in the direction direction.
// Otherwise, creates and returns a new position struct storing the end of the streak, guaranteed to
// be a piece of mine.
position *getStreakEnd(const enum piece board[][SIZE], const position *ptrPos, int direction, enum piece mine) {
    
    // Get first position in this direction.
    position *cur = getPosInDir(ptrPos, direction);
    
    // Piece we are looking for in streak.
    enum piece other = opposite(mine);
    
    // This can't take you out of bounds, or be anything but the opposite color.
    if (!inbounds(cur) || board[cur->x][cur->y] != other) {
        free(cur);
        return NULL;
    }
    
    // Now keep on going in this direction and streak.
    while (board[cur->x][cur->y] == other) {
        position *next = getPosInDir(cur, direction);
        free(cur);
        cur = next;
        if (!inbounds(cur)) break;
    }
    
    // Can't go off the board or be empty.
    if (!inbounds(cur) || board[cur->x][cur->y] == EMPTY) {
        free(cur);
        return NULL;
    }
    
    // Return end of streak.
    return cur;
}

// Assumes that the move specified is valid. Executes placing mine on board at the position
// pointed to by ptrPos.
void executeMove(enum piece board[][SIZE], const position *ptrPos, enum piece mine) {
    
    // Try each direction.
    int dir;
    for (dir = 0; dir < NUMDIR; dir++) {
        
        // Get the end of the streak in this direction.
        position *tmpPtr = getStreakEnd(board, ptrPos, dir, mine);
        if (tmpPtr == NULL) continue;
        
        // Flip the pieces in between ptrPos and tmp.
        flipPieces(board, ptrPos, tmpPtr, dir, mine);
        
        // Place my piece at the end of the streak.
        board[ptrPos->x][ptrPos->y] = mine;
        
        // Don't need this any more.
        free(tmpPtr);
    }
    
}

void flipPieces(enum piece board[][SIZE], const position *ptrStart, const position *ptrEnd, int dir, enum piece mine) {
    
    // First position to flip.
    position *curPos = getPosInDir(ptrStart, dir);
    
    // Keep on going till we get to the end.
    while (!equal(curPos, ptrEnd)) {
        
        // Flip this piece.
        board[curPos->x][curPos->y] = mine;
        
        // Go to the next piece in this direction.
        position *nextPos = getPosInDir(curPos, dir);
        free(curPos);
        curPos = nextPos;
    }
}

// Returns the number of pieces on board equal to mine.
int count(enum piece board[][SIZE], enum piece mine) {
    
    // Go through all squares and count.
    int i, j, res = 0;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            if (board[i][j] == mine)
                res++;
    
    // This is our count.
    return res;
}

// Copies the contents of sourceBoard into destBoard.
void copy(enum piece destBoard[][SIZE], const enum piece sourceBoard[][SIZE]) {
    int i, j;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            destBoard[i][j] = sourceBoard[i][j];
}
