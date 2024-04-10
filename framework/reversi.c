// Arup Guha
// 2/10/2019
// Framework for COP 3502 Final Project: Reversi

/*** Note: To compile do:

gcc -o reversi.exe reversi.c reversi_functions.c team03.c team21.c

Of course, replace 20 and 21 with whichever teams you want to play
and modify the code that calls those functions accordingly.

***/

/*** Also, there is a facility to play human v human or human v computer.
     Just edit the call from main to play the version you want to.
     Also, the computer v computer version currently waits one second
     between moves (doesn't count against a player) so viewers can sort of
     see the moves. Feel free to comment this out during testing. It's
     currently on lines 148 and 149.
***/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "reversi_functions.h"
#include "team21.h"
#include "../src/team03.h"
#include "../src/teamrand.h"
#include "../src/teamnaive.h"
#include "reversi.h"

int main(void) {
    test();
    return 0;
}

void humanVHuman() {
    
    // Initialize and print the board. Black goes first.
    enum piece board[SIZE][SIZE];
    initBoard(board);
    printBoard(board);
    enum piece player = BLACK;
    
    // Go till the game is done.
    while (!gameOver(board)) {
        
        // We can flip the team if the current team can't move.
        if (!canMove(board, player))
            player = opposite(player);
        
        // Get this player's move and execute it.
        position *mymove = getUserMove(board, player);
        executeMove(board, mymove, player);
        free(mymove);
        
        // Print the result and go to the other player.
        printBoard(board);
        player = opposite(player);
    }
    
    // Get scores.
    int blackScore = score(board, BLACK);
    int whiteScore = score(board, WHITE);
    
    // Print scores.
    printf("Black's score is %d\n", blackScore);
    printf("White's score is %d\n", whiteScore);
    
    // Output the result.
    if (blackScore > whiteScore)
        printf("Black wins by %d points.\n", blackScore - whiteScore);
    else if (whiteScore > blackScore)
        printf("White wins by %d points.\n", whiteScore - blackScore);
    else
        printf("It's a tie!\n");
}

void computerVComputer() {
    
    // Initialize and print the board. Black goes first.
    enum piece board[SIZE][SIZE];
    initBoard(board);
    printBoard(board);
    enum piece player = BLACK;
    
    int blackTime = MAXTIME;
    int whiteTime = MAXTIME;
    enum boolean blackOutOfTime = FALSE;
    enum boolean whiteOutOfTime = FALSE;
    
    // Go till the game is done.
    while (!gameOver(board)) {
        
        // We can flip the team if the current team can't move.
        if (!canMove(board, player))
            player = opposite(player);
        
        position *mymove = NULL;
        
        // Black Computer Player
        if (player == BLACK) {
            
            // Do the move and time it.
            int startT = time(0);
            mymove = team03Move(board, player, blackTime);
            int endT = time(0);
            
            // Update time.
            blackTime = blackTime - (endT - startT);
            printf("The black team selected row %d, column %d\n", mymove->x, mymove->y);
        }
            
            // White Computer Player
        else {
            
            // Do the move and time it.
            int startT = time(0);
            mymove = team21Move(board, player, whiteTime);
            int endT = time(0);
            
            // Update time.
            whiteTime = whiteTime - (endT - startT);
            printf("The white team selected row %d, column %d\n", mymove->x, mymove->y);
        }
        
        // Execute then free move.
        executeMove(board, mymove, player);
        free(mymove);
        
        // Black ran out of time.
        if (blackTime < 0) {
            printf("Sorry, the Black Player ran out of time and loses automatically.\n");
            blackOutOfTime = TRUE;
            break;
        }
        
        // White ran out of time.
        if (whiteTime < 0) {
            printf("Sorry, the White Player ran out of time and loses automatically.\n");
            whiteOutOfTime = TRUE;
            break;
        }
        
        // Print the result and go to the other player.
        printBoard(board);
        player = opposite(player);
        
        // This waits about one second, so we can see the moves scrolling.
        //int curT = time(0);
        //while (time(0) == curT);
    }
    
    if (!blackOutOfTime && !whiteOutOfTime) {
        
        // Get scores.
        int blackScore = score(board, BLACK);
        int whiteScore = score(board, WHITE);
        
        // Print scores.
        printf("Black's score is %d\n", blackScore);
        printf("White's score is %d\n", whiteScore);
        
        // Output the result.
        if (blackScore > whiteScore)
            printf("Black wins by %d points.\n", blackScore - whiteScore);
        else if (whiteScore > blackScore)
            printf("White wins by %d points.\n", whiteScore - blackScore);
        else
            printf("It's a tie!\n");
    }
        
        // Black player ran out of time.
    else if (blackOutOfTime) {
        printf("White wins because the Black Player ran out of time.\n");
    }
        // White player ran out of time.
    else {
        printf("Black wins because the White Player ran out of time.\n");
    }
}

void humanVComputer() {
    
    // Initialize and print the board. Black goes first.
    enum piece board[SIZE][SIZE];
    initBoard(board);
    printBoard(board);
    enum piece player = BLACK;
    
    int compTime = MAXTIME;
    enum boolean outOfTime = FALSE;
    
    // Go till the game is done.
    while (!gameOver(board)) {
        
        // We can flip the team if the current team can't move.
        if (!canMove(board, player))
            player = opposite(player);
        
        position *mymove = NULL;
        
        // Hard-coded human first.
        if (player == BLACK)
            mymove = getUserMove(board, player);
            
            // Computer Second.
        else {
            
            // Do the move and time it.
            int startT = time(0);
            mymove = team03Move(board, player, compTime);
            int endT = time(0);
            
            // Update time.
            compTime = compTime - (endT - startT);
            printf("The computer selected row %d, column %d\n", mymove->x, mymove->y);
        }
        
        // Execute then free move.
        executeMove(board, mymove, player);
        free(mymove);
        
        // Time issue.
        if (compTime < 0) {
            printf("Sorry, the computer ran out of time and loses automatically.\n");
            outOfTime = TRUE;
            break;
        }
        
        // Print the result and go to the other player.
        printBoard(board);
        player = opposite(player);
    }
    
    if (!outOfTime) {
        
        // Get scores.
        int blackScore = score(board, BLACK);
        int whiteScore = score(board, WHITE);
        
        // Print scores.
        printf("Black's score is %d\n", blackScore);
        printf("White's score is %d\n", whiteScore);
        
        // Output the result.
        if (blackScore > whiteScore)
            printf("Black wins by %d points.\n", blackScore - whiteScore);
        else if (whiteScore > blackScore)
            printf("White wins by %d points.\n", whiteScore - blackScore);
        else
            printf("It's a tie!\n");
    }
        
        // Ran out of time.
    else {
        printf("Computer has defaulted and lost.\n");
    }
}

int play(enum piece ourColor) {
    
    // Initialize and print the board. Black goes first.
    enum piece board[SIZE][SIZE];
    initBoard(board);
    //printBoard(board);
    enum piece player = BLACK;
    
    int blackTime = MAXTIME;
    int whiteTime = MAXTIME;
    enum boolean blackOutOfTime = FALSE;
    enum boolean whiteOutOfTime = FALSE;
    
    // Go till the game is done.
    while (!gameOver(board)) {
        
        // We can flip the team if the current team can't move.
        if (!canMove(board, player))
            player = opposite(player);
        
        position *mymove = NULL;
        
        // Black Computer Player
        if (player == ourColor) {
            
            // Do the move and time it.
            int startT = time(0);
            clock_t clock_start = clock();
            mymove = team03Move(board, player, blackTime);
            clock_t clock_end = clock();
            int endT = time(0);
            long long ms = (1000.0 * (clock_end - clock_start)) / CLOCKS_PER_SEC;
            printf("We took %lld ms (counted as %d s)\n", ms, endT - startT);
            
            // Update time.
            blackTime = blackTime - (endT - startT);
            printf("We selected row %d, column %d\n", mymove->x, mymove->y);
        }
            
            // White Computer Player
        else {
            
            // Do the move and time it.
            int startT = time(0);
            mymove = teamnaiveMove(board, player, whiteTime);
            int endT = time(0);
            
            // Update time.
            whiteTime = whiteTime - (endT - startT);
            printf("Opponent selected row %d, column %d\n", mymove->x, mymove->y);
        }
        
        // Execute then free move.
        executeMove(board, mymove, player);
        free(mymove);
        
        // Black ran out of time.
        if (blackTime < 0) {
            printf("Sorry, the Black Player ran out of time and loses automatically.\n");
            blackOutOfTime = TRUE;
            break;
        }
        
        // White ran out of time.
        if (whiteTime < 0) {
            printf("Sorry, the White Player ran out of time and loses automatically.\n");
            whiteOutOfTime = TRUE;
            break;
        }
        
        // Print the result and go to the other player.
        printBoard(board);
        player = opposite(player);
        
        // This waits about one second, so we can see the moves scrolling.
        //int curT = time(0);
        //while (time(0) == curT);
    }
    
    if (!blackOutOfTime && !whiteOutOfTime) {
        
        // Get scores.
        int blackScore = score(board, BLACK);
        int whiteScore = score(board, WHITE);
        
        // Print scores.
        //printf("Black's score is %d\n", blackScore);
        //printf("White's score is %d\n", whiteScore);
        
        // Output the result.
        if (blackScore > whiteScore) {
            if (ourColor == BLACK) printf("We win by %d points.\n", blackScore - whiteScore);
            else printf("They win by %d points.\n", blackScore - whiteScore);
            return 2;
        } else if (whiteScore > blackScore) {
            if (ourColor == WHITE) printf("We win by %d points.\n", whiteScore - blackScore);
            else printf("They win by %d points.\n", whiteScore - blackScore);
            return 0;
        } else {
            printf("It's a tie!\n");
            return 1;
        }
    }
        
        // Black player ran out of time.
    else if (blackOutOfTime) {
        //printf("White wins because the Black Player ran out of time.\n");
        return 0;
    }
        // White player ran out of time.
    else {
        //printf("Black wins because the White Player ran out of time.\n");
        return 2;
    }
}


// 1000 games, 4 depth naive / pure random
// NAIVE: 1729
// RANDOM: 271

// 100 games, 5 depth naive / pure random
// NAIVE: 173
// RANDOM: 27

// 1000 games, 4 depth naive / 4 depth [mymoves - othermoves]
// NAIVE: 0
// SMARTER: 200
// (LMAO)
// oh wait bc there's no randomness it's the same game 100 times shit

// 100 games, 4 depth smart / pure random
// SMARTER: 155
// RANDOM: 45

void test() {
    srand(time(NULL));
    int us = 0;
    int them = 0;
    int tie = 0;
//    int black = 0;
//    int white = 0;
    for (int i = 1; i <= 100; i++) {
        enum piece ourColor = (rand() % 2) ? WHITE : BLACK;
        printf("We are playing %s\n", ourColor == WHITE ? "white" : "black");
        int res = play(ourColor);
        if (res == 1) tie++;
        else {
            enum piece winColor = (res == 0) ? WHITE : BLACK;
            if (winColor == ourColor) us++;
            else them++;
        }
        if (i % 10 == 0) printf("%d\n", i);
    }
    printf("us (team03): %d\nthem (naive): %d\ntied: %d\n", us, them, tie);
}
