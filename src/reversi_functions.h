#ifndef REVERSI_FUNCTIONS_H_INCLUDED
#define REVERSI_FUNCTIONS_H_INCLUDED

#define SIZE 8
#define NUMDIR 8

enum piece {EMPTY, WHITE, BLACK};
enum boolean {FALSE, TRUE};

typedef struct position {
    int x;
    int y;
} position;

void initBoard(enum piece board[][SIZE]);
void printBoard(enum piece board[][SIZE]);
enum boolean inbounds(const position* ptrPos);
enum piece opposite(enum piece mine);
position* getPosInDir(const position* ptrPos, int direction);
enum boolean equal(const position* ptrP1, const position* ptrP2);
enum boolean isValidMove(const enum piece board[][SIZE], const position* ptrPos, enum piece mine);
position* getStreakEnd(const enum piece board[][SIZE], const position* ptrPos, int direction, enum piece mine);
void executeMove(enum piece board[][SIZE], const position* ptrPos, enum piece mine);
void flipPieces(enum piece board[][SIZE], const position* ptrStart, const position* ptrEnd, int dir, enum piece mine);

position* getUserMove(const enum piece board[][SIZE], enum piece mine);
enum boolean moveWrapper(enum piece board[][SIZE], const position* ptrPos, enum piece mine);
position* getPossibleMoves(const enum piece board[][SIZE], enum piece mine, int* numMovesPtr);
enum boolean canMove(const enum piece board[][SIZE], enum piece mine);
enum boolean gameOver(const enum piece board[][SIZE]);
int score(const enum piece board[][SIZE], enum piece mine);

int count(enum piece board[][SIZE], enum piece mine);
void copy(enum piece destBoard[][SIZE], const enum piece sourceBoard[][SIZE]);

#endif // REVERSI_FUNCTIONS_H_INCLUDED
