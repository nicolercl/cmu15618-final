#ifndef SOLVER_H
#define SOLVER_H
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "parameters.h"
#include "tetris.h"
tetris_block solver(tetris_game *tg, parameters param, int depth, int nthreads);
#endif
