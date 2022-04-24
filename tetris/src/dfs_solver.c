#include "dfs_solver.h"
#include "parameters.h"
#include "sys/time.h"
#include "omp.h"
#include "helper.h"

#define DEPTH 4
// return the best position for tg->falling
// we use block->typ to store the height
// and row to store the points
tetris_block dfs_solve(const tetris_game *tg, const parameters param, int depth){
    // we reach the end, calculate the height and return.
    if(depth == DEPTH){
        tetris_block pos;
        tg_remove(tg, tg->falling);
        pos.typ = tg_get_score(tg, param);
        tg_put(tg, tg->falling);
        return pos;
    }

    FILE *f;
    if(0){
        f = fopen("tetris.log", "a");
    }

    int orientation, col_position, lines_cleared;
    tetris_block best_pos, next_pos;
    best_pos.typ = INT_MAX;
    tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
    for(int o = 0; o < NUM_ORIENTATIONS; o++){
        for(int c = -1; c < tg->cols; c++){
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
            next_pos = dfs_solve(solver_tg, param, depth+1);

            if(next_pos.typ < best_pos.typ){
                best_pos.typ = next_pos.typ;
                best_pos.loc.col = col_position;
                best_pos.ori = orientation;
            }
        }
    } 
    
    // fclose(f);
    return best_pos;
}

void openmp_dfs_solve(const tetris_game *tg, const parameters param, int depth, tetris_block *result){

    int combination = NUM_ORIENTATIONS * tg->cols;
    tetris_block best_pos;
    best_pos.typ = INT_MAX;
    #pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int idx      = omp_get_thread_num();
        tetris_block next_pos;
        int o, c;
        int orientation, col_position, lines_cleared;
        tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
        while(idx < combination){
            struct timeval time; 
            gettimeofday(&time,NULL);
            suseconds_t start = time.tv_sec * 1000000 + time.tv_usec;
            o = idx % NUM_ORIENTATIONS;
            c = idx / NUM_ORIENTATIONS;
            tg_copy(solver_tg, tg);

            tg_move(solver_tg, -solver_tg->falling.loc.col);

            tg_rotate(solver_tg, o);

            tg_move(solver_tg, c);

            orientation = solver_tg->falling.ori;
            col_position = solver_tg->falling.loc.col;

            tg_down(solver_tg);
            tg_put(solver_tg, solver_tg->falling);
            lines_cleared = tg_check_lines(solver_tg);
            tg_adjust_score(solver_tg, lines_cleared);
            
            next_pos = dfs_solve(solver_tg, param, depth+1);
            
            gettimeofday(&time,NULL);
            suseconds_t end = time.tv_sec * 1000000 + time.tv_usec;
    //        printf("idx: %d took %fs\n", idx, (float)(end - start) / 1000000.0);
    //        fflush(stdout);
            #pragma omp critical
            if(next_pos.typ < best_pos.typ){
                best_pos.typ = next_pos.typ;
                result->loc.col = col_position;
                result->ori = orientation;
            }
            idx += nthreads;
        }
    }
    
    // fclose(f);
}

void dfs_solver(tetris_game *tg, parameters param, tetris_block *result){

    struct timeval time; 
    gettimeofday(&time,NULL);
    suseconds_t start = time.tv_sec * 1000000 + time.tv_usec;
    tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
    // before we solve, turn off rand()
    tg->use_random = 0;
    tg_copy(solver_tg, tg);
    openmp_dfs_solve(solver_tg, param, 0, result);
    tg_delete(solver_tg);
    tg->use_random = 1;
    gettimeofday(&time,NULL);
    suseconds_t end = time.tv_sec * 1000000 + time.tv_usec;
//    printf("Total: %f\n", (float)(end - start) / 1000000.0);
 //   fflush(stdout);
}
