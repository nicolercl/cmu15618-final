#ifndef DFS_SOLVER_H
#define DFS_SOLVER_H
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "parameters.h"
#include "tetris.h"
tetris_block dfs_solver(tetris_game* tg, parameters param);
void tg_copy(tetris_game *dest, const tetris_game *src);
int tg_get_score(tetris_game *obj, const parameters param);

#endif
