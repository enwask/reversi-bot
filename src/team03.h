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
 * Data types         *
 **********************
 */

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

/**
 * Lightweight position struct (position pointers are ugly).
 * Also row before col is easier to use in a lot of the function contexts
 * so functions using pos_t (like makePos) use this order too.
 */
typedef struct pos {
    int8_t y, x; // signed to avoid underflow issues in move loops
} pos_t;


/*
 **********************
 * Game interface     *
 **********************
 */

/**
 * Performs a move for our team. Defers to `getMove`.
 * @param board the (slow/default) board state, which we'll load to our board type
 * @param mine which piece color we're playing
 * @param secondsleft how much time we have left for the game
 * @return the position we place a piece at.
 */
position *team03Move(const enum piece board[][SIZE], enum piece mine, int secondsleft);

/**
 * Performs a move for our team.
 * @param board the current board state
 * @param col the color we're playing (0/1 for black/white)
 * @param time the time (in seconds) our team has left on the game timer
 * @return the position we placed a piece at
 */
pos_t team03_getMove(board_t state, int col, int time);


/*
 **********************
 * Board state utils  *
 **********************
 */

/**
 * Loads the default/slow board state into our board data type.
 * @return the translated board state.
 */
board_t team03_loadBoard(const enum piece board[][SIZE]);

/**
 * Prints the state of the board.
 * @param state the board state
 */
void team03_print(board_t state);

/**
 * Returns a mask containing on bits in every position where there
 * is a piece of the given color.
 * @param state the board state to check
 * @param col the piece color to look for
 * @return the described mask
 */
uint64_t team03_getPieces(board_t state, int col);

/**
 * Checks if there is a piece at the position and returns its color
 * if there is.
 * @param state the board state
 * @param pos the position to check
 * @return the color of the piece (1 or 0) if there is one present; otherwise, -1.
 */
int team03_getPiece(board_t state, pos_t pos);

/**
 * Checks if there is a piece of any color at the given position.
 * @param state the board state
 * @param pos the position to check
 * @return 1 if there is a piece at the given position; otherwise 0
 */
int team03_hasPiece(board_t state, pos_t pos);

/**
 * Gets the color of a piece at the given position. If there is no
 * piece at that position, the returned value is indeterminate.
 * @param state the current board state
 * @param pos the position to look at
 * @return the color of the piece at the given position (0/1 for black/white)
 */
int team03_getColor(board_t state, pos_t pos);

/**
 * Sets the piece at the given position to the given color.
 * If there was no piece, places one first.
 * @param state the board state to update
 */
void team03_setPiece(board_t *state, pos_t pos, int col);

/**
 * Counts all pieces on the board with the given color.
 * @param state the current board state
 * @param col the color to count (0/1 for black/white)
 * @return the number of pieces on the board with the given color
 */
int team03_count(board_t state, int col);

/**
 * Checks if the given board states are equal.
 * @param state1 the first board position
 * @param state2 the second board position
 * @return 1 if the board states are the same; 0 otherwise.
 */
int team03_boardEquals(board_t state1, board_t state2);

/**
 * Checks if the states of the given positions are equal.
 * @param pos1 the first position to check
 * @param pos2 the second position to check
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
 * @param row the row/y position
 * @param col the colummn/x position
 * @return
 */
pos_t team03_makePos(int8_t row, int8_t col);

/**
 * Checks whether the position is within the bounds of the board.
 * @param pos the position to check
 * @return 1 if the position is in bounds; 0 otherwise
 */
int team03_inBounds(pos_t pos);

/**
 * Gets the flat bit index for the given position.
 * @param pos the position
 * @return an integer in [0, 63]: the index of the bit corresponding
 * to the given position
 */
int8_t team03_getIndexByPos(pos_t pos);

/**
 * Computes a bitmask for the move between start and end, inclusive.
 * Assumes they are on the same horizontal, vertical or diagonal axis.
 * @param start the start position of the range
 * @param end the end position of the range
 * @return a mask with the bits in the described range asserted
 */
uint64_t team03_getMoveMask(pos_t start, pos_t end);


/*
 **********************
 * Game logic         *
 **********************
 */

/**
 * Returns a (dynamic) array of all possible move positions for the
 * given color. The size is set in `num`.<br/>
 * <b>These moves are not necessarily valid;</b> the only guarantee
 * is that they are empty cells adjacent to at least one cell of the
 * opposite color.
 * @param state the current board state
 * @param col the color to check moves for
 * @param num output pointer for the array size
 * @return a pointer to the (dynamically allocated!) result array
 */
pos_t *team03_getMoves(board_t state, int col, int *num);

/**
 * Executes the described move, returning the newly updated board state.
 * If the move is not valid, the returned board will equal the original
 * state.
 * @param state the current board state
 * @param pos the position to play at
 * @param col the color (0/1) of the piece to place
 * @return the board state after making the given move
 */
board_t team03_executeMove(board_t state, pos_t pos, int col);

/**
 * Executes part of a move in the given direction. If there is a valid
 * range that would be flipped by this move, modifies the board state
 * to reflect that change.
 * @param state the board state to (maybe) update
 * @param start the position of the move
 * @param col the color being played
 * @param dy row/y component of the direction for this partial move
 * @param dy column/x component of the direction for this partial move
 */
void team03_executeMovePartial(board_t *state, pos_t start, int col, int8_t dx, int8_t dy);

/**
 * Flips the pieces between the provided start and end positions, inclusive.
 * Assumes all pieces in the range are present.
 * @param state the board state to update
 * @param start the start position of the run to flip
 * @param end the end position of the run to flip
 */
void team03_flipPieces(board_t *state, pos_t start, pos_t end);

/**
 * Sets the pieces between the provided start and end positions, inclusive,
 * to the desired color. Also places pieces if any cells in range are empty.
 * @param state the board state to update
 * @param start the start position of the run to set
 * @param end the end position of the run to set
 * @param col the color to set the pieces to
 */
void team03_setPieces(board_t *state, pos_t start, pos_t end, int col);


/*
 **********************
 * Bit manipulation   *
 **********************
 */

/**
 * Gets the flat bit index for the given y/x indices.
 * @return an integer in [0, 63]: the index of the bit for the given
 * position
 */
int8_t team03_getIndex(int8_t y, int8_t x);

/**
 * Checks whether the given bit is on.
 * @param mask the bitmask to check
 * @param ind the index to check in the integer
 * @return 1 if the bit at the given index is on; 0 otherwise.
 */
int team03_getBit(uint64_t mask, int8_t ind);

/**
 * Sets the bit at the given index to the given value.
 * @param mask the mask to operate on
 * @param ind the index of the bit to modify
 * @param value 0 to set the bit off; any other value to set it on
 */
void team03_setBit(uint64_t *mask, int8_t ind, int value);

/**
 * Creates a mask with a range of bits between `start` and `end` asserted.
 * @param start the start (from bit 0->) of the range to assert
 * @param offset the (inclusive) end of the range to assert
 * @return the described mask
 */
uint64_t team03_rangeMask(int8_t start, int8_t end);

/**
 * Counts the number of set bits in the given integer.
 * @param num the integer
 * @return the number of bits that are on in `num`
 */
int8_t team03_popcount(uint64_t num);

#endif // TEAM03_H
