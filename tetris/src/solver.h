#ifndef SOLVER_H
#define SOLVER_H
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "parameters.h"
#include "tetris.h"
void solver(tetris_game *tg, parameters param, int depth, int nthreads, tetris_block *result);
#endif
