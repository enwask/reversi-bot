#!/bin/sh
gcc -o reversi src/reversi.c src/reversi_functions.c src/team03.c rivals/teamnaive.c rivals/teamrand.c
