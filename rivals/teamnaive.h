/*
 * COP3502H Final Project
 * Team 03
 */

#ifndef TEAMNAIVE_H
#define TEAMNAIVE_H

#include <stdlib.h>
#include "../src/reversi_functions.h"
#include "../src/team03.h"


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
position *teamnaiveMove(const enum piece board[][SIZE], enum piece mine, int secondsleft);

/**
 * Performs a move for our team.
 * @param board the current board state
 * @param color the color we're playing (0/1 for black/white)
 * @param time the time (in seconds) our team has left on the game timer
 * @return the position we placed a piece at
 */
pos_t teamnaive_getMove(board_t state, int color, int time);



/*
 **********************
 * Solv'n stuf        *
 **********************
 */

int teamnaive_evalBoardNaive(board_t state, int color);

solvePair_t teamnaive_solveBoard(board_t state, int color, int layer);



/*
 **********************
 * Board state utils  *
 **********************
 */

/**
 * Loads the default/slow board state into our board data type.
 * @return the translated board state.
 */
board_t teamnaive_loadBoard(const enum piece board[][SIZE]);

/**
 * Prints the state of the board.
 * @param state the board state
 */
void teamnaive_print(board_t state);

/**
 * Returns a mask containing on bits in every position where there
 * is a piece of the given color.
 * @param state the board state to check
 * @param color the piece color to look for
 * @return the described mask
 */
uint64_t teamnaive_getPieces(board_t state, int color);

/**
 * Checks if there is a piece at the position and returns its color
 * if there is.
 * @param state the board state
 * @param pos the position to check
 * @return the color of the piece (1 or 0) if there is one present; otherwise, -1.
 */
int teamnaive_getPiece(board_t state, pos_t pos);

/**
 * Checks if there is a piece of any color at the given position.
 * @param state the board state
 * @param pos the position to check
 * @return 1 if there is a piece at the given position; otherwise 0
 */
int teamnaive_hasPiece(board_t state, pos_t pos);

/**
 * Gets the color of a piece at the given position. If there is no
 * piece at that position, the returned value is indeterminate.
 * @param state the current board state
 * @param pos the position to look at
 * @return the color of the piece at the given position (0/1 for black/white)
 */
int teamnaive_getColor(board_t state, pos_t pos);

/**
 * Sets the piece at the given position to the given color.
 * If there was no piece, places one first.
 * @param state the board state to update
 * @param pos the position to set the piece at
 * @param color the color to set the piece to
 */
void teamnaive_setPiece(board_t *state, pos_t pos, int color);

/**
 * Counts all pieces on the board with the given color.
 * @param state the current board state
 * @param color the color to count (0/1 for black/white)
 * @return the number of pieces on the board with the given color
 */
int teamnaive_count(board_t state, int color);

/**
 * Checks if the given board states are equal.
 * @param state1 the first board position
 * @param state2 the second board position
 * @return 1 if the board states are the same; 0 otherwise.
 */
int teamnaive_boardEquals(board_t state1, board_t state2);

/**
 * Checks if the states of the given positions are equal.
 * @param pos1 the first position to check
 * @param pos2 the second position to check
 * @return 1 if the cells have the same state (on-ness and color);
 * 0 otherwise.
 */
int teamnaive_pieceEquals(board_t state, pos_t pos1, pos_t pos2);


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
pos_t teamnaive_makePos(int8_t row, int8_t col);

/**
 * Checks whether the position is within the bounds of the board.
 * @param pos the position to check
 * @return 1 if the position is in bounds; 0 otherwise
 */
int teamnaive_inBounds(pos_t pos);

/**
 * Gets the flat bit index for the given position.
 * @param pos the position
 * @return an integer in [0, 63]: the index of the bit corresponding
 * to the given position
 */
int8_t teamnaive_getIndexByPos(pos_t pos);

/**
 * Computes a bitmask for the move between start and end, inclusive.
 * Assumes they are on the same horizontal, vertical or diagonal axis.
 * @param start the start position of the range
 * @param end the end position of the range
 * @return a mask with the bits in the described range asserted
 */
uint64_t teamnaive_getMoveMask(pos_t start, pos_t end);


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
 * @param color the color to check moves for
 * @param num output pointer for the array size
 * @return a pointer to the (dynamically allocated!) result array
 */
pos_t *teamnaive_getMoves(board_t state, int color, int *num);

/**
 * Executes the described move, returning the newly updated board state.
 * If the move is not valid, the returned board will equal the original
 * state.
 * @param state the current board state
 * @param pos the position to play at
 * @param color the color (0/1) of the piece to place
 * @return the board state after making the given move
 */
board_t teamnaive_executeMove(board_t state, pos_t pos, int color);

/**
 * Executes part of a move in the given direction. If there is a valid
 * range that would be flipped by this move, modifies the board state
 * to reflect that change.
 * @param state the board state to (maybe) update
 * @param start the position of the move
 * @param color the color being played
 * @param dy row/y component of the direction for this partial move
 * @param dy column/x component of the direction for this partial move
 */
void teamnaive_executeMovePartial(board_t *state, pos_t start, int color, int8_t dy, int8_t dx);

/**
 * Flips the pieces between the provided start and end positions, inclusive.
 * Assumes all pieces in the range are present.
 * @param state the board state to update
 * @param start the start position of the run to flip
 * @param end the end position of the run to flip
 */
void teamnaive_flipPieces(board_t *state, pos_t start, pos_t end);

/**
 * Sets the pieces between the provided start and end positions, inclusive,
 * to the desired color. Also places pieces if any cells in range are empty.
 * @param state the board state to update
 * @param start the start position of the run to set
 * @param end the end position of the run to set
 * @param color the color to set the pieces to
 */
void teamnaive_setPieces(board_t *state, pos_t start, pos_t end, int color);

/**
 * Create pair of position and score to return from recursive solve function.
  * @param pos the position to return
  * @param score the score to return
  */
solvePair_t teamnaive_makeSolvePair(pos_t pos, int score);


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
int8_t teamnaive_getIndex(int8_t y, int8_t x);

/**
 * Checks whether the given bit is on.
 * @param mask the bitmask to check
 * @param ind the index to check in the integer
 * @return 1 if the bit at the given index is on; 0 otherwise.
 */
int teamnaive_getBit(uint64_t mask, int8_t ind);

/**
 * Sets the bit at the given index to the given value.
 * @param mask the mask to operate on
 * @param ind the index of the bit to modify
 * @param value 0 to set the bit off; any other value to set it on
 */
void teamnaive_setBit(uint64_t *mask, int8_t ind, int value);

/**
 * Creates a mask with a range of bits between `start` and `end` asserted.
 * @param start the start (from bit 0->) of the range to assert
 * @param offset the (inclusive) end of the range to assert
 * @return the described mask
 */
uint64_t teamnaive_rangeMask(int8_t start, int8_t end);

/**
 * Counts the number of set bits in the given integer.
 * @param num the integer
 * @return the number of bits that are on in `num`
 */
int teamnaive_popcount(uint64_t num);

#endif // TEAMNAIVE_H
