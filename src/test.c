#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include "team03.h"

int stateEquals(board_t a, board_t b) { return a.on == b.on && a.color == b.color; }

void setPieces(board_t *state, pos_t pos1, pos_t pos2, int col) {
    int8_t x1 = pos1.x, y1 = pos1.y;
    int8_t x2 = pos2.x, y2 = pos2.y;
    if (y1 > y2 || (y1 == y2 && x1 > x2)) {
        int8_t tmp = y1;
        y1 = y2, y2 = tmp;
        tmp = x1;
        x1 = x2, x2 = tmp;
    }
    
    if (x1 == x2) {
        for (int8_t y = y1; y <= y2; y++) {
            team03_setPiece(state, team03_makePos(y, x1), col);
        }
        return;
    }
    
    if (y1 == y2) {
        for (int8_t x = x1; x <= x2; x++) {
            team03_setPiece(state, team03_makePos(y1, x), col);
        }
        return;
    }
    
    for (int8_t y = y1; y <= y2; y++) {
        if (x1 <= x2) {
            for (int8_t x = x1; x <= x2; x++) {
                if (abs((signed) x - x1) != abs((signed) y - y1)) continue;
                team03_setPiece(state, team03_makePos(y, x), col);
            }
            continue;
        }
        
        for (int8_t x = x1; x <= x1 && x >= x2; x--) {
            if (abs((signed) x - x1) != abs((signed) y - y1)) continue;
            team03_setPiece(state, team03_makePos(y, x), col);
        }
    }
}

int main(void) {
    board_t fast;
    fast.on = fast.color = 0;

//    for (int i = 0; i < 100000; i++) {
    for (int8_t y1 = 0; y1 < 8; y1++) {
        for (int8_t x1 = 0; x1 < 8; x1++) {
            for (int8_t y2 = 0; y2 < 8; y2++) {
                for (int8_t x2 = 0; x2 < 8; x2++) {
                    int col = (y1 + x1 + y2 + x2) % 2;
                    if (!(x1 == x2
                          || y1 == y2
                          || (abs(x1 - x2) == abs(y1 - y2))))
                        continue;
                    
                    pos_t pos1 = team03_makePos(y1, x1);
                    pos_t pos2 = team03_makePos(y2, x2);
                    
                    board_t st1 = fast, st2 = fast;
                    setPieces(&st1, pos1, pos2, col);
                    team03_setPieces(&st2, pos1, pos2, col);
                    
                    if (!stateEquals(st1, st2)) {
                        printf("oh no :(\n");
                        
                        printf("Expected:\n");
                        team03_print(st1);
                        
                        printf("\nGot:\n");
                        team03_print(st2);
                        assert(0);
                    }
                }
            }
        }
    }
    printf("OK!\n");
//    }

////    int64_t mask = team03_rangeMask(0, 63);
////    printf("rangeMask(0,63):\n");
////    for (int i = 0; i < 64; i++) printf("%d", team03_getBit(mask, i));
////    printf("\n");
//
//    board_t state;
//    state.on = state.color = 0;
//    for (int i = 0; i < 64; i++) team03_setBit(&state.on, i, 1);
//    while (1) {
//        unsigned x1, y1, x2, y2;
//        printf("x1, y1, x2, y2:\n> ");
//        scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
//
//        team03_flip(&state, team03_makePos(y1, x1), team03_makePos(y2, x2));
//        team03_print(state);
//        puts("\n\n");
//    }
}
