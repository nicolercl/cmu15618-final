#include "dfs_solver.h"
#include "sys/time.h"
#include "omp.h"

#define DEPTH 4
#define DEBUG 0
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
    int up_degree = 2;
    int side_degree = 2;

    for(int i = 0; i < obj->rows; i++){
        for(int j = 0; j < obj->cols; j++){
            if(TC_IS_FILLED(tg_get(obj, i, j)))
                continue;

            if(tg_check(obj, i-1, j) && TC_IS_FILLED(tg_get(obj, i-1, j))){
                score += tg_poly(obj->rows - i, up_degree);
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
    int row;
    for(row = obj->rows - 1; row >= 0; row--){
        if(tg_line_empty(obj, row))
            break;
    }
    return obj->rows - 1 - row;
}

int tg_get_score(tetris_game *obj){
    int score = 0;
    int h =  tg_get_height(obj);
    int holes = tg_get_holes_score(obj);
    int bump  = tg_get_bumpiness(obj);
    score += 10 * h * h;
    score += holes;
    return score;
}

void tg_copy(tetris_game *dest, tetris_game *src){
    // copy board 
    memcpy(dest->board, src->board, sizeof(char) * src->rows * src->cols);
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
}

// return the best position for tg->falling
// we use block->typ to store the height
// and row to store the points
tetris_block dfs_solve(tetris_game *tg, int depth){
    // we reach the end, calculate the height and return.
    if(depth == DEPTH){
        tetris_block pos;
        pos.typ = tg_get_score(tg);
        return pos;
    }

    FILE *f;
    if(0){
        f = fopen("tetris.log", "a");
    }

    int orientation, col_position, lines_cleared;
    tetris_block best_pos, next_pos;
    best_pos.typ = INT_MAX;
    for(int o = 0; o < NUM_ORIENTATIONS; o++){

        for(int c = 0; c < tg->cols; c++){
            tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
            tg_copy(solver_tg, tg);

            // first we move the piece to the leftmost
            // (loc.col == 0)
            tg_move(solver_tg, -solver_tg->falling.loc.col);

            // then we rotate the piece
            tg_rotate(solver_tg, o);

            // move the piece to position
            tg_move(solver_tg, c);

            // save the ori, loc infomation
            orientation = solver_tg->falling.ori;
            col_position = solver_tg->falling.loc.col;

            // we try this position and update the board
            tg_down(solver_tg);
            lines_cleared = tg_check_lines(solver_tg);
            tg_adjust_score(solver_tg, lines_cleared);
            

            // this would give us the best pos and score of next block
            // since we already falled down current block
            next_pos = dfs_solve(solver_tg, depth+1);

            if(0){
                fprintf(f, "O %d C: %d H: %d RH: %d\n", o, c, next_pos.typ, tg_get_score(solver_tg));
                tg_print(solver_tg, f);
            }

            if(next_pos.typ < best_pos.typ){
                if(0){
                    fprintf(f, "SOLUTION O %d C: %d H: %d RH: %d\n", o, c, next_pos.typ, tg_get_score(solver_tg));
                    tg_print(solver_tg, f);
                }

                best_pos.typ = next_pos.typ;
                best_pos.loc.col = col_position;
                best_pos.ori = orientation;
            }

            // resume the board
            tg_delete(solver_tg);
            //tg_copy(solver_tg, tg);
        }
    } 
    
    // fclose(f);
    return best_pos;
}

tetris_block openmp_dfs_solve(tetris_game *tg, int depth){

    int combination = NUM_ORIENTATIONS * tg->cols;
    tetris_block best_pos;
    best_pos.typ = INT_MAX;
    #pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int idx      = omp_get_thread_num();
        tetris_block next_pos;
        while(idx < combination){
            struct timeval time; 
            gettimeofday(&time,NULL);
            suseconds_t start = time.tv_sec * 1000000 + time.tv_usec;
            int o, c;
            int orientation, col_position, lines_cleared;
            o = idx / tg->cols;
            c = idx % tg->cols;
            tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
            tg_copy(solver_tg, tg);

            tg_move(solver_tg, -solver_tg->falling.loc.col);

            tg_rotate(solver_tg, o);

            tg_move(solver_tg, c);

            orientation = solver_tg->falling.ori;
            col_position = solver_tg->falling.loc.col;

            tg_down(solver_tg);
            lines_cleared = tg_check_lines(solver_tg);
            tg_adjust_score(solver_tg, lines_cleared);
            
            next_pos = dfs_solve(solver_tg, depth+1);
            
            gettimeofday(&time,NULL);
            suseconds_t end = time.tv_sec * 1000000 + time.tv_usec;
            printf("idx: %d took %fs\n", idx, (float)(end - start) / 1000000.0);
            fflush(stdout);
            #pragma omp critical
            if(next_pos.typ < best_pos.typ){
                if(DEBUG){
                    FILE *f;
                    f = fopen("tetris.log", "a");
                    fprintf(f, "SOLUTION Thread: %d O %d C: %d H: %d RH: %d\n", idx, o, c, next_pos.typ, tg_get_score(solver_tg));
                    tg_print(solver_tg, f);
                    fclose(f);
                }

                best_pos.typ = next_pos.typ;
                best_pos.loc.col = col_position;
                best_pos.ori = orientation;
            }

            tg_delete(solver_tg);
            idx += nthreads;
        }
    }
    
    // fclose(f);
    return best_pos;
}

tetris_block dfs_solver(tetris_game *tg){

    struct timeval time; 
    gettimeofday(&time,NULL);
    suseconds_t start = time.tv_sec * 1000000 + time.tv_usec;
    tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
    tg_copy(solver_tg, tg);
    tetris_block result;
    result = openmp_dfs_solve(solver_tg, 0);
    tg_delete(solver_tg);
    gettimeofday(&time,NULL);
    suseconds_t end = time.tv_sec * 1000000 + time.tv_usec;
    printf("Total: %f\n", (float)(end - start) / 1000000.0);
    fflush(stdout);
    return result;
}
