#include <stdio.h>
#include <stdlib.h>
#include "team03.h"

int main(void) {
//    uint64_t mask = team03_rangeMask(0, 63);
//    printf("rangeMask(0,63):\n");
//    for (int i = 0; i < 64; i++) printf("%d", team03_getBit(mask, i));
//    printf("\n");
    
    board_t state;
    state.on = state.color = 0;
    for (int i = 0; i < 64; i++) team03_setBit(&state.on, i, 1);
    while (1) {
        unsigned x1, y1, x2, y2;
        printf("x1, y1, x2, y2:\n> ");
        scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
        
        team03_flip(&state, team03_makePos(y1, x1), team03_makePos(y2, x2));
        team03_print(state);
        puts("\n\n");
    }
}
