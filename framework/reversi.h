#ifndef REVERSI_H_INCLUDED
#define REVERSI_H_INCLUDED

#include "reversi_functions.h"
#define MAXTIME 40

void humanVHuman();
void humanVComputer();
void computerVComputer();
int play(enum piece ourColor);
void test();

#endif // REVERSI_H_INCLUDED
