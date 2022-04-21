#ifndef DFS_CILK_H
#define DFS_CILK_H
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "parameters.h"
#include "tetris.h"
#define TABLE_SIZE 100000000

tetris_block dfs_solver_dp(tetris_game* tg, parameters param, char ** dp);


#endif
