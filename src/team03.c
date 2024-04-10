/*
 * COP3502H Final Project
 * Team 03
 * Erick + Benjamin
 */


/*
 **********************
 * Configuration      *
 **********************
 */

#define TEAM03_DEBUG 0
// Toggle pretty-printing search status

// Define escape code macros for debug printing
#if TEAM03_DEBUG
#   define ANSI_RED "\x1b[31m"
#   define ANSI_GREEN "\x1b[32m"
#   define ANSI_CYAN "\x1b[36m"
#   define ANSI_RESET "\x1b[0m"
#   define VT100_CLEAR_LINE "\033[A\33[2K"
#endif

// Check if GCC optimizations are available
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7) || defined(__clang__))
#   define GCC_OPTIM_AVAILABLE
#endif
// If available, enable the O2 optimization level
#ifdef GCC_OPTIM_AVAILABLE
#   pragma GCC push_options
#   pragma GCC optimize("O2")
#endif // GCC_OPTIM_AVAILABLE


/*
 **********************
 * Includes & globals *
 **********************
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>
#include "team03.h"

const int team03_timePadding = 10; // padding (ms) for search timer
const int team03_maxLayers = 24; // max depth of iterative search
long long team03_maxTime = 5000; // max time (ms) per move; overwritten later
struct timeval team03_startTime; // start time of the current move


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
position *team03Move(const enum piece board[][SIZE], enum piece mine, int secondsleft) {
    // Translate stuff
    board_t state = team03_loadBoard(board);
    int color = (mine == WHITE);
    
    // Perform the move with our own state format
    pos_t pos = team03_getMove(state, color, secondsleft);
    
    // Dynamically allocate the return position
    position *res = malloc(sizeof(position));
    res->x = pos.y, res->y = pos.x;
    
    // Return the position (duh)
    return res;
}

/**
 * Performs a move for our team.
 *
 * @param board the current board state
 * @param color the color we're playing (0/1 for black/white)
 * @param time the time (in seconds) our team has left on the game timer
 *
 * @return the position we placed a piece at
 */
pos_t team03_getMove(board_t state, int color, int time) {
    // Start the move clock and allocate time
    gettimeofday(&team03_startTime, 0);
    team03_maxTime = team03_allocateTime(state, color, time);
    
    // Search for a move
    pos_t res = team03_iterate(state, color);

#if TEAM03_DEBUG
    // Print how much time we took to pick a move, if debug is on
    long long took = team03_timeSinceMs(team03_startTime);
    printf("team03_iterate took " ANSI_RED "%lli ms\n\n" ANSI_RESET, took);
#endif
    
    // Return the move we chose
    return res;
}


/*
 **********************
 * Heuristics         *
 **********************
 */

/**
 * Allocates time for this turn, returning a bound for the max
 * amount of time to spend on the current move (in ms).
 *
 * @param state the current board state
 * @param color our piece color
 * @param timeLeft how many seconds we have left
 *
 * @return the max time for this turn, in ms
 */
long long team03_allocateTime(board_t state, int color, int timeLeft) {
    // Count pieces
    int numPieces = team03_count(state, color) + team03_count(state, !color);
    
    // Calculate number of ms until the next full second
    long long timeToNextSecond = 1000 - (team03_startTime.tv_usec / 1000);
    
    // First four moves are free (unless we're forced up against the
    // end of a second; in which case we take 20ms and count as 1
    if (numPieces <= 11) {
        long long res = timeToNextSecond - team03_timePadding;
        return (res < 20) ? 20 : res;
    }
    
    // If we would get under 5.5s, take ~6.5 and count as 6
    long long res = 5000 + timeToNextSecond;
    if (res < 5500) res += 1000;
    
    // Return our computed time allocation
    return res - team03_timePadding;
}

/**
 * Computes a relative score for how many of the pieces on th
 * board we currently own. Not weighted very highly.
 *
 * @param state the board state
 * @param color the color to count pieces for
 *
 * @return a relative parity score
 */
float team03_computeParity(board_t state, int color) {
    // Score: (# of our pieces / # of total pieces)
    return team03_count(state, color)
           / (float) team03_popcount(state.on);
}

/**
 * Computes the given color's mobility for the current board state,
 * counting the number of valid moves that color can take.
 *
 * @param state the current board state
 * @param color the color to consider moves for
 *
 * @return the number of valid moves that the color can take
 */
int team03_computeMobility(board_t state, int color) {
    // Count for number of valid moves
    int res = 0;
    
    // Loop over all cell positions
    pos_t pos = team03_makePos(0, 0);
    for (; pos.y < 8; pos.y++) {
        for (pos.x = 0; pos.x < 8; pos.x++) {
            // Update our valid move count if we found one
            res += team03_isValidMove(state, pos, color);
        }
    }
    return res;
}

/**
 * Statically evaluate the current board position for a given color.
 * Only accounts for the current level, disregarding future moves.
 *
 * @param state the current board state
 * @param color the color to evaluate for
 *
 * @return a relative score for the current board state
 */
int team03_evaluateStatic(board_t state, int color) {
    // Calculate an overall mobility score
    int score = team03_computeMobility(state, color)
                - team03_computeMobility(state, !color);
    
    // Add parity score
    int parityScore = team03_computeParity(state, color) * 2;
    score += parityScore;
    
    // Weight the corners
    const pos_t corners[4] = {
            {0, 0},
            {0, 7},
            {7, 7},
            {7, 0}
    };
    
    // Corner weight starts at 4; goes up by 4 every 8 turns
    int cornerWeight = 4 + (team03_getTurnNum(state, color) / 8) * 4;
    for (int i = 0; i < 4; i++) {
        if (team03_getPiece(state, corners[i]) == color) score += cornerWeight;
    }
    /*
    const pos_t adjCorners[12] = {{0, 1}, {1, 1}, {1, 0}, {0, 6}, {1, 6}, {1, 7}, {7, 6}, {6, 6}, {6, 7}, {7, 1}, {6, 1}, {6, 0}};
    for (int i = 0; i < 12; i++) {
        pos_t corn;
        if (adjCorners[i].x <= 1) corn.x = 0;
        else corn.x = 7;
        if (adjCorners[i].y <= 1) corn.y = 0;
        else corn.y = 7;
        if (team03_getPiece(state, adjCorners[i]) == color &&
            team03_getPiece(state, corn) != color) score -= 3;
    }
    */
    
    return score;
}


/*
 **********************
 * The actual bot lol *
 **********************
 */

/**
 * Iteratively deepens the search tree, evaluating at higher and
 * higher depths until we've used as much time as we want.
 *
 * @param state the current board state
 * @param color our color
 *
 * @return the best move we find before we use the allotted time
 */
pos_t team03_iterate(board_t state, int color) {
    // Get our valid moves from this position, in decreasing order of static score
    solvePair_t moveList[64];
    int num = team03_getMoves(state, color, moveList, 1);

#if TEAM03_DEBUG
    // Print our status if debugging is on
    printf("Our turn! " ANSI_CYAN "We are %s (%c)\n"
           ANSI_RESET "Found " ANSI_CYAN "%d"
           ANSI_RESET " moves, allocated "
           ANSI_RED "%lli ms" ANSI_RESET "\n\n",
           
           color ? "white" : "black",
           color ? 'O' : 'X', num, team03_maxTime);
#endif
    
    // If we don't have any moves, we shouldn't have gotten a move at all
    assert(num != 0 && "Our turn but no moves available!");
    
    // Track our overall best move & position to return
    pos_t bestPos = moveList[0].pos;
    pos_t retPos = bestPos;

#if TEAM03_DEBUG
#   define DEPTH_STATUS_MAX 14 // # of layers to display in depth status
    // Debug search status
    printf("Search depth:\n\n\n\n");
#endif
    
    // Iteratively deepen the search
    for (int layers = 1; layers <= team03_maxLayers; layers++) {
#if TEAM03_DEBUG
        // Print search depth indicator
        for (int i = 0; i < 2; i++) printf(VT100_CLEAR_LINE " ");
        for (int i = 1; i <= DEPTH_STATUS_MAX; i++)
            if (i == layers) {
                if (i <= 9) printf(ANSI_CYAN "[%-1d] " ANSI_RESET, i);
                else printf(ANSI_CYAN "[%-2d]" ANSI_RESET, i);
            } else printf(" %-3d", i);
        if (layers > DEPTH_STATUS_MAX) printf(ANSI_CYAN "[%-3d]\n  " ANSI_RESET, layers);
        else printf(" ...\n  ");
        for (int i = 1; i < ((layers > DEPTH_STATUS_MAX) ? DEPTH_STATUS_MAX + 1 : layers); i++) printf("    ");
        printf(ANSI_CYAN " ▲\n" ANSI_RESET);
#endif
        
        // Track our best move and alpha/beta for this depth
        int best = -1e9, alpha = -1e9, beta = 1e9;
        bestPos = team03_makePos(-1, -1);
        
        // Iterate through valid moves for this position
        for (int i = 0; i < num; i++) {
            // DLS on the current move
            solvePair_t pair = moveList[i];
            solvePair_t pair2 = team03_solveBoard(
                    team03_executeMove(state, pair.pos, color),
                    color ^ 1, layers - 1, -beta, -alpha);
            
            // If we ran out of time, return our current best position
            if (pair2.pos.x == -2) {
#if TEAM03_DEBUG
                // Print how much time we've taken
                long long taken = team03_timeSinceMs(team03_startTime);
                printf("\rTimeout at depth " ANSI_CYAN "%d" ANSI_RESET
                       " after " ANSI_RED "%lli ms\n" ANSI_RESET,
                       layers, taken);
#endif
                return retPos;
            }
            
            // Update the move's score with the opponent's best move
            int score = 0 - pair2.score;
            moveList[i].score = score;
            
            // Update our current best move
            if (score > best) {
                best = score;
                bestPos = pair.pos;
            }
            
            // Pruning or something
            if (score > alpha) alpha = score;
            if (alpha >= beta) {
                best = alpha;
                break;
            }
        }
        
        // Sort our move list on the updated scores
        team03_sort(moveList, 0, num - 1);
        
        // Update return pos to the best move from this depth
        retPos = bestPos;
    }
    
    // Return the best move we found
    return retPos;
}

/**
 * Finds the best move by searching up to the given depth. If we
 * use all of the allotted time (> team03_maxTime ms), returns a
 * pair with position (-2, -2).
 *
 * @param state the current board state
 * @param color the current color being considered
 * @param layer the number of layers to search through
 * @param alpha the alpha
 * @param beta the beta
 *
 * @return the best move
 */
solvePair_t team03_solveBoard(board_t state, int color, int layer, int alpha, int beta) {
    // Check for a timeout
    long long taken = team03_timeSinceMs(team03_startTime);
    if (taken >= team03_maxTime) {
        solvePair_t pair = team03_makeSolvePair(team03_makePos(-2, -2), 0);
        return pair;
    }
    
    // If we're at a leaf, return our best move at this level
    if (layer == 0) {
        int score = team03_evaluateStatic(state, color);
        pos_t pos = team03_makePos(-1, -1);
        return team03_makeSolvePair(pos, score);
    }
    
    // Find valid moves (without static eval)
    solvePair_t pairs[64];
    int num = team03_getMoves(state, color, pairs, 0);
    
    // Check if there aren't any moves available for the current color
    if (num == 0) {
        // Check if the opponent can move
        num = team03_getMoves(state, !color, pairs, 0);
        if (num) {
            // If so, do the opponent move
            solvePair_t ret = team03_solveBoard(state, color ^ 1, layer - 1, -beta, -alpha);
            ret.score = 0 - ret.score;
            return ret;
        }
        
        // Otherwise the game is over at this depth; set the score to inf/-inf for win/lose
        int score;
        if (team03_count(state, color) > team03_count(state, color ^ 1)) score = 1e8;
        else if (team03_count(state, color) < team03_count(state, color ^ 1)) score = -1e8;
        else score = 0;
        
        // Return the thing
        pos_t pos = team03_makePos(-1, -1);
        solvePair_t ret = team03_makeSolvePair(pos, score);
        return ret;
    }
    
    // Track our current best move
    int best = -1e9;
    pos_t bestPos = team03_makePos(-1, -1);
    
    // Loop over valid moves for the current color
    for (int i = 0; i < num; i++) {
        // Execute the current move and figure out the opponent's best move
        board_t cur = team03_executeMove(state, pairs[i].pos, color);
        solvePair_t oppSolve = team03_solveBoard(cur, !color, layer - 1, -beta, -alpha);
        
        // If we ran out of time, return the opponent's move
        // TODO: ???
        if (oppSolve.pos.x == -2) return oppSolve;
        
        // Update the best move
        int score = 0 - oppSolve.score;
        if (score > best) {
            best = score;
            bestPos = pairs[i].pos;
        }
        
        // Pruning or something
        if (score > alpha) alpha = score;
        if (alpha >= beta) {
            solvePair_t pair = team03_makeSolvePair(bestPos, alpha);
            return pair;
        }
    }
    
    // Return the best move we found
    solvePair_t pair = team03_makeSolvePair(bestPos, best);
    return pair;
}

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
int team03_getMoves(board_t state, int color, solvePair_t *arr, int evaluate) {
    // # of valid moves we've found
    int num = 0;
    
    // Loop over all cell positions
    pos_t pos = team03_makePos(0, 0);
    for (; pos.y < 8; pos.y++) {
        for (pos.x = 0; pos.x < 8; pos.x++) {
            // Execute this move; skip if its invalid
            board_t cur = team03_executeMove(state, pos, color);
            if (team03_boardEquals(state, cur)) continue;
            
            // Compute the score for this move
            int score = evaluate ? team03_evaluateStatic(state, color) : 0;
            arr[num++] = team03_makeSolvePair(pos, score);
        }
    }
    
    // Sort the output array
    if (num > 0 && evaluate) team03_sort(arr, 0, num - 1);
    return num;
}


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
board_t team03_loadBoard(const enum piece board[][SIZE]) {
    // Initialize an empty board
    board_t res;
    res.on = res.color = 0;
    
    // Iterate over board positions
    for (int8_t i = 0; i < 8; i++) {
        for (int8_t j = 0; j < 8; j++) {
            // Operate on the cell at this index
            int8_t ind = team03_getIndex(i, j);
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
 * Prints the state of the board.
 *
 * @param state the board state
 */
void team03_print(board_t state) {
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
            int8_t ind = team03_getIndex(i, j);
            if (!team03_getBit(state.on, ind)) printf(". "); // empty cell
            else {
                // Non-empty cell
                int color = team03_getBit(state.color, ind);
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
 *
 * @param state the board state to check
 * @param color the piece color to look for
 *
 * @return the described mask
 */
uint64_t team03_getPieces(board_t state, int color) {
    // Compute a mask for the requested color
    uint64_t color_mask = color ? state.color : ~state.color;
    return state.on & color_mask; // filter placed pieces by the color
}

/**
 * Checks if there is a piece at the position and returns its color
 * if there is.
 *
 * @param state the board state
 * @param pos the position to check
 *
 * @return the color of the piece (1 or 0) if there is one present; otherwise, -1.
 */
int team03_getPiece(board_t state, pos_t pos) {
    if (!team03_hasPiece(state, pos)) return -1;
    return team03_getColor(state, pos);
}

/**
 * Checks if there is a piece of any color at the given position.
 *
 * @param state the board state
 * @param pos the position to check
 *
 * @return 1 if there is a piece at the given position; otherwise 0
 */
int team03_hasPiece(board_t state, pos_t pos) {
    int8_t ind = team03_getIndexByPos(pos);
    return team03_getBit(state.on, ind);
}

/**
 * Gets the color of a piece at the given position. If there is no
 * piece at that position, the returned value is indeterminate.
 *
 * @param state the current board state
 * @param pos the position to look at
 *
 * @return the color of the piece at the given position (0/1 for black/white)
 */
int team03_getColor(board_t state, pos_t pos) {
    int8_t ind = team03_getIndexByPos(pos);
    return team03_getBit(state.color, ind);
}

/**
 * Sets the piece at the given position to the given color.
 * If there was no piece, places one first.
 *
 * @param state the board state to update
 * @param pos the position to set the piece at
 * @param color the color to set the piece to
 */
void team03_setPiece(board_t *state, pos_t pos, int color) {
    int8_t ind = team03_getIndexByPos(pos);
    team03_setBit(&state->on, ind, 1);
    team03_setBit(&state->color, ind, color);
}

/**
 * Counts all pieces on the board with the given color.
 *
 * @param state the current board state
 * @param color the color to count (0/1 for black/white)
 *
 * @return the number of pieces on the board with the given color
 */
int team03_count(board_t state, int color) {
    // Count the number of set bits in the mask for this piece color
    return team03_popcount(team03_getPieces(state, color));
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
 *
 * @param state1 the first board position
 * @param state2 the second board position
 *
 * @return 1 if the board states are the same; 0 otherwise.
 */
int team03_boardEquals(board_t state1, board_t state2) {
    return state1.on == state2.on && state1.color == state2.color;
}

/**
 * Checks if the states of the given positions are equal.
 *
 * @param pos1 the first position to check
 * @param pos2 the second position to check
 *
 * @return 1 if the cells have the same state (on-ness and color);
 * 0 otherwise.
 */
int team03_pieceEquals(board_t state, pos_t pos1, pos_t pos2) {
    int a = team03_hasPiece(state, pos1);
    int b = team03_hasPiece(state, pos2);
    
    if (a ^ b) return 0; // if the on-ness differs, then not equal
    if (!a) return 1; // if both are off, we don't care about the color
    
    // Otherwise true if the colors are equal
    return team03_getColor(state, pos1)
           == team03_getColor(state, pos2);
}


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
pos_t team03_makePos(int8_t row, int8_t col) {
    pos_t res;
    res.y = row, res.x = col;
    return res;
}

/**
 * Checks whether the position is within the bounds of the board.
 *
 * @param pos the position to check
 *
 * @return 1 if the position is in bounds; 0 otherwise
 */
int team03_inBounds(pos_t pos) {
    return pos.y >= 0 && pos.y < 8 && pos.x >= 0 && pos.x < 8;
}

/**
 * Gets the flat bit index for the given position.
 *
 * @param pos the position
 *
 * @return an integer in [0, 63]: the index of the bit corresponding to the given position
 */
int8_t team03_getIndexByPos(pos_t pos) {
    return team03_getIndex(pos.y, pos.x);
}

/**
 * Computes a bitmask for the move between start and end, inclusive.
 * Assumes they are on the same horizontal, vertical or diagonal axis.
 *
 * @param start the start position of the range
 * @param end the end position of the range
 *
 * @return a mask with the bits in the described range asserted
 */
uint64_t team03_getMoveMask(pos_t start, pos_t end) {
    // TODO: I think this code works but worth more testing
    // Smaller index first for ease of use
    if (start.y > end.y || (start.y == end.y && start.x > end.x)) {
        pos_t tmp = start;
        start = end, end = tmp;
    }
    
    // Convert positions to bit indices
    int8_t u = team03_getIndexByPos(start);
    int8_t v = team03_getIndexByPos(end);
    
    // Horizontal range
    if (start.y == end.y) {
        // All the bits we need to flip are contiguous
        return team03_rangeMask(u, v);
    }
    
    // Vertical range
    if (start.x == end.x) {
        // Mask with all bits in the target column
        uint64_t mask = 0x0101010101010101ull << start.x;
        return mask & team03_rangeMask(u, v); // restrict the flipped range
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
        return mask & team03_rangeMask(u, v);
    }
    
    // Aligned with the anti-diagonal
    // Shift the template mask
    signed dif = (signed) start.x + start.y - 7;
    uint64_t mask = (dif >= 0) ? (anti << dif) : (anti >> -dif);
    
    // Restrict the range
    return mask & team03_rangeMask(u, v);
}


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
int team03_isValidMove(board_t state, pos_t pos, int color) {
    // TODO: test this more lol
    // If the cell is nonempty, we can't place here
    if (team03_hasPiece(state, pos)) return 0;
    
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
        board_t cur = state;
        team03_executeMovePartial(&cur, pos, color, dir.y, dir.x);
        
        // If the state was modified, we did a flip => this move is valid
        if (!team03_boardEquals(state, cur)) return 1;
    }
    
    // If we didn't make any flips, the move is invalid
    return 0;
}

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
board_t team03_executeMove(board_t state, pos_t pos, int color) {
    // TODO: test this more lol
    // If the cell is nonempty, we can't place here
    if (team03_hasPiece(state, pos)) return state;
    
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
        team03_executeMovePartial(&state, pos, color, dir.y, dir.x);
    }
    
    // If we didn't make any moves, the state is unmodified
    return state;
}

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
void team03_executeMovePartial(board_t *state, pos_t start, int color, int8_t dy, int8_t dx) {
    // Loop over the run we're looking at
    pos_t end = team03_makePos(start.y + dy, start.x + dx);
    for (; team03_inBounds(end); end.y += dy, end.x += dx) {
        int piece = team03_getPiece(*state, end);
        if (piece == !color) continue; // if the opponent has a piece here, keep going
        
        // If the cell is empty and |run| > 1, flip this range
        if (piece == color && (end.y != start.y + dy || end.x != start.x + dx)) {
            team03_setPieces(state, start, end, color);
        }
        
        // Either we performed a move or the range is invalid
        return;
    }
}

/**
 * Flips the pieces between the provided start and end positions, inclusive.
 * Assumes all pieces in the range are present.
 *
 * @param state the board state to update
 * @param start the start position of the run to flip
 * @param end the end position of the run to flip
 */
void team03_flipPieces(board_t *state, pos_t start, pos_t end) {
    // Compute a mask for the bits between start and end
    uint64_t mask = team03_getMoveMask(start, end);
    state->color ^= mask; // flip the bits in range
}

/**
 * Sets the pieces between the provided start and end positions, inclusive,
 * to the desired color. Also places pieces if any cells in range are empty.
 *
 * @param state the board state to update
 * @param start the start position of the run to set
 * @param end the end position of the run to set
 * @param color the color to set the pieces to
 */
void team03_setPieces(board_t *state, pos_t start, pos_t end, int color) {
    // Compute a mask for the bits between start and end
    uint64_t mask = team03_getMoveMask(start, end);
    
    // Place pieces where they're missing
    state->on |= mask;
    
    // Set the piece colors
    if (color) state->color |= mask;
    else state->color &= ~mask;
}

/**
 * Create pair of position and score to return from recursive solve function.
 *
  * @param pos the position to return
  * @param score the score to return
  *
  * @return the pair, duh
  */
solvePair_t team03_makeSolvePair(pos_t pos, int score) {
    solvePair_t res;
    res.pos = pos, res.score = score;
    return res;
}


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
void team03_sort(solvePair_t *pairs, int lo, int hi) {
    if (lo >= hi) return;
    int md = (lo + hi) / 2;
    
    // Sort the subarrays
    team03_sort(pairs, lo, md);
    team03_sort(pairs, md + 1, hi);
    
    // Merge the subarrays in sorted order
    team03_merge(pairs, lo, md + 1, hi);
}

/**
 * Merges the subarrays [lo, md) and [md, hi].
 *
 * @param arr the array to operate on
 * @param lo the leftmost index of the range to merge
 * @param md the middle index of the range to merge
 * @param hi the rightmost index of the range to merge
 */
void team03_merge(solvePair_t *arr, int lo, int md, int hi) {
    // Setup
    int n = hi - lo + 1;
    solvePair_t temp[64];
    
    // Left/right subarray indices; temp array index
    int i = lo, j = md, k = 0;
    while (i < md || j <= hi) {
        // Take a value from the left subarray
        if (j > hi || (i < md && (arr[i].score > arr[j].score))) {
            temp[k++] = arr[i++];
            continue;
        }
        
        // Otherwise take from the right
        temp[k++] = arr[j++];
    }
    
    // Copy back to original array; free up temp
    for (i = lo; i <= hi; i++) arr[i] = temp[i - lo];
}

/**
 * Computes the amount of time that has passed since the
 * start timestamp, in milliseconds.
 *
 * @param start the start timestamp
 *
 * @return the elapsed time in ms
 */
long long team03_timeSinceMs(struct timeval start) {
    // Get the current time
    struct timeval now;
    gettimeofday(&now, 0);
    
    // Compute the elapsed time in microseconds
    long long diff_usec =
            (now.tv_usec + 1000000ll * now.tv_sec)
            - (start.tv_usec + 1000000ll * start.tv_sec);
    
    // Return the elapsed time in ms
    return diff_usec / 1000;
}

/**
 * Gets the approximate turn number from a piece count
 *
 * @param state the current board state
 * @param color the color to count moves for
 *
 * @return the approximate turn number (starts at 1)
 */
int team03_getTurnNum(board_t state, int color) {
    int num = team03_count(state, color)
              + team03_count(state, !color);
    return (num - 4) / 2 + 1;
}


/*
 **********************
 * Bit manipulation   *
 **********************
 */

/**
 * Convenience function for checking whether a bit is on by
 * its position.
 *
 * @param mask the mask to check
 * @param pos the position of the bit to check
 *
 * @return 1 if the bit at pos is on, 0 otherwise
 */
int team03_getBitAt(uint64_t mask, pos_t pos) {
    return team03_getBit(mask, team03_getIndexByPos(pos));
}

/**
 * Convenience function for setting the bit by its position.
 *
 * @param mask a pointer to the mask to modify
 * @param pos the position of the bit to set
 * @param value 0 to turn the bit off; any other value to turn it on
 */
void team03_setBitAt(uint64_t *mask, pos_t pos, int value) {
    team03_setBit(mask, team03_getIndexByPos(pos), value);
}

/**
 * Gets the flat bit index for the given y/x indices.
 *
 * @return an integer in [0, 63]: the index of the bit for the given position
 */
int8_t team03_getIndex(int8_t y, int8_t x) {
    return y * 8 + x;
}

/**
 * Checks whether the given bit is on.
 *
 * @param mask the bitmask to check
 * @param ind the index to check in the integer
 *
 * @return 1 if the bit at the given index is on; 0 otherwise.
 */
int team03_getBit(uint64_t mask, int8_t ind) {
    uint64_t check = ((uint64_t) 1) << ind;
    return (mask & check) != 0;
}

/**
 * Sets the bit at the given index to the given value.
 *
 * @param mask the mask to operate on
 * @param ind the index of the bit to modify
 * @param value 0 to set the bit off; any other value to set it on
 */
void team03_setBit(uint64_t *mask, int8_t ind, int value) {
    uint64_t set = ((uint64_t) 1) << ind;
    
    if (value) *mask |= set;
    else *mask &= ~set;
}

/**
 * Creates a mask with a range of bits between `start` and `end` asserted.
 *
 * @param start the start (from bit 0->) of the range to assert
 * @param offset the (inclusive) end of the range to assert
 *
 * @return the described mask
 */
uint64_t
team03_rangeMask(int8_t start, int8_t end) {
    uint64_t mask = 1ull << (end - start);
    mask <<= 1, mask--;
    return mask << start;
}

/**
 * Counts the number of set bits in the given integer.
 *
 * @param num the integer
 *
 * @return the number of bits that are on in `num`
 */
int team03_popcount(uint64_t num) {
#ifdef __has_builtin
#   if __has_builtin(__builtin_popcountll)
#       define popcount(x) __builtin_popcountll(x)
#   endif // has_builtin
#endif // ifdef
#ifdef popcount
    // If GCC's __builtin_popcount is available, we use that as
    // it's often a single machine-level instruction
    return popcount(num);
#else
    // Otherwise we do it manually
    // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
    int res = 0;
    for (; num; res++) num &= num - 1;
    return res;
#endif
}

// Pop our optimization settings
#ifdef GCC_OPTIM_AVAILABLE
#   pragma GCC pop_options
#endif // GCC_OPTIM_AVAILABLE
