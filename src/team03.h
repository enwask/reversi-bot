/*
 * COP3502H Final Project
 * Team 03
 */

#ifndef TEAM03_H
#define TEAM03_H

#include <stdlib.h>
#include "../framework/reversi_functions.h"


/*
 **********************
 * Fixed-width types  *
 **********************
 */

// Define fixed-width types if missing (pre-C99)
#ifndef _UINT64_T
#   define _UINT64_T
typedef unsigned long long uint64_t;
#endif //_UINT64_T

#ifndef _INT8_T
#   define _INT8_T
typedef signed char int8_t;
#endif // _INT8_T


/*
 **********************
 * Data types         *
 **********************
 */

#ifndef BOARD_H
#define BOARD_H
/**
 * Fast board data type.
 * <br/><br/>
 *
 * The first integer stores, for every cell, whether there is a piece in the
 * cell at that index (0-63 from top to bottom, left to right).
 *
 * The second integer stores, for each cell with a piece in it, the color of
 * that cell. The bits at any other (empty) positions are meaningless.
 * <br/><br/>
 *
 * In theory, this gives us really fast lookups and flips, which should
 * directly correspond with better moves since we can traverse further
 * down the game tree.
 */
typedef struct board {
    uint64_t on, color;
} board_t;
#endif // BOARD_H

#ifndef POS_H
#define POS_H
/**
 * Lightweight position struct.
 * y=row, x=col
 */
typedef struct pos {
    int8_t y, x; // signed to avoid underflow issues in move loops
} pos_t;
#endif // POS_H

#ifndef SOLVEPAIR_H
#define SOLVEPAIR_H
/**
 * A solve result to return from move evaluation functions.
 */
typedef struct solvePair {
    pos_t pos;
    int score;
} solvePair_t;
#endif // SOLVEPAIR_H


/*
 **********************
 * Game interface     *
 **********************
 */

/**
 * Performs a move for our team. Defers to `getMove`.
 *
 * @param board the (slow/default) board state, which we'll load to our board type
 * @param mine which piece color we're playing
 * @param secondsleft how much time we have left for the game
 *
 * @return the position we place a piece at.
 */
position *team03Move(const enum piece board[][SIZE], enum piece mine, int secondsleft);

/**
 * Performs a move for our team.
 *
 * @param board the current board state
 * @param color the color we're playing (0/1 for black/white)
 * @param time the time (in seconds) our team has left on the game timer
 *
 * @return the position we placed a piece at
 */
pos_t team03_getMove(board_t state, int color, int time);


/*
 **********************
 * Heuristics         *
 **********************
 */

/**
 * Computes a rough estimate of the given color's mobility for the
 * current board state, by counting the number of cells that could
 * potentially be a valid move location (that is, empty cells that
 * are adjacent to at least one cell of the opponent's color).
 * <br/><br/>
 *
 * This estimate is definitionally an upper bound for the number
 * of valid moves.
 *
 * @param state the current board state
 * @param color the color to check mobility for
 *
 * @return the number of *potential* move locations
 */
int team03_estimateMobility(board_t state, int color);

/**
 * Computes the given color's mobility for the current board state,
 * counting the number of valid moves that color can take.
 *
 * @param state the current board state
 * @param color the color to consider moves for
 *
 * @return the number of valid moves that the color can take
 */
int team03_computeMobility(board_t state, int color);

/**
 * Statically evaluate the current board position for a given color.
 * Only accounts for the current level, disregarding future moves.
 *
 * @param state the current board state
 * @param color the color to evaluate for
 *
 * @return a relative score for the current board state
 */
int team03_evaluateStatic(board_t state, int color);


/*
 **********************
 * The actual bot lol *
 **********************
 */

pos_t team03_iterate(board_t state, int color);

// TODO: ben pls write doc comments :(
// TODO: fukc u 2: electric boogaloo
solvePair_t team03_solveBoard(board_t state, int color, int layer, int alpha, int beta);

/**
 * Statically evaluates all of color's moves for the current board
 * state, outputting a list pairing move positions with their static
 * scores in descending order of value.
 * <br/><br/>
 * If evaluate is 0, instead just gets the moves that are valid.
 *
 * @param state the current board state
 * @param color the color to check moves for
 * @param arr a pointer to an array solvePair_t[64]
 * @param evaluate whether to evaluate and sort the move list before returning
 * 
 * @return the result array size, or -1 if any move score was found outside of [min, max]
 */
int team03_getMoveScores(board_t state, int color, solvePair_t *arr, int evaluate);


/*
 **********************
 * Board state utils  *
 **********************
 */

/**
 * Loads the default/slow board state into our board data type.
 *
 * @return the translated board state.
 */
board_t team03_loadBoard(const enum piece board[][SIZE]);

/**
 * Prints the state of the board.
 *
 * @param state the board state
 */
void team03_print(board_t state);

/**
 * Returns a mask containing on bits in every position where there
 * is a piece of the given color.
 *
 * @param state the board state to check
 * @param color the piece color to look for
 *
 * @return the described mask
 */
uint64_t team03_getPieces(board_t state, int color);

/**
 * Checks if there is a piece at the position and returns its color
 * if there is.
 *
 * @param state the board state
 * @param pos the position to check
 *
 * @return the color of the piece (1 or 0) if there is one present; otherwise, -1.
 */
int team03_getPiece(board_t state, pos_t pos);

/**
 * Checks if there is a piece of any color at the given position.
 *
 * @param state the board state
 * @param pos the position to check
 *
 * @return 1 if there is a piece at the given position; otherwise 0
 */
int team03_hasPiece(board_t state, pos_t pos);

/**
 * Gets the color of a piece at the given position. If there is no
 * piece at that position, the returned value is indeterminate.
 *
 * @param state the current board state
 * @param pos the position to look at
 *
 * @return the color of the piece at the given position (0/1 for black/white)
 */
int team03_getColor(board_t state, pos_t pos);

/**
 * Sets the piece at the given position to the given color.
 * If there was no piece, places one first.
 *
 * @param state the board state to update
 * @param pos the position to set the piece at
 * @param color the color to set the piece to
 */
void team03_setPiece(board_t *state, pos_t pos, int color);

/**
 * Counts all pieces on the board with the given color.
 *
 * @param state the current board state
 * @param color the color to count (0/1 for black/white)
 *
 * @return the number of pieces on the board with the given color
 */
int team03_count(board_t state, int color);

/**
 * Checks if the given board states are equal.
 *
 * @param state1 the first board position
 * @param state2 the second board position
 *
 * @return 1 if the board states are the same; 0 otherwise.
 */
int team03_boardEquals(board_t state1, board_t state2);

/**
 * Checks if the states of the given positions are equal.
 *
 * @param pos1 the first position to check
 * @param pos2 the second position to check
 *
 * @return 1 if the cells have the same state (on-ness and color);
 * 0 otherwise.
 */
int team03_pieceEquals(board_t state, pos_t pos1, pos_t pos2);


/*
 **********************
 * Position utilities *
 **********************
 */

/**
 * Creates a pos_t instance from y and x positions.
 *
 * @param row the row/y position
 * @param col the colummn/x position
 *
 * @return
 */
pos_t team03_makePos(int8_t row, int8_t col);

/**
 * Checks whether the position is within the bounds of the board.
 *
 * @param pos the position to check
 *
 * @return 1 if the position is in bounds; 0 otherwise
 */
int team03_inBounds(pos_t pos);

/**
 * Gets the flat bit index for the given position.
 *
 * @param pos the position
 *
 * @return an integer in [0, 63]: the index of the bit corresponding to the given position
 */
int8_t team03_getIndexByPos(pos_t pos);

/**
 * Computes a bitmask for the move between start and end, inclusive.
 * Assumes they are on the same horizontal, vertical or diagonal axis.
 *
 * @param start the start position of the range
 * @param end the end position of the range
 *
 * @return a mask with the bits in the described range asserted
 */
uint64_t team03_getMoveMask(pos_t start, pos_t end);


/*
 **********************
 * Sorting & misc     *
 **********************
 */

/**
 * Sorts the subarray [lo, hi] of solve pairs in decreasing
 * order of score.
 *
 * @param pairs the array to sort
 * @param lo the leftmost index of the range to sort
 * @param hi the rightmost index of the range to sort
 */
void team03_sort(solvePair_t *pairs, int lo, int hi);

/**
 * Merges the subarrays [lo, md) and [md, hi].
 *
 * @param arr the array to operate on
 * @param lo the leftmost index of the range to merge
 * @param md the middle index of the range to merge
 * @param hi the rightmost index of the range to merge
 */
void team03_merge(solvePair_t *arr, int lo, int md, int hi);

/*
 **********************
 * Game logic         *
 **********************
 */

/**
 * Checks if the described move is valid. Implementation is identical
 * to `executeMove` except that we break early if any one of the
 * directions we're looking in is a valid flip.
 *
 * @param state the current board state
 * @param pos the position to try playing at
 * @param color the color (0/1) of the piece to place
 */
int team03_isValidMove(board_t state, pos_t pos, int color);

/**
 * Executes the described move, returning the newly updated board state.
 * If the move is not valid, the returned board will equal the original
 * state.
 *
 * @param state the current board state
 * @param pos the position to play at
 * @param color the color (0/1) of the piece to place
 *
 * @return the board state after making the given move
 */
board_t team03_executeMove(board_t state, pos_t pos, int color);

/**
 * Executes part of a move in the given direction. If there is a valid
 * range that would be flipped by this move, modifies the board state
 * to reflect that change.
 *
 * @param state the board state to (maybe) update
 * @param start the position of the move
 * @param color the color being played
 * @param dy row/y component of the direction for this partial move
 * @param dy column/x component of the direction for this partial move
 */
void team03_executeMovePartial(board_t *state, pos_t start, int color, int8_t dy, int8_t dx);

/**
 * Flips the pieces between the provided start and end positions, inclusive.
 * Assumes all pieces in the range are present.
 *
 * @param state the board state to update
 * @param start the start position of the run to flip
 * @param end the end position of the run to flip
 */
void team03_flipPieces(board_t *state, pos_t start, pos_t end);

/**
 * Sets the pieces between the provided start and end positions, inclusive,
 * to the desired color. Also places pieces if any cells in range are empty.
 *
 * @param state the board state to update
 * @param start the start position of the run to set
 * @param end the end position of the run to set
 * @param color the color to set the pieces to
 */
void team03_setPieces(board_t *state, pos_t start, pos_t end, int color);

/**
 * Create pair of position and score to return from recursive solve function.
 *
  * @param pos the position to return
  * @param score the score to return
  *
  * @return the pair, duh
  */
solvePair_t team03_makeSolvePair(pos_t pos, int score);


/*
 **********************
 * Bit manipulation   *
 **********************
 */

/**
 * Gets the flat bit index for the given y/x indices.
 *
 * @return an integer in [0, 63]: the index of the bit for the given position
 */
int8_t team03_getIndex(int8_t y, int8_t x);

/**
 * Checks whether the given bit is on.
 *
 * @param mask the bitmask to check
 * @param ind the index to check in the integer
 *
 * @return 1 if the bit at the given index is on; 0 otherwise.
 */
int team03_getBit(uint64_t mask, int8_t ind);

/**
 * Sets the bit at the given index to the given value.
 *
 * @param mask the mask to operate on
 * @param ind the index of the bit to modify
 * @param value 0 to set the bit off; any other value to set it on
 */
void team03_setBit(uint64_t *mask, int8_t ind, int value);

/**
 * Convenience function for checking whether a bit is on by
 * its position.
 *
 * @param mask the mask to check
 * @param pos the position of the bit to check
 *
 * @return 1 if the bit at pos is on, 0 otherwise
 */
int team03_getBitAt(uint64_t mask, pos_t pos);

/**
 * Convenience function for setting the bit by its position.
 *
 * @param mask a pointer to the mask to modify
 * @param pos the position of the bit to set
 * @param value 0 to turn the bit off; any other value to turn it on
 */
void team03_setBitAt(uint64_t *mask, pos_t pos, int value);

/**
 * Creates a mask with a range of bits between `start` and `end` asserted.
 *
 * @param start the start (from bit 0->) of the range to assert
 * @param offset the (inclusive) end of the range to assert
 *
 * @return the described mask
 */
uint64_t team03_rangeMask(int8_t start, int8_t end);

/**
 * Counts the number of set bits in the given integer.
 *
 * @param num the integer
 *
 * @return the number of bits that are on in `num`
 */
int team03_popcount(uint64_t num);

#endif // TEAM03_H
