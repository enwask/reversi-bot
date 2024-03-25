#include <stdio.h>
#include <stdlib.h>
#include "team03.h"

/**
 * Performs a move for our team. Defers to `getMove`.
 * @param board the (slow/default) board state, which we'll load to our board type
 * @param mine which piece color we're playing
 * @param secondsleft how much time we have left for the game
 * @return the position we place a piece at.
 */
position *team03Move(const enum piece board[][SIZE], enum piece mine, int secondsleft) {
    // Translate stuff
    board_t state = team03_loadBoard(board);
    int col = (mine == WHITE);
    
    // Perform the move with our own state format
    pos_t pos = team03_getMove(state, col, secondsleft);
    
    // Dynamically allocate the return position
    position *res = malloc(sizeof(position));
    res->x = pos.x, res->y = pos.y;
    
    // Return the position (duh)
    return res;
}

/**
 * Performs a move for our team.
 * @param board the current board state
 * @param col the color we're playing (0/1 for black/white)
 * @param time the time (in seconds) our team has left on the game timer
 * @return the position we placed a piece at
 */
pos_t team03_getMove(board_t state, int col, int time) {
    team03_print(state);
    printf("\nBlack has %d pieces\nWhite has %d pieces\n\n", team03_count(state, 0), team03_count(state, 1));
    return team03_makePos(0, 0);
}

/**
 * Loads the default/slow board state into our board data type.
 * @return the translated board state.
 */
board_t team03_loadBoard(const enum piece board[][SIZE]) {
    // Initialize an empty board
    board_t res;
    res.on = res.color = 0;
    
    // Iterate over board positions
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            // Operate on the cell at this index
            uint8_t ind = team03_getIndex(i, j);
            switch (board[i][j]) {
                case EMPTY: break;
                case WHITE: team03_setBit(&res.color, ind, 1); // fall through to set res.on bit
                case BLACK: team03_setBit(&res.on, ind, 1); // don't need to set color for black
                
            }
        }
    }
    
    // Return the translated board state
    return res;
}

/**
 * Checks if there is a piece of any color at the given position.
 * @param state the board state
 * @param pos the position to check
 * @return 1 if there is a piece at the given position; otherwise 0
 */
int team03_getPiece(board_t state, pos_t pos) {
    int8_t ind = team03_getPosIndex(pos);
    return team03_getBit(state.on, ind);
}

/**
 * Gets the color of a piece at the given position. If there is no
 * piece at that position, the returned value is indeterminate.
 * @param state the current board state
 * @param pos the position to look at
 * @return the color of the piece at the given position (0/1 for black/white)
 */
int team03_getColor(board_t state, pos_t pos) {
    int8_t ind = team03_getPosIndex(pos);
    return team03_getBit(state.color, ind);
}

/**
 * Prints the state of the board.
 * @param state the board state
 */
void team03_print(board_t state) {
    // Print column headers
    printf("  ");
    for (uint8_t i = 0; i < 8; i++)
        printf("%d ", i);
    printf("\n");
    
    
    // Iterate over board cells
    for (uint8_t i = 0; i < 8; i++) {
        // Print the row header
        printf("%d ", i);
        
        for (uint8_t j = 0; j < 8; j++) {
            // Print this cell
            uint8_t ind = team03_getIndex(i, j);
            if (!team03_getBit(state.on, ind)) printf(". "); // empty cell
            else {
                // Non-empty cell
                int col = team03_getBit(state.color, ind);
                printf("%c ", col ? 'X' : 'O'); // print the cell color
            }
        }
        printf("\n"); // end of this row
    }
    printf("\n"); // padding
}

/**
 * Counts all pieces on the board with the given color.
 * @param state the current board state
 * @param col the color to count (0/1 for black/white)
 * @return the number of pieces on the board with the given color
 */
int team03_count(board_t state, int col) {
    // Compute a mask of the pieces we're looking for
    int64_t color_mask = col ? state.color : ~state.color;
    int64_t mask = state.on & color_mask;
    
    // Count the number of set bits in our mask
    return team03_popcount(mask);
}

/**
 * Checks if the states of the given positions are equal.
 * @return 1 if the cells have the same state; 0 otherwise.
 */
int team03_equals(board_t state, pos_t pos1, pos_t pos2) {
    int a = team03_getPiece(state, pos1);
    int b = team03_getPiece(state, pos2);
    
    if (a ^ b) return 0; // if the on-ness differs, then not equal
    if (!a) return 1; // if both are off, we don't care about the color
    
    // Otherwise true if the colors are equal
    return team03_getColor(state, pos1) ^
           team03_getColor(state, pos1);
}

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
pos_t *team03_getMoves(board_t state, int col, int *num);

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
pos_t team03_makePos(int8_t y, int8_t x) {
    pos_t res;
    res.y = y, res.x = x;
    return res;
}

/**
 * Gets the flat bit index for the given position.
 * @param pos the position
 * @return an integer in [0, 63]; the index of the bit corresponding
 * to the given position
 */
uint8_t team03_getPosIndex(pos_t pos) { return team03_getIndex(pos.y, pos.x); }

/**
 * Gets the flat bit index for the given y/x indices.
 * @return an integer in [0, 63]: the index of the bit for the given
 * position
 */
uint8_t team03_getIndex(uint8_t y, uint8_t x) { return y * 8 + x; }

/**
 * Checks whether the given bit is on.
 * @param mask the bitmask to check
 * @param ind the index to check in the integer
 * @return 1 if the bit at the given index is on; 0 otherwise.
 */
int team03_getBit(int64_t mask, uint8_t ind) {
    uint64_t check = ((uint64_t) 1) << ind;
    return (mask & check) != 0;
}

/**
 * Sets the bit at the given index to the given value.
 * @param mask the mask to operate on
 * @param ind the index of the bit to modify
 * @param value 0 to set the bit off; any other value to set it on
 */
void team03_setBit(int64_t *mask, uint8_t ind, int value) {
    uint64_t set = ((uint64_t) 1) << ind;
    
    if (value) *mask |= set;
    else *mask &= ~set;
}

/**
 * Counts the number of set bits in the given integer.
 * @param num the integer
 * @return the number of bits that are on in `num`
 */
uint8_t team03_popcount(int64_t num) {
#ifdef __has_builtin
#if __has_builtin(__builtin_popcount)
#define popcount(x) __builtin_popcount(x)
#endif // has_builtin
#endif // ifdef
#ifdef popcount
    // If GCC's __builtin_popcount is available, we use that as
    // it's often a single machine-level instruction
    return (uint8_t) popcount(num);
#else
    // Otherwise we do it manually
    // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
    uint8_t res = 0;
    for (; num; res++) num &= num - 1;
    return res;
#endif
}
