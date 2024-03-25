// Arup Guha
// 2/18/2024
// A "dummy" testing team for the COP 3502 final project.

#include <stdlib.h>
#include <time.h>

#include "team20.h"
#include "reversi_functions.h"

position* team20Move(const enum piece board[][SIZE], enum piece mine, int secondsleft) {

    // For randomness!
    srand(time(0));

    // Get the move list, just choose random one.
    int numMoves;
    position* choices = getPossibleMoves(board, mine, &numMoves);

    // Copy the move into our newly created struct.
    position* res = malloc(sizeof(position));

    // Silly move, chooses the one with the minimal x (row) value.
    int idx = 0;
    for (int i=0; i<numMoves; i++) {
        if (choices[i].x < choices[idx].x)
            idx = i;
    }

    // Assign move.
    res->x = choices[idx].x;
    res->y = choices[idx].y;

    // Free the array.
    free(choices);

    // Return our randomly selected move.
    return res;
}
