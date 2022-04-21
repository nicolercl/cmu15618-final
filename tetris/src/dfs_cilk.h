#ifndef DFS_CILK_H
#define DFS_CILK_H
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "parameters.h"
#include "tetris.h"
#include "helper.h"
#include <cilk/cilk.h>
#include <cilk/reducer_min.h> //needs to be included to use the addition reducer
tetris_block dfs_cilk(tetris_game* tg, parameters param);

#endif
