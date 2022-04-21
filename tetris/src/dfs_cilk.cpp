#include "dfs_cilk.h"
#include "parameters.h"
#include "sys/time.h"

#define DEPTH 2
#define DEBUG 0

struct min_comp
{
    bool operator() (const tetris_block &block1, const tetris_block &block2) const
    {
        return block1.typ < block2.typ;
    }
};

tetris_block dfs_solve(tetris_game *tg, const parameters param, int depth){
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

    // int orientation, col_position, lines_cleared;
    tetris_block best_pos = tg->falling;
    best_pos.typ = INT_MAX;
    cilk::reducer_min<tetris_block, min_comp> reducer_best_pos;

    cilk_for (int o = 0; o < NUM_ORIENTATIONS; o++){
        cilk_for (int c = 0; c < tg->cols; c++){
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
            int orientation = solver_tg->falling.ori;
            int col_position = solver_tg->falling.loc.col;

            // we try this position and update the board
            tg_down(solver_tg);
            int lines_cleared = tg_check_lines(solver_tg);
            tg_adjust_score(solver_tg, lines_cleared);
            

            // this would give us the best pos and score of next block
            // since we already falled down current block
            tetris_block next_pos = dfs_solve(solver_tg, param, depth+1);
            next_pos.loc.col = col_position;
            next_pos.ori = orientation;

            *reducer_best_pos = cilk::min_of(*reducer_best_pos, next_pos);
        }
    }

    // cilk_sync;
    
    // fclose(f);
    best_pos = reducer_best_pos.get_value();
    return best_pos;
}

tetris_block dfs_cilk(tetris_game* tg, parameters param) {
    struct timeval time; 
    gettimeofday(&time,NULL);
    suseconds_t start = time.tv_sec * 1000000 + time.tv_usec;
    tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
    // before we solve, turn off rand()
    tg->use_random = 0;
    tg_copy(solver_tg, tg);
    tetris_block result;
    result = dfs_solve(solver_tg, param, 0);
    tg_delete(solver_tg);
    tg->use_random = 1;
    gettimeofday(&time,NULL);
    suseconds_t end = time.tv_sec * 1000000 + time.tv_usec;
    printf("Total: %f\n", (float)(end - start) / 1000000.0);
    fflush(stdout);
    return result;
}
