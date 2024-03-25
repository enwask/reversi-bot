#!/bin/sh
gcc -o reversi framework/reversi.c framework/reversi_functions.c framework/team21.c src/team03.c
./reversi
