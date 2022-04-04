#ifndef BFS_H
#define BFS_H

#include <stdio.h> // for FILE
#include <stdbool.h> // for bool
#include "tetris.h"

#define BFS_LEVEL 4

typedef struct {
    char *board;
    int level;
    // orientation and offset of first move
    int ori;
    int col;
} state;

tetris_block solve(tetris_game* game);

#endif // BFS_H
