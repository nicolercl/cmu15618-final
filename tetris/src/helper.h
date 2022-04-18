#ifndef HELPER_H 
#define HELPER_H 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "parameters.h"
#include "tetris.h"

bool tg_line_empty(tetris_game *obj, int i);
int tg_get_column(tetris_game *obj, int col);
int tg_poly(int x, int degree);
int tg_get_holes_score(tetris_game *obj);
int tg_get_bumpiness(tetris_game *obj);
int tg_get_height(tetris_game *obj);
int tg_get_score(tetris_game *obj, const parameters param);
void tg_copy(tetris_game *dest, const tetris_game *src);
#endif
