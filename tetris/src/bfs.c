#include "bfs.h"
#include "queue.h"
#include <string.h>

#define QUEUE_SIZE 2000000 //1679616

void delete_state(state *state) {
    free(state->board);
    free(state);
}

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

state* bfs(queue* q, tetris_game *game) {
    state *best;
    int board_size = game->rows * game->cols;
    int q_size = size(q);
    int level = game->rows;
    int cols = game->cols;

    for (int i = 0; i < q_size; i++){
        state *cur_s = front(q);
        pop(q);

        // make a copy for rotate with applied state
        tetris_game* rotate = apply_state(game, cur_s, board_size);

        // if last step, stop expanding
        if (cur_s->level <= level) {
            level = cur_s->level;
            best = init_state(cur_s->board, rotate->rows, rotate->cols, cur_s, level);
        }

        for (int ori = 1; ori <= 4; ori++) { // orientation
            // rotate clock-wise
            tg_handle_move(rotate, TM_CLOCK);

            // make a copy for shift
            tetris_game* shift = deep_copy(rotate);

            for (int c = 0; c < cols; c++) { // right shift
                tg_move(shift, -shift->falling.loc.col);
                tg_move(shift, c);
                tetris_game *obj = deep_copy(shift);           // make a copy for drop
                tg_handle_move(obj, TM_DROP);

                state * new_s = update_state(obj, cur_s);
                push(q, new_s);
                tg_delete(obj);
            }
            tg_delete(shift);
        }
        delete_state(cur_s);
        tg_delete(rotate);
    }
    return best;
}

/**
 * solve.
 * @param[in] game
 * @param[out] orientation
 * @param[out] offset
 */
tetris_block solve(tetris_game* tg, int height, int hole, int bumpiness, int n_threads) {
    tetris_game* game = deep_copy(tg);

    int board_size = game->rows * game->cols;
    queue* queues[n_threads];
    state* states[n_threads];
    tetris_game* games[n_threads];
    tetris_block falling = game->falling;

    state* start = init_state(game->board, game->rows, game->cols, NULL, get_level(game, game->rows, game->cols));

    for (int i = 0; i < n_threads; i++) {
        queues[i] = create_queue(QUEUE_SIZE / n_threads);
    }

    int steps = 0;

    while(steps <= NEXT_N-2) {
        // all possible states of the previous step
        if (steps == 0) {

            // make a copy for rotate with applied state
            tetris_game* rotate = apply_state(game, start, board_size);

            for (int i = 1; i <= 4; i++) { // orientation
                // rotate clock-wise
                tg_handle_move(rotate, TM_CLOCK);

                // make a copy for shift
                tetris_game* shift = deep_copy(rotate);

                for (int c = 0; c < game->cols; c++) { // right shift
                    tg_move(shift, -shift->falling.loc.col);
                    tg_move(shift, c);
                    tetris_game *obj = deep_copy(shift);           // make a copy for drop
                    int col = obj->falling.loc.col;
                    tg_handle_move(obj, TM_DROP);

                    state * new_s = update_state(obj, start);

                    save_first_move(new_s, i % 4, col);
                    push(queues[c % n_threads], new_s);
                    tg_delete(obj);
                }
                tg_delete(shift);
            }
            tg_delete(rotate);
        } 
        else {
            for (int i = 0; i < n_threads; i++)
                games[i] = deep_copy(game);

            #pragma omp parallel for default(shared) schedule(dynamic)
            for (int i = 0; i < n_threads; i++) {
                states[i] = bfs(queues[i], games[i]);
                tg_delete(games[i]);
            }
        }
        steps++;
        // update game falling piece here
        tg_remove(game, game->falling);
        tg_new_falling(game);
        tg_put(game, game->falling);
    }

    int level = game->rows;
    for (int i = 0; i < n_threads; i++) {
        if (states[i]->level < level) {
            falling.ori = states[i]->ori;
            falling.loc.col = states[i]->col;
            level = states[i]->level;
        }
    }
    for (int i = 0; i < n_threads; i++) {
        delete_queue(queues[i]);
        delete_state(states[i]);
    }
    tg_delete(game);
    return falling;
}