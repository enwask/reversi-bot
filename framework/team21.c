// Arup Guha
// 2/18/2024
// A second testing team for the COP 3502 final project.
// This one's slightly smarter; it chooses a greedy first move.

#include <stdlib.h>
#include <time.h>

#include "team21.h"
#include "reversi_functions.h"

position* team21Move(const enum piece board[][SIZE], enum piece mine, int secondsleft) {

    // For randomness!
    srand(time(0));

    // Get the move list.
    int numMoves;
    position* choices = getPossibleMoves(board, mine, &numMoves);

    // Will store best move and best index here.
    int i, maxI = -1, max = -1;

    // Try each move.
    for (i=0; i<numMoves; i++) {

        // Make a copy of the original board.
        enum piece myBoard[SIZE][SIZE];
        copy(myBoard, board);

        // Make the move on this board.
        executeMove(myBoard, &choices[i], mine);

        // Calculate how many pieces I now have.
        int tmpPieces = count(myBoard, mine);

        // Update if this move is better.
        if (i == 0 || tmpPieces > max) {
            maxI = i;
            max = tmpPieces;
        }
    }

    // Copy the move into our newly created struct.
    position* res = malloc(sizeof(position));
    res->x = choices[maxI].x;
    res->y = choices[maxI].y;

    // Free the array.
    free(choices);

    // Return our randomly selected move.
    return res;
}
