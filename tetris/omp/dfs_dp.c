#include "dfs_dp.h"
#include "sys/time.h"
#include "dfs_solver.h"
#define DEPTH 4
#define DEBUG 0

// djb2 hash
int hash(char *str){
    unsigned long hash = 5381;
    int c;

    while (c = *str++) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % TABLE_SIZE;
}

char tg_get_const(const tetris_game *obj, int row, int column)
{
  return obj->board[obj->cols * row + column];
}

char* get_skyline(const tetris_game *tg) {
    char * skyline = malloc(tg->cols + 1);
    int mn_height = tg->rows - 1;
    for (int i = 0; i < tg->cols; i++) {
        for (int j = tg->rows-1; j >= 0; j++) {
            if (!TC_IS_EMPTY(tg_get_const(tg, i, j))) {
                skyline[i] = j;
                if (mn_height > j) {
                    mn_height = j;
                }
                break;
            }
        }
    }
    for (int i = 0; i < tg->cols; i++) {
        skyline[i] -= mn_height;
    }
    return skyline;
}

void delete_skyline(char* skyline) {
    free(skyline);
}

char *store_best(tetris_block block){
    char *best = malloc(3);
    best[0] = '0' + block.loc.col;
    best[1] = '0' + block.ori;
    best[2] = '\0';
    return best;
}

tetris_block dfs_dp(const tetris_game *tg, const parameters param, int depth, char ** dp){
    // we reach the end, calculate the height and return.
    if(depth == DEPTH){
        tetris_block pos;
        pos.typ = tg_get_score(tg, param);
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
    char *skyline = get_skyline(tg);
    skyline[tg->cols] = tg->falling.typ;

    int h = hash(skyline);
    char *str = dp[h];
    // return hashed result
    if (str != NULL) {
        best_pos.loc.col = str[0] - '0';
        best_pos.ori = str[1] - '0';
        // printf("Best position from hash: %d, %d", best_pos.loc.col, best_pos.ori);
        return best_pos;
    }

    for(int o = 0; o < NUM_ORIENTATIONS; o++){
        for(int c = 0; c < tg->cols; c++){
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
            next_pos = dfs_dp(solver_tg, param, depth+1, dp);

            if(next_pos.typ < best_pos.typ){
                best_pos.typ = next_pos.typ;
                best_pos.loc.col = col_position;
                best_pos.ori = orientation;
            }
        }
    }
    dp[h] = store_best(best_pos);
    // printf("hash result: %d: %s\n", h, dp[h]);
    delete_skyline(skyline);
    // fclose(f);
    return best_pos;
}

tetris_block dfs_solver_dp(tetris_game* tg, parameters param, char ** dp){
    struct timeval time; 
    gettimeofday(&time,NULL);
    // suseconds_t start = time.tv_sec * 1000000 + time.tv_usec;
    tetris_game *solver_tg = tg_create(tg->rows, tg->cols);
    // before we solve, turn off rand()
    tg->use_random = 0;
    tg_copy(solver_tg, tg);
    tetris_block result;
    result = dfs_dp(solver_tg, param, 0, dp);
    tg_delete(solver_tg);
    tg->use_random = 1;
    gettimeofday(&time,NULL);
    // suseconds_t end = time.tv_sec * 1000000 + time.tv_usec;
//    printf("Total: %f\n", (float)(end - start) / 1000000.0);
 //   fflush(stdout);
    return result;
}