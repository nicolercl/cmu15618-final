#include "solver.h"
#include "parameters.h"
#include "sys/time.h"
#include "omp.h"
#include "helper.h"
#include <assert.h>
#define DEBUG 0
#define DEBUG2 0
#define TIME 1

void openmp_solve(tetris_game *tg, const parameters param, int depth, int nthreads, tetris_block *result){
    FILE *f = 0;
    if(DEBUG){
        f = fopen("tetris.log", "a");
        fprintf(f, "SOLVE!\n");
    }
    float min_total_t = INT_MAX, min_solve_t = INT_MAX, min_copy_t = INT_MAX, min_drop_t = INT_MAX, min_crit_t = INT_MAX;
    float max_total_t = INT_MIN, max_solve_t = INT_MIN, max_copy_t = INT_MIN, max_drop_t = INT_MIN, max_crit_t = INT_MIN;
    float avg_total_t = 0, avg_solve_t = 0, avg_copy_t = 0, avg_drop_t = 0, avg_crit_t = 0;
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
        
    	struct timeval time; 
    	float solve_t = 0, copy_t = 0, drop_t = 0, crit_t = 0;
	suseconds_t start_t, end_t;
	suseconds_t total_start, total_end;
        gettimeofday(&time,NULL);
        total_start = time.tv_sec * 1000000 + time.tv_usec;
        tetris_game *solver_tg;
        int chunk_i = omp_get_thread_num();
        int local_best_score = INT_MAX;
        int local_best_ori;
        int local_best_tcol;
        int tcol[4], ori[4];
		gettimeofday(&time,NULL);
		start_t = time.tv_sec * 1000000 + time.tv_usec;
		// solve bottleneck for DEPTH 2
  		solver_tg = (tetris_game*)malloc(sizeof(tetris_game));
  		tg_init(solver_tg, tg->rows, tg->cols);
       // solver_tg = tg_create(tg->rows, tg->cols);
		gettimeofday(&time,NULL);
		end_t = time.tv_sec * 1000000 + time.tv_usec;
		crit_t += (float)(end_t - start_t);
        int tetris_id, lines_cleared;
        int start = chunk_i * chunk_size;
        int end   = MIN(start + chunk_size, total_combination);
        for(int idx = start; idx < end; idx++) {
            int id = idx;
            int div = 1;
            gettimeofday(&time,NULL);
            start_t = time.tv_sec * 1000000 + time.tv_usec;
            gettimeofday(&time,NULL);
            tg_copy(solver_tg, tg);
            end_t = time.tv_sec * 1000000 + time.tv_usec;
	    	copy_t += (float)(end_t - start_t);
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

            gettimeofday(&time,NULL);
            start_t = time.tv_sec * 1000000 + time.tv_usec;
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
            gettimeofday(&time,NULL);
            end_t = time.tv_sec * 1000000 + time.tv_usec;
	    drop_t += (float)(end_t - start_t);

            gettimeofday(&time,NULL);
            start_t = time.tv_sec * 1000000 + time.tv_usec;
            tg_remove(solver_tg, solver_tg->falling);
            int score = tg_get_score(solver_tg, param);
            tg_put(solver_tg, solver_tg->falling);
            gettimeofday(&time,NULL);
            end_t = time.tv_sec * 1000000 + time.tv_usec;
	    solve_t += (float)(end_t - start_t);

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
        gettimeofday(&time,NULL);
        total_end = time.tv_sec * 1000000 + time.tv_usec;

		float total_t = (float)(total_end - total_start);
		if(TIME){
			#pragma omp critical
			{
				avg_total_t += total_t; 
				avg_solve_t += solve_t;
				avg_drop_t  += drop_t;
				avg_copy_t  += copy_t;
				avg_crit_t  += crit_t;
				get_min_max(total_t, &min_total_t, 0);
				get_min_max(solve_t, &min_solve_t, 0);
				get_min_max(copy_t, &min_copy_t, 0);
				get_min_max(drop_t, &min_drop_t, 0);
				get_min_max(crit_t, &min_crit_t, 0);
				get_min_max(total_t, &max_total_t, 1);
				get_min_max(solve_t, &max_solve_t, 1);
				get_min_max(copy_t, &max_copy_t, 1);
				get_min_max(drop_t, &max_drop_t, 1);
				get_min_max(crit_t, &max_crit_t, 1);
			}
		}
    }

    if(DEBUG2) printf("global best %d c %d o %d\n", global_best_score, global_col, global_ori);

    if(TIME){
		if(min_crit_t == 0) min_crit_t = 1;
		if(min_solve_t == 0) min_solve_t = 1;
		if(min_drop_t == 0) min_drop_t = 1;
		if(min_copy_t == 0) min_copy_t = 1;

        printf("avg_total:%f avg_solve:%f avg_drop:%f avg_copy:%f avg_crit:%f ", avg_total_t / (float)nthreads, avg_solve_t / (float)nthreads, avg_drop_t / (float)nthreads, avg_copy_t / (float)nthreads,
			avg_crit_t / (float)nthreads);
		printf("ratio_total_t:%f ratio_solve_t:%f ratio_drop_t:%f ratio_copy_t:%f ratio_crit_t:%f\n", max_total_t / min_total_t, 
			max_solve_t / min_solve_t, max_drop_t / min_drop_t, max_copy_t / min_copy_t, max_crit_t / min_crit_t);
		fflush(stdout);
    }

    result->ori = global_ori;
    result->loc.col = global_col;

    free(combinations);
    if(DEBUG) fclose(f);
}

void solver(tetris_game *tg, parameters param, int depth, int nthreads, tetris_block* result){

    tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
    // before we solve, turn off rand()
    tg->use_random = 0;
    tg_copy(solver_tg, tg);
    openmp_solve(solver_tg, param, depth, nthreads, result);
    tg_delete(solver_tg);
    tg->use_random = 1;
}
