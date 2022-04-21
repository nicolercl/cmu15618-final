#ifndef DFS_CILK_H
#define DFS_CILK_H
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "parameters.h"
#include "tetris.h"
#include "helper.h"
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h> //needs to be included to use the addition reducer 
tetris_block dfs_cilk(tetris_game* tg, parameters param);
// void tg_copy(tetris_game *dest, const tetris_game *src);
// int tg_get_score(tetris_game *obj, const parameters param);

#endif
