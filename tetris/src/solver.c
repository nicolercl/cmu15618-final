#include "solver.h"
#include "parameters.h"
#include "sys/time.h"
#include "omp.h"
#include "helper.h"
#define DEBUG 0

tetris_block openmp_solve(tetris_game *tg, const parameters param, int depth, int nthreads){
    FILE *f;
    if(DEBUG){
        f = fopen("tetris.log", "a");
    }

    tetris_block global_best_pos;
    global_best_pos.typ = INT_MAX;
    #pragma omp parallel 
    {
        int chunk_size, num_of_chunk, total_combination = 1;
        int *combinations = (int*)malloc(sizeof(int) * depth);
        combinations[0] = (tg->cols * tetris_orientation_number[tg->falling.typ]);
        for(int i = 1; i < depth; i++){
            combinations[i] = \
            (tg->cols * tetris_orientation_number[tg->next[i-1].typ]);
        }
        for(int i = 0; i < depth; i++)
            total_combination *= combinations[i];
        chunk_size = (total_combination + nthreads - 1) / nthreads;
        num_of_chunk = (total_combination + chunk_size - 1) / chunk_size;
        //printf("nthreads %d size %d \n", nthreads, chunk_size * num_of_chunk);
        
        tetris_game *solver_tg;
        int chunk_i = omp_get_thread_num();
        int *ori = (int*)malloc(sizeof(int) * depth);
        int *tcol = (int*)malloc(sizeof(int) * depth);
        solver_tg = tg_create(tg->rows, tg->cols);
        int tetris_id, lines_cleared;
        while(chunk_i < num_of_chunk){
            int tidx      = omp_get_thread_num();
            struct timeval time; 
            gettimeofday(&time,NULL);
            suseconds_t start_time = time.tv_sec * 1000000 + time.tv_usec;
            tetris_block local_best_pos;
            local_best_pos.typ = INT_MAX;
            int start = chunk_i * chunk_size;
            int end   = start + chunk_size;
            for(int idx = start; idx < end; idx++) {
                int id = idx;
                int div = 1;
                tg_copy(solver_tg, tg);
                // bcd
                for(int j = 1; j < depth; j++)
                   div *= combinations[j];
                for(int j = 0; j < depth; j++){
                    tetris_id = id / div;
                    id = id % div;
                    if(j + 1 < depth) div /= combinations[j+1];
                    ori[j] = tetris_id / solver_tg->cols;
                    tcol[j] = tetris_id % solver_tg->cols ; 
                }
                int o, c;
                for(int j = 0; j < depth; j++){
                    tg_move(solver_tg, -solver_tg->falling.loc.col);
                    tg_rotate(solver_tg, -solver_tg->falling.ori);

                    // then we rotate the piece
                    tg_rotate(solver_tg, ori[j]);

                    // move the piece to position
                    tg_move(solver_tg, tcol[j]);

                    // record the current pos and ori
                    if(j == 0){
                        o = solver_tg->falling.ori;
                        c = solver_tg->falling.loc.col;
                    }

                    tg_down(solver_tg);
                    lines_cleared = tg_check_lines(solver_tg);
                    tg_adjust_score(solver_tg, lines_cleared);
                }

                if(DEBUG) tg_print(solver_tg, f);

                int score = tg_get_score(solver_tg, param);
                if(score < local_best_pos.typ){
                    local_best_pos.typ = score;
                    local_best_pos.loc.col = c;
                    local_best_pos.ori = o;
                }
            }

            #pragma omp critical
            if(local_best_pos.typ < global_best_pos.typ){
                global_best_pos.typ = local_best_pos.typ;
                global_best_pos.loc.col = local_best_pos.loc.col;
                global_best_pos.ori = local_best_pos.ori;
            }

            gettimeofday(&time,NULL);
            suseconds_t end_time = time.tv_sec * 1000000 + time.tv_usec;
            chunk_i += nthreads;
        }
        free(ori);
        free(tcol);
        tg_delete(solver_tg);
        free(combinations);
    }

    return global_best_pos;
}

tetris_block solver(tetris_game *tg, parameters param, int depth, int nthreads){

    struct timeval time; 
    gettimeofday(&time,NULL);
    suseconds_t start = time.tv_sec * 1000000 + time.tv_usec;
    tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
    // before we solve, turn off rand()
    tg->use_random = 0;
    tg_copy(solver_tg, tg);
    tetris_block result;
    result = openmp_solve(solver_tg, param, depth, nthreads);
    tg_delete(solver_tg);
    tg->use_random = 1;
    gettimeofday(&time,NULL);
    suseconds_t end = time.tv_sec * 1000000 + time.tv_usec;
//    printf("Total: %f\n", (float)(end - start) / 1000000.0);
 //   fflush(stdout);
    return result;
}
