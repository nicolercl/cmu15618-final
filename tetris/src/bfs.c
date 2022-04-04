#include "bfs.h"
#include "queue.h"
#include <string.h>

#define QUEUE_SIZE 100

/**
 * Initialize state and return pointer to state.
 * Update board and level while keeping the first move orientation and offset.
 */
state* init_state(char *board, int level, int board_size, state *oldstate) {
    state *s = (state*) malloc(sizeof(state));
    s->board = (char*) malloc(board_size);
    strncpy(s->board, board, board_size);
    s->level = level;
    if (oldstate) {
        s->ori = oldstate->ori;
        s->offset = oldstate->offset;
    }
    return s;
}
/**
 * Store the orientation and offset of the first move
 */
void save_first_move(state* s, int orientation, int offset) {
    s->ori = orientation;
    s->offset = offset;
}

/**
 * Apply state to game object.
 */ 
tetris_game apply_state(tetris_game game, state *state, int board_size) {
    tetris_game copy = game;
    // copy.board = malloc(board_size);
    // fprintf(stderr, "board_size %d\n", state->board[0]);
    strncpy(copy.board, state->board, board_size);
    copy.level = state->level;
    return copy;
}

/**
 * Calculate results of move and return state.
 */
state* update_state(tetris_game game, state *oldstate) {
    int removed_lines = tg_check_lines(&game);
    tg_adjust_score(&game, removed_lines);
    return init_state(game.board, game.level, game.cols * game.rows, oldstate);
}

/**
 * solve.
 * @param[in] game
 * @param[out] orientation
 * @param[out] offset
 */
void solve(tetris_game game, int* orientation, int* offset) {
    int board_size = game.rows * game.cols;

    queue* q = create_queue(QUEUE_SIZE);
    state* start = init_state(game.board, game.level, board_size, NULL);
    push(q, start);

    int steps = 0;
    int cols = game.cols;
    int level = game.level;

    while(steps < NEXT_N) {
        // all possible states of the previous step
        for (int q_size = size(q); q_size > 0; q_size--) {
            state *cur_s = front(q);
            pop(q);
            // make a copy for rotate with applied state
            tetris_game rotate = apply_state(game, cur_s, board_size);

            // check if game over
            if (tg_game_over(&rotate)) {
                *orientation = cur_s->ori;
                *offset = cur_s->offset;
                return;
            }

            // if last step, stop expanding
            if (steps == NEXT_N - 1 && rotate.level < level) {
                *orientation = cur_s->ori;
                *offset = cur_s->offset;
                continue;
            }

            for (int i = 0; i < 4; i++) { // orientation
                // rotate clock-wise
                tg_handle_move(&rotate, TM_CLOCK);

                // make a copy for shift
                tetris_game shift = rotate;
                for (int left = 0; left < cols/2; left++) { // left shift
                    tg_handle_move(&shift, TM_LEFT);
                    tetris_game obj = shift;                // make a copy for drop
                    tg_handle_move(&obj, TM_DROP);
                    
                    state * new_s = update_state(obj, cur_s);
                    if (steps == 0) {
                        save_first_move(new_s, i, -left);
                    }
                    push(q, new_s);
                }

                // reset shift copy
                shift = rotate;
                for (int right = 1; right < cols/2; right++) { // right shift
                    tg_handle_move(&shift, TM_RIGHT);
                    tetris_game obj = shift;                   // make a copy for drop
                    tg_handle_move(&obj, TM_DROP);

                    state * new_s = update_state(obj, cur_s);
                    if (steps == 0) {
                        save_first_move(new_s, i, right);
                    }
                    push(q, new_s);
                }
            }
        }
        steps++;
        // update game falling piece here
        tg_new_falling(&game);
    }
}
