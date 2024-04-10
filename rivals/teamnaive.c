#include <stdio.h>
#include <stdlib.h>
#include "teamnaive.h"
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
position *teamnaiveMove(const enum piece board[][SIZE], enum piece mine, int secondsleft) {
    // Translate stuff
    board_t state = teamnaive_loadBoard(board);
    int color = (mine == WHITE);
    
    // Perform the move with our own state format
    pos_t pos = teamnaive_getMove(state, color, secondsleft);
    
    // Dynamically allocate the return position
    position *res = malloc(sizeof(position));
    res->x = pos.y, res->y = pos.x;
    
    // Return the position (duh)
    return res;
}

/**
 * Performs a move for our team.
 * @param board the current board state
 * @param color the color we're playing (0/1 for black/white)
 * @param time the time (in seconds) our team has left on the game timer
 * @return the position we placed a piece at
 */
pos_t teamnaive_getMove(board_t state, int color, int time) {
    //teamnaive_print(state);
    //printf("\nBlack has %d pieces\nWhite has %d pieces\n\n", teamnaive_count(state, 0), teamnaive_count(state, 1));
    solvePair_t pair = teamnaive_solveBoard(state, color, 4);
    return pair.pos;
}

/*
**********************
* Solv'n stuf        *
**********************
*/


int teamnaive_evalBoard(board_t state, int color) {
    // Inverted mobility heuristic so we try to pass
    return team03_computeMobility(state, !color)
           - team03_computeMobility(state, color) * 8
           + rand() % 2;
}

solvePair_t teamnaive_solveBoard(board_t state, int color, int layer) {
    if (layer == 0) {
        int score = teamnaive_evalBoard(state, color);
        pos_t pos = teamnaive_makePos(-1, -1);
        return teamnaive_makeSolvePair(pos, score);
    }
    
    int best = -1e9;
    pos_t bestPos = teamnaive_makePos(0, 0);
    int iCanPlay = 0;
    int otherCanPlay = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            pos_t pos = teamnaive_makePos(r, c);
            if (!teamnaive_boardEquals(state, teamnaive_executeMove(state, pos, color ^ 1))) otherCanPlay = 1;
            board_t nState = teamnaive_executeMove(state, pos, color);
            if (teamnaive_boardEquals(state, nState)) continue;
            iCanPlay = 1;
            solvePair_t pair = teamnaive_solveBoard(nState, color ^ 1, layer - 1);
            int score = 0 - pair.score;
            //printf("%d\n", score);
            
            if (score > best) {
                best = score;
                bestPos = pos;
            }
        }
    }
    
    if (!iCanPlay) {
        if (otherCanPlay) {
            solvePair_t ret = teamnaive_solveBoard(state, color ^ 1, layer - 1);
            ret.score = 0 - ret.score;
            return ret;
        } else {
            int score;
            if (teamnaive_count(state, color) > teamnaive_count(state, color ^ 1)) score = 1e8;
            else if (teamnaive_count(state, color) < teamnaive_count(state, color ^ 1)) score = -1e8;
            else score = 0;
            pos_t pos = teamnaive_makePos(-1, -1);
            solvePair_t ret = teamnaive_makeSolvePair(pos, score);
            return ret;
        }
    }
    
    //printf("%d %d\n", best, layer);
    
    solvePair_t pair = teamnaive_makeSolvePair(bestPos, best);
    return pair;
    
}





/*
 **********************
 * Board state utils  *
 **********************
 */

/**
 * Loads the default/slow board state into our board data type.
 * @return the translated board state.
 */
board_t teamnaive_loadBoard(const enum piece board[][SIZE]) {
    // Initialize an empty board
    board_t res;
    res.on = res.color = 0;
    
    // Iterate over board positions
    for (int8_t i = 0; i < 8; i++) {
        for (int8_t j = 0; j < 8; j++) {
            // Operate on the cell at this index
            int8_t ind = teamnaive_getIndex(i, j);
            switch (board[i][j]) {
                case EMPTY: break;
                case WHITE: teamnaive_setBit(&res.color, ind, 1); // fall through to set res.on bit
                case BLACK: teamnaive_setBit(&res.on, ind, 1); // don't need to set color for black
                
            }
        }
    }
    
    // Return the translated board state
    return res;
}

/**
 * Prints the state of the board.
 * @param state the board state
 */
void teamnaive_print(board_t state) {
    // Print column headers
    printf("  ");
    for (int8_t i = 0; i < 8; i++)
        printf("%d ", i);
    printf("\n");
    
    // Iterate over board cells
    for (int8_t i = 0; i < 8; i++) {
        // Print the row header
        printf("%d ", i);
        
        for (int8_t j = 0; j < 8; j++) {
            // Print this cell
            int8_t ind = teamnaive_getIndex(i, j);
            if (!teamnaive_getBit(state.on, ind)) printf(". "); // empty cell
            else {
                // Non-empty cell
                int color = teamnaive_getBit(state.color, ind);
                printf("%c ", color ? 'O' : 'X'); // print the cell color
            }
        }
        printf("\n"); // end of this row
    }
    printf("\n"); // padding
}

/**
 * Returns a mask containing on bits in every position where there
 * is a piece of the given color.
 * @param state the board state to check
 * @param color the piece color to look for
 * @return the described mask
 */
uint64_t teamnaive_getPieces(board_t state, int color) {
    // Compute a mask for the requested color
    uint64_t color_mask = color ? state.color : ~state.color;
    return state.on & color_mask; // filter placed pieces by the color
}

/**
 * Checks if there is a piece at the position and returns its color
 * if there is.
 * @param state the board state
 * @param pos the position to check
 * @return the color of the piece (1 or 0) if there is one present; otherwise, -1.
 */
int teamnaive_getPiece(board_t state, pos_t pos) {
    if (!teamnaive_hasPiece(state, pos)) return -1;
    return teamnaive_getColor(state, pos);
}

/**
 * Checks if there is a piece of any color at the given position.
 * @param state the board state
 * @param pos the position to check
 * @return 1 if there is a piece at the given position; otherwise 0
 */
int teamnaive_hasPiece(board_t state, pos_t pos) {
    int8_t ind = teamnaive_getIndexByPos(pos);
    return teamnaive_getBit(state.on, ind);
}

/**
 * Gets the color of a piece at the given position. If there is no
 * piece at that position, the returned value is indeterminate.
 * @param state the current board state
 * @param pos the position to look at
 * @return the color of the piece at the given position (0/1 for black/white)
 */
int teamnaive_getColor(board_t state, pos_t pos) {
    int8_t ind = teamnaive_getIndexByPos(pos);
    return teamnaive_getBit(state.color, ind);
}

/**
 * Sets the piece at the given position to the given color.
 * If there was no piece, places one first.
 * @param state the board state to update
 * @param pos the position to set the piece at
 * @param color the color to set the piece to
 */
void teamnaive_setPiece(board_t *state, pos_t pos, int color) {
    int8_t ind = teamnaive_getIndexByPos(pos);
    teamnaive_setBit(&state->on, ind, 1);
    teamnaive_setBit(&state->color, ind, color);
}

/**
 * Counts all pieces on the board with the given color.
 * @param state the current board state
 * @param color the color to count (0/1 for black/white)
 * @return the number of pieces on the board with the given color
 */
int teamnaive_count(board_t state, int color) {
    // Count the number of set bits in the mask for this piece color
    return teamnaive_popcount(teamnaive_getPieces(state, color));
}

/**
 * Checks if the given board states are equal.
 * <p>
 * TODO: Something to keep in mind here:
     * If the garbage values in the color mask (where the associated
     * bits in `state.on` are off) differ, this function will return
     * false.<br/>
     *
     * <b>If we end up doing manipulations in other code that might
     * modify colors at positions where pieces are not present</b>,
     * this function should be changed to check the masks for each
     * color ANDed with the on masks.<br/>
     *
     * For now we don't ever change color bits from 0 if there hasn't
     * been a piece placed at them, so we should be fine.
 * </p>
 * @param state1 the first board position
 * @param state2 the second board position
 * @return 1 if the board states are the same; 0 otherwise.
 */
int teamnaive_boardEquals(board_t state1, board_t state2) {
    return state1.on == state2.on && state1.color == state2.color;
}

/**
 * Checks if the states of the given positions are equal.
 * @return 1 if the cells have the same state; 0 otherwise.
 */
int teamnaive_pieceEquals(board_t state, pos_t pos1, pos_t pos2) {
    int a = teamnaive_hasPiece(state, pos1);
    int b = teamnaive_hasPiece(state, pos2);
    
    if (a ^ b) return 0; // if the on-ness differs, then not equal
    if (!a) return 1; // if both are off, we don't care about the color
    
    // Otherwise true if the colors are equal
    return teamnaive_getColor(state, pos1)
           == teamnaive_getColor(state, pos2);
}


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
pos_t teamnaive_makePos(int8_t row, int8_t col) {
    pos_t res;
    res.y = row, res.x = col;
    return res;
}

/**
 * Checks whether the position is within the bounds of the board.
 * @param pos the position to check
 * @return 1 if the position is in bounds; 0 otherwise
 */
int teamnaive_inBounds(pos_t pos) {
    return pos.y >= 0 && pos.y < 8 && pos.x >= 0 && pos.x < 8;
}

/**
 * Gets the flat bit index for the given position.
 * @param pos the position
 * @return an integer in [0, 63]; the index of the bit corresponding
 * to the given position
 */
int8_t teamnaive_getIndexByPos(pos_t pos) {
    return teamnaive_getIndex(pos.y, pos.x);
}

/**
 * Computes a bitmask for the move between start and end, inclusive.
 * Assumes they are on the same horizontal, vertical or diagonal axis.
 * @param start the start position of the range
 * @param end the end position of the range
 * @return a mask with the bits in the described range asserted
 */
uint64_t teamnaive_getMoveMask(pos_t start, pos_t end) {
    // TODO: I think this code works but worth more testing
    // Smaller index first for ease of use
    if (start.y > end.y || (start.y == end.y && start.x > end.x)) {
        pos_t tmp = start;
        start = end, end = tmp;
    }
    
    // Convert positions to bit indices
    int8_t u = teamnaive_getIndexByPos(start);
    int8_t v = teamnaive_getIndexByPos(end);
    
    // Horizontal range
    if (start.y == end.y) {
        // All the bits we need to flip are contiguous
        return teamnaive_rangeMask(u, v);
    }
    
    // Vertical range
    if (start.x == end.x) {
        // Mask with all bits in the target column
        uint64_t mask = 0x0101010101010101ull << start.x;
        return mask & teamnaive_rangeMask(u, v); // restrict the flipped range
    }
    
    // Diagonal range
    
    // Main diagonal template masks
    const uint64_t main = 0x8040201008040201ull; // southeast
    const uint64_t anti = 0x0102040810204080ull; // northeast
    
    // Aligned with the main diagonal
    if (start.x < end.x) {
        // Shift the main diagonal
        signed dif = (signed) start.y - start.x;
        uint64_t mask = (dif >= 0) ? (main >> dif) : (main << -dif);
        
        // Restrict the range
        return mask & teamnaive_rangeMask(u, v);
    }
    
    // Aligned with the anti-diagonal
    // Shift the template mask
    signed dif = (signed) start.x + start.y - 7;
    uint64_t mask = (dif >= 0) ? (anti << dif) : (anti >> -dif);
    
    // Restrict the range
    return mask & teamnaive_rangeMask(u, v);
}


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
board_t teamnaive_executeMove(board_t state, pos_t pos, int color) {
    // TODO: test this lol
    // If the cell is nonempty, we can't place here
    if (teamnaive_hasPiece(state, pos)) return state;
    
    // Directions to check for flip ranges over
    const pos_t dirs[] = {
            {-1, -1},
            {-1, 0},
            {-1, 1},
            {0,  -1},
            {0,  1},
            {1,  -1},
            {1,  0},
            {1,  1}
    };
    
    // Try all directions
    for (int i = 0; i < 8; i++) {
        // Flip the range in this direction, if it's valid
        pos_t dir = dirs[i];
        teamnaive_executeMovePartial(&state, pos, color, dir.y, dir.x);
    }
    
    // If we didn't make any moves, the state is unmodified
    return state;
}

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
void teamnaive_executeMovePartial(board_t *state, pos_t start, int color, int8_t dy, int8_t dx) {
//    printf("\ndy = %d, dx = %d, col = %d\n", dy, dx, col);
    
    // Loop over the run we're looking at
    pos_t end = teamnaive_makePos(start.y + dy, start.x + dx);
    for (; teamnaive_inBounds(end); end.y += dy, end.x += dx) {
        int piece = teamnaive_getPiece(*state, end);
//        printf("(y, x) = (%d, %d) -> %d\n", end.y, end.x, piece);
        if (piece == !color) continue; // if the opponent has a piece here, keep going
        
        // If the cell is empty and |run| > 1, flip this range
        if (piece == color && (end.y != start.y + dy || end.x != start.x + dx)) {
//            printf("Setting run (%d, %d) -> (%d, %d)\n", start.y, start.x, end.y, end.x);
            teamnaive_setPieces(state, start, end, color);
        }
        
        // Either we performed a move or the range is invalid
        return;
    }
}

/**
 * Flips the pieces between the provided start and end positions, inclusive.
 * Assumes all pieces in the range are present.
 * @param state the board state to update
 * @param start the start position of the run to flip
 * @param end the end position of the run to flip
 */
void teamnaive_flipPieces(board_t *state, pos_t start, pos_t end) {
    // Compute a mask for the bits between start and end
    uint64_t mask = teamnaive_getMoveMask(start, end);
    state->color ^= mask; // flip the bits in range
}

/**
 * Sets the pieces between the provided start and end positions, inclusive,
 * to the desired color. Also places pieces if any cells in range are empty.
 * @param state the board state to update
 * @param start the start position of the run to set
 * @param end the end position of the run to set
 * @param color the color to set the pieces to
 */
void teamnaive_setPieces(board_t *state, pos_t start, pos_t end, int color) {
    // Compute a mask for the bits between start and end
    uint64_t mask = teamnaive_getMoveMask(start, end);
    
    // Place pieces where they're missing
    state->on |= mask;
    
    // Set the piece colors
    if (color) state->color |= mask;
    else state->color &= ~mask;
}

/**
 * Create pair of position and score to return from recursive solve function.
  * @param pos the position to return
  * @param score the score to return
  */
solvePair_t teamnaive_makeSolvePair(pos_t pos, int score) {
    solvePair_t res;
    res.pos = pos, res.score = score;
    return res;
}


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
int8_t teamnaive_getIndex(int8_t y, int8_t x) { return y * 8 + x; }

/**
 * Checks whether the given bit is on.
 * @param mask the bitmask to check
 * @param ind the index to check in the integer
 * @return 1 if the bit at the given index is on; 0 otherwise.
 */
int teamnaive_getBit(uint64_t mask, int8_t ind) {
    uint64_t check = ((uint64_t) 1) << ind;
    return (mask & check) != 0;
}

/**
 * Sets the bit at the given index to the given value.
 * @param mask the mask to operate on
 * @param ind the index of the bit to modify
 * @param value 0 to set the bit off; any other value to set it on
 */
void teamnaive_setBit(uint64_t *mask, int8_t ind, int value) {
    uint64_t set = ((uint64_t) 1) << ind;
    
    if (value) *mask |= set;
    else *mask &= ~set;
}

/**
 * Creates a mask with a range of bits between `start` and `end` asserted.
 * @param start the start (from bit 0->) of the range to assert
 * @param offset the (inclusive) end of the range to assert
 * @return the described mask
 */
uint64_t teamnaive_rangeMask(int8_t start, int8_t end) {
    uint64_t mask = 1ull << (end - start);
    mask <<= 1, mask--;
    return mask << start;
}

/**
 * Counts the number of set bits in the given integer.
 * @param num the integer
 * @return the number of bits that are on in `num`
 */
int teamnaive_popcount(uint64_t num) {
#ifdef __has_builtin
#if __has_builtin(__builtin_popcountll)
#define popcount(x) __builtin_popcountll(x)
#endif // has_builtin
#endif // ifdef
#ifdef popcount
    // If GCC's __builtin_popcount is available, we use that as
    // it's often a single machine-level instruction
    return popcount(num);
#else
    // Otherwise we do it manually
    // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
    int8_t res = 0;
    for (; num; res++) num &= num - 1;
    return res;
#endif
}
