#include "solver.h"
#include "parameters.h"
#include "sys/time.h"
#include "omp.h"
#include "helper.h"
#include <assert.h>
#define DEBUG 0
#define DEBUG2 0

void openmp_solve(tetris_game *tg, const parameters param, int depth, int nthreads, tetris_block *result){
    FILE *f;
    if(DEBUG){
        f = fopen("tetris.log", "a");
        fprintf(f, "SOLVE!\n");
    }

    int global_best_score = INT_MAX;
    int global_ori, global_col;
    int chunk_size, total_combination = 1;
    int *combinations = (int*)malloc(sizeof(int) * depth);
    combinations[0] = (tg->cols * tetris_orientation_number[tg->falling.typ]);
    for(int i = 1; i < depth; i++){
        combinations[i] = \
        (tg->cols * tetris_orientation_number[tg->next[i-1].typ]);
    }
    for(int i = 0; i < depth; i++)
        total_combination *= combinations[i];

    chunk_size = (total_combination + nthreads - 1) / nthreads;
    #pragma omp parallel 
    {
        //printf("nthreads %d size %d \n", nthreads, chunk_size * num_of_chunk);
        
        tetris_game *solver_tg;
        int chunk_i = omp_get_thread_num();
        int local_best_score = INT_MAX;
        int local_best_ori;
        int local_best_tcol;
        int tcol[4], ori[4];
        solver_tg = tg_create(tg->rows, tg->cols);
        int tetris_id, lines_cleared;
        int start = chunk_i * chunk_size;
        int end   = MIN(start + chunk_size, total_combination);
        for(int idx = start; idx < end; idx++) {
            int id = idx;
            int div = 1;
            tg_copy(solver_tg, tg);
            solver_tg->line_cleared = 0;
            // bcd
            for(int j = 1; j < depth; j++)
               div *= combinations[j];

            for(int j = 0; j < depth; j++){
                tetris_id = id / div;
                id = id % div;
                if(j + 1 < depth) div /= combinations[j+1];
                ori[j] = tetris_id / solver_tg->cols;
                tcol[j] = tetris_id % solver_tg->cols - 1; 
                // assert
                if(j == depth - 1) assert(div == 1);
            }

            for(int j = 0; j < depth; j++){
                tg_move(solver_tg, -solver_tg->falling.loc.col);
                tg_rotate(solver_tg, -solver_tg->falling.ori);

                // then we rotate the piece
                tg_rotate(solver_tg, ori[j]);

                // move the piece to position
                tg_move(solver_tg, tcol[j]);

                tg_down(solver_tg);
                lines_cleared = tg_check_lines(solver_tg);
                tg_adjust_score(solver_tg, lines_cleared);
            }

            tg_remove(solver_tg, solver_tg->falling);
            int score = tg_get_score(solver_tg, param);
            tg_put(solver_tg, solver_tg->falling);

            if(score < local_best_score){
                local_best_score = score;
                local_best_ori = ori[0];
                local_best_tcol = tcol[0];
                if(DEBUG) {
                    tg_remove(solver_tg, solver_tg->falling);
                    fprintf(f, "Best Score: %d\n", local_best_score);
                    fprintf(f, "Height: %d\n", tg_get_height(solver_tg));
                    tg_print(solver_tg, f);
                    tg_put(solver_tg, solver_tg->falling);
                }
            }
        }
        if (DEBUG2) printf("thread %d local best %d c %d o %d\n", chunk_i, local_best_score, local_best_tcol, local_best_ori);

        #pragma omp critical
        {
            if(local_best_score < global_best_score){
                global_best_score = local_best_score;
                global_ori = local_best_ori;
                global_col = local_best_tcol;
            }

        }
        tg_delete(solver_tg);
    }

    if(DEBUG2) printf("global best %d c %d o %d\n", global_best_score, global_col, global_ori);
    result->ori = global_ori;
    result->loc.col = global_col;

    free(combinations);
    if(DEBUG) fclose(f);
}

void solver(tetris_game *tg, parameters param, int depth, int nthreads, tetris_block* result){

    struct timeval time; 
    gettimeofday(&time,NULL);
    suseconds_t start = time.tv_sec * 1000000 + time.tv_usec;
    tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
    // before we solve, turn off rand()
    tg->use_random = 0;
    tg_copy(solver_tg, tg);
    openmp_solve(solver_tg, param, depth, nthreads, result);
    tg_delete(solver_tg);
    tg->use_random = 1;
    gettimeofday(&time,NULL);
    suseconds_t end = time.tv_sec * 1000000 + time.tv_usec;
    return result;
}
