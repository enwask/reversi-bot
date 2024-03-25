/*
 * COP3502H Final Project
 * Team 03
 */

#ifndef TEAM03_H
#define TEAM03_H

#include <stdlib.h>
#include "../framework/reversi_functions.h"

/**
 * Fast board data type.
 * <br/><br/>
 *
 * The first integer stores, for every cell, whether there is a piece in the
 * cell at that index (0-64 from top to bottom, left to right).
 *
 * The second integer stores, for each cell with a piece in it, the color of
 * that cell. The bits at any other (empty) positions are meaningless.
 * <br/><br/>
 *
 * In theory, this gives us really fast lookups and flips, and a has a few
 * nice perks. For instance, we can enumerate pieces of a given color pretty
 * much instantly, by using the first integer as a mask and ANDing it with
 * the second integer (inverting if we're interested in black pieces instead
 * of white). <br/><br/>
 *
 * This gives us fast lookups and flips in most cases; all our manipulations
 * should be vastly faster than the default implementations with their extra
 * overhead.
 */
typedef struct board {
    int64_t on, color;
} board_t;

/**
 * Lightweight position struct (position pointers are ugly)
 * Also row before col is easier to use in a lot of the function contexts
 */
typedef struct pos {
    int8_t y, x;
} pos_t;

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

/**
 * Loads the default/slow board state into our board data type.
 * @return the translated board state.
 */
board_t team03_loadBoard(const enum piece board[][SIZE]);

/**
 * Checks if there is a piece of any color at the given position.
 * @param state the board state
 * @param pos the position to check
 * @return 1 if there is a piece at the given position; otherwise 0
 */
int team03_getPiece(board_t state, pos_t pos);

/**
 * Gets the color of a piece at the given position. If there is no
 * piece at that position, the returned value is indeterminate.
 * @param state the current board state
 * @param pos the position to look at
 * @return the color of the piece at the given position (0/1 for black/white)
 */
int team03_getColor(board_t state, pos_t pos);

/**
 * Prints the state of the board.
 * @param state the board state
 */
void team03_print(board_t state);

/**
 * Counts all pieces on the board with the given color.
 * @param state the current board state
 * @param col the color to count (0/1 for black/white)
 * @return the number of pieces on the board with the given color
 */
int team03_count(board_t state, int col);

/**
 * Checks if the states of the given positions are equal.
 * @return 1 if the cells have the same state; 0 otherwise.
 */
int team03_equals(board_t state, pos_t pos1, pos_t pos2);

/**
 * Checks whether the given piece color can be played anywhere on
 * the board.
 * @param state the current board state
 * @param col the color of the piece to check moves for
 * @return 1 if there is at least one move for this color; 0 otherwise
 */
int team03_canMove(board_t state, int col);

/**
 * Checks if the game is over (i.e. neither color has a move they
 * can make anywhere on the board).
 * @param state the current board state
 * @return 1 if there is no move available for either team; 0 otherwise
 */
int team03_isGameOver(board_t state);

/**
 * Returns a (dynamic) array of all possible move positions for the
 * given color. The size is set in `num`.
 * @param state the current board state
 * @param col the color to check moves for
 * @param num output pointer for the array size
 * @return a pointer to the (dynamically allocated!) result array
 */
position *team03_getMoves(board_t state, int col, int *num);

/**
 * Checks if the given move is valid on the current board state.
 * @param state the board state
 * @param pos the position to play at
 * @param col the color (0/1 for black/white) to play at this position
 * @return 1 if this is a valid move, 0 otherwise.
 */
int team03_isValidMove(board_t state, pos_t pos, int col);

/**
 * Tries to execute the given move, if it is valid for the current
 * board state. Returns whether the move was successful.
 * @param state the current board state
 * @param pos the position to play at
 * @param col the color of the piece to play
 * @return 1 if the move was executed successfully; 0 otherwise
 */
int team03_tryMove(board_t *state, pos_t pos, int col);

/**
 * Executes the described move, modifying the provided state.
 * Assumes the move is valid.
 * @param state a pointer to the board state to update
 * @param pos the position to play at
 * @param col the color (0/1) of the piece to place
 */
void team03_executeMove(board_t *state, pos_t pos, int col);

/**
 * Flips the pieces between the provided start and end positions, inclusive.
 * <br/><br/>
 * Defers to `team03_doFlip`
 * @param state the board state to update
 * @param start the start position of the run to flip
 * @param end the end position of the run to flip
 */
void team03_flip(board_t *state, pos_t start, pos_t end);

/**
 * Flips the pieces between the start and end indices, inclusive.
 * @param state the board state to update
 * @param start the start index of the run to flip
 * @param end the end index of the run to flip
 */
void team03_doFlip(board_t *state, uint8_t start, uint8_t end);

/**
 * Creates a pos_t instance from y and x positions.
 * @param y the y position
 * @param x the x position
 * @return
 */
pos_t team03_makePos(int8_t y, int8_t x);

/**
 * Gets the flat bit index for the given position.
 * @param pos the position
 * @return an integer in [0, 63]: the index of the bit corresponding
 * to the given position
 */
uint8_t team03_getPosIndex(pos_t pos);

/**
 * Gets the flat bit index for the given y/x indices.
 * @return an integer in [0, 63]: the index of the bit for the given
 * position
 */
uint8_t team03_getIndex(uint8_t y, uint8_t x);

/**
 * Checks whether the given bit is on.
 * @param mask the bitmask to check
 * @param ind the index to check in the integer
 * @return 1 if the bit at the given index is on; 0 otherwise.
 */
int team03_getBit(int64_t mask, uint8_t ind);

/**
 * Sets the bit at the given index to the given value.
 * @param mask the mask to operate on
 * @param ind the index of the bit to modify
 * @param value 0 to set the bit off; any other value to set it on
 */
void team03_setBit(int64_t *mask, uint8_t ind, int value);

/**
 * Counts the number of set bits in the given integer.
 * @param num the integer
 * @return the number of bits that are on in `num`
 */
uint8_t team03_popcount(int64_t num);

#endif // TEAM03_H
