#include "bfs.h"
#include "queue.h"
#include <string.h>

#define QUEUE_SIZE 2000000 //1679616

/*
  Print a game board
 */
void print(char * board, int rows, int cols) {
  int i, j;
  for (i = 0; i < rows; i++) {
    fputs("|", stderr);
    for (j = 0; j < cols; j++) {
      if (TC_IS_EMPTY(board[i * cols + j])) {
        fputs(TC_EMPTY_STR, stderr);
      } else {
        fputs(TC_BLOCK_STR, stderr);
      }
    }
    fputs("|", stderr);
    fputc('\n', stderr);
  }
}
/**
 * Get level of board.
 */
int get_level(tetris_game *obj, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (!TC_IS_EMPTY(tg_get(obj, i, j))) {
                return rows - i;
            }
        }
    }
    return 0;
}

/**
 * Initialize state and return pointer to state.
 * Update board and level while keeping the first move orientation and offset.
 */
state* init_state(char *board, int rows, int cols, state *old_state, int level) {
    state *s = (state*) malloc(sizeof(state));
    s->board = (char*) malloc(rows * cols);
    memcpy(s->board, board, rows * cols);
    s->level = level;
    if (old_state) {
        s->ori = old_state->ori;
        s->col = old_state->col;
    }
    return s;
}
/**
 * Store the orientation and offset of the first move
 */
void save_first_move(state* s, int orientation, int col) {
    s->ori = orientation;
    s->col = col;
}
/**
 * Deep copy game object
 */
tetris_game* deep_copy(tetris_game* src) {
    tetris_game *dst = malloc(sizeof(tetris_game));
    memcpy(dst, (void *)src, sizeof(tetris_game));
    dst->board = (char*) malloc(src->rows * src->cols);
    memcpy(dst->board, src->board, src->rows * src->cols);
    return dst;
}

/**
 * Apply state to game object and return new copy.
 */ 
tetris_game* apply_state(tetris_game* game, state *state, int board_size) {
    tetris_game* copy = deep_copy(game);
    memcpy(copy->board, state->board, board_size);
    return copy;
}

/**
 * Calculate results of move and return state.
 */
state* update_state(tetris_game* game, state *oldstate) {
    tetris_game* copy = deep_copy(game);
    int level = get_level(game, game->rows, game->cols);
    int removeLines = tg_check_lines(game);
    return init_state(copy->board, game->cols, game->rows, oldstate, level - removeLines);
}

/**
 * solve.
 * @param[in] game
 * @param[out] orientation
 * @param[out] offset
 */
tetris_block solve(tetris_game* tg) {
    tetris_game* game = deep_copy(tg);

    int board_size = game->rows * game->cols;
    tetris_block falling = game->falling;

    queue* q = create_queue(QUEUE_SIZE);
    state* start = init_state(game->board, game->rows, game->cols, NULL, get_level(game, game->rows, game->cols));
    push(q, start);

    // fprintf(stderr, "solve\n");
    // print(game->board, game->rows, game->cols);
    // fprintf(stderr, "==========\n");

    int steps = 0;
    int cols = game->cols;
    int level = game->rows;

    while(steps <= NEXT_N-3) {
        // all possible states of the previous step
        // printf("Step %d queue size: %d\n", steps, size(q));
        for (int q_size = size(q); q_size > 0; q_size--) {
            state *cur_s = front(q);
            pop(q);
            // make a copy for rotate with applied state
            tetris_game* rotate = apply_state(game, cur_s, board_size);

            // check if game over
            if (tg_game_over(rotate)) {
                falling.ori = cur_s->ori;
                falling.loc.col = cur_s->col;
                return falling;
            }

            // if last step, stop expanding
            if (steps == NEXT_N-3 && cur_s->level <= level) {
                falling.ori = cur_s->ori;
                falling.loc.col = cur_s->col;
                level = cur_s->level;
                // fprintf(stderr, "update better\n");
                // print(cur_s->board, game->rows, game->cols);
                // fprintf(stderr, "==========\n");
                continue;
            }

            for (int i = 1; i <= 4; i++) { // orientation
                // rotate clock-wise
                tg_handle_move(rotate, TM_CLOCK);

                // make a copy for shift
                tetris_game* shift = deep_copy(rotate);
                for (int left = 1; left < cols/2; left++) { // left shift
                    tg_handle_move(shift, TM_LEFT);
                    tetris_game* obj = deep_copy(shift);     // make a copy for drop
                    int col = obj->falling.loc.col;
                    tg_handle_move(obj, TM_DROP);
                    
                    state * new_s = update_state(obj, cur_s);
                    if (steps == 0) {
                        save_first_move(new_s, i % 4, col);
                    }
                    push(q, new_s);
                }

                // reset shift copy back to 1 col to the left of origin
                for (int right = 1; right < cols/2 - 1; right++)
                    tg_handle_move(shift, TM_RIGHT);

                for (int right = 0; right < cols/2; right++) { // right shift
                    tg_handle_move(shift, TM_RIGHT);
                    tetris_game *obj = deep_copy(shift);           // make a copy for drop
                    int col = obj->falling.loc.col;
                    tg_handle_move(obj, TM_DROP);

                    state * new_s = update_state(obj, cur_s);
                    if (steps == 0) {
                        save_first_move(new_s, i, col);
                    }
                    push(q, new_s);
                }
            }
        }
        steps++;
        // update game falling piece here
        tg_remove(game, game->falling);
        tg_new_falling(game);
        tg_put(game, game->falling);
    }
    // fprintf(stderr, "loc: %d, ori: %d\n", falling.loc.col, falling.ori);
    // fprintf(stderr, "========s===============\n");
    return falling;
}
