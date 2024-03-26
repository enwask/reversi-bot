#!/bin/sh
gcc -o reversi_test src/test.c framework/reversi_functions.c src/team03.c
sleep 1
./reversi_test
