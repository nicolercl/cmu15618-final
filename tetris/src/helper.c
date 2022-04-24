#include "helper.h"
#include <assert.h>
bool tg_line_empty(tetris_game *obj, int i){
    for(int j = 0; j < obj->cols; j++){
        if(TC_IS_FILLED(tg_get(obj, i, j)))
            return false;
    }
    return true;
}

int tg_get_column(tetris_game *obj, int col){
    int count = 0;
    for(int i = 0; i < obj->rows; i++){
       if(TC_IS_FILLED(tg_get(obj, i, col))) 
           count++;
    }
    return count;
}

int tg_poly(int x, int degree){
    if(degree == -1)
        return 0;

    int result = 1;
    for(int i = 0; i < degree; i++)
        result *= x;
    return result;
}

int tg_get_holes_score(tetris_game *obj){
    int score = 0;
    int up_degree = 3;
    int side_degree = 2;

    for(int i = 0; i < obj->rows; i++){
        for(int j = 0; j < obj->cols; j++){
            if(TC_IS_FILLED(tg_get(obj, i, j)))
                continue;

            for(int k = i-1; k >= i-1 ; k--){
                if(tg_check(obj, k, j) && TC_IS_FILLED(tg_get(obj, k, j))){
                    score += tg_poly(obj->rows - i, up_degree);
                }
            }

            if(tg_check(obj, i, j+1) && TC_IS_FILLED(tg_get(obj, i, j+1))){
                score += tg_poly(obj->rows - i, side_degree);
            }

            if(tg_check(obj, i, j-1) && TC_IS_FILLED(tg_get(obj, i, j-1))){
                score += tg_poly(obj->rows - i, side_degree);
            }
        }
    }
    return score;
}

int tg_get_bumpiness(tetris_game *obj){
    int prev = tg_get_column(obj, 0);
    int cur, sum = 0;

    for(int i = 1; i < obj->cols; i++){
        cur = tg_get_column(obj, i);
        sum += abs(prev - cur);
        prev = cur;
    }

    return sum;
}

int tg_get_height(tetris_game *obj){
    return obj->height;
    /*
    int row;
    for(row = 0; row < obj->rows; row++){
        if(!tg_line_empty(obj, row))
            break;
    }
    return obj->rows - 1 - row;
    */
}

int tg_get_score(tetris_game *obj, const parameters param){
    float score = 0;
    float h =  (float)tg_get_height(obj);
    float holes = (float)tg_get_holes_score(obj);
    float bump  = (float)tg_get_bumpiness(obj);
    score += param.weights[0] * h * h;
    score += param.weights[1] * holes;
    score += param.weights[2] * bump;
    if(obj->line_cleared >= 2)
        score -= param.weights[3] * (obj->line_cleared * obj->line_cleared);
    return (int)score;
}

void tg_copy(tetris_game *dest, const tetris_game *src){
    // copy board 
    assert(src->rows == 22);
    memcpy(dest->board, src->board, sizeof(char) * src->rows * src->cols);
     for(int i = 0; i < src->rows; i++)
         for(int j = 0; j < src->cols; j++)
             dest->board[i * src->cols + j] = src->board[i * src->cols + j] ;
    
    // copy next blocks
    memcpy(dest->next, src->next, sizeof(tetris_block) * NEXT_N);
    dest->rows = src->rows;
    dest->cols = src->cols;
    dest->points = src->points;
    dest->level  = src->level;
    dest->ticks_till_gravity = src->ticks_till_gravity;
    dest->lines_remaining = src->lines_remaining;
    dest->falling = src->falling;
    dest->stored  = src->stored;
    dest->use_random = src->use_random;
    dest->height = src->height;
}

