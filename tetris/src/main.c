/***************************************************************************//**

  @file         main.c

  @author       Stephen Brennan

  @date         Created Wednesday, 10 June 2015

  @brief        Main program for tetris.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <ncurses.h>
#include <string.h>
#include <omp.h>

#if WITH_SDL
# include <SDL/SDL.h>
# include <SDL/SDL_mixer.h>
#endif

#include "tetris.h"
#include "dfs_solver.h"
#include "bfs.h"
#include "util.h"
#include "parameters.h"

/*
  2 columns per cell makes the game much nicer.
 */
#define COLS_PER_CELL 2
#define MAXIMUM_MOVES 128
#define USE_SOLVER 1
#define GUI 0
#define DFS 1
#define BFS 0
#define TIME_LIMIT 20 // 60 seconds
#define DELAY 1
#define NUM_OF_THREADS 8
#define ROW 22
#define COL 10

// genetic algorithm params
#define REPEAT 2

/*
  Macro to print a cell of a specific type to a window.
 */
#define ADD_BLOCK(w,x) waddch((w),' '|A_REVERSE|COLOR_PAIR(x));     \
                       waddch((w),' '|A_REVERSE|COLOR_PAIR(x))
#define ADD_EMPTY(w) waddch((w), ' '); waddch((w), ' ')

parameters solver_params = {{1.0, 5.5, 0.5}};

void get_moves(tetris_block falling, tetris_block result, int* moves){
    int move, step, ptr = 0;
    
    // move for orientations
    move = (falling.ori < result.ori) ? TM_CLOCK: TM_COUNTER;
    step = (falling.ori < result.ori) ? 1 : -1;
    for(int i = falling.ori; i != result.ori; i += step){
        moves[ptr++] = move;
    }

    // move for cols 
    move = (falling.loc.col < result.loc.col) ? TM_RIGHT : TM_LEFT;
    step = (falling.loc.col < result.loc.col) ? 1 : -1;
    for(int i = falling.loc.col; i != result.loc.col; i += step){
        moves[ptr++] = move;
    }
    moves[ptr++] = TM_DROP;
    moves[ptr] = TM_NONE;
}

/*
  Print the tetris board onto the ncurses window.
 */
void display_board(WINDOW *w, tetris_game *obj)
{
  int i, j;
  box(w, 0, 0);
  for (i = 0; i < obj->rows; i++) {
    wmove(w, 1 + i, 1);
    for (j = 0; j < obj->cols; j++) {
      if (TC_IS_FILLED(tg_get(obj, i, j))) {
        ADD_BLOCK(w,tg_get(obj, i, j));
      } else {
        ADD_EMPTY(w);
      }
    }
  }
  wnoutrefresh(w);
}

/*
  Display a tetris piece in a dedicated window.
*/
void display_piece(WINDOW *w, tetris_block block)
{
  int b;
  tetris_location c;
  wclear(w);
  box(w, 0, 0);
  if (block.typ == -1) {
    wnoutrefresh(w);
    return;
  }
  for (b = 0; b < TETRIS; b++) {
    c = TETROMINOS[block.typ][block.ori][b];
    wmove(w, c.row + 1, c.col * COLS_PER_CELL + 1);
    ADD_BLOCK(w, TYPE_TO_CELL(block.typ));
  }
  wnoutrefresh(w);
}

/*
  Display score information in a dedicated window.
 */
void display_score(WINDOW *w, tetris_game *tg)
{
  wclear(w);
  box(w, 0, 0);
  wprintw(w, "Score\n%d\n", tg->points);
  wprintw(w, "Level\n%d\n", tg->level);
  wprintw(w, "Lines\n%d\n", tg->lines_remaining);
  wnoutrefresh(w);
}

/*
  Boss mode!  Make it look like you're doing work.
 */
void boss_mode(void)
{
  clear();
#if WITH_SDL
  Mix_PauseMusic();
#endif
  printw("user@workstation-312:~/Documents/presentation $ ls -l\n"
         "total 528\n"
         "drwxr-xr-x 2 user users   4096 Jun  9 17:05 .\n"
         "drwxr-xr-x 4 user users   4096 Jun 10 09:52 ..\n"
         "-rw-r--r-- 1 user users  88583 Jun  9 14:13 figure1.png\n"
         "-rw-r--r-- 1 user users  65357 Jun  9 15:40 figure2.png\n"
         "-rw-r--r-- 1 user users   4469 Jun  9 16:17 presentation.aux\n"
         "-rw-r--r-- 1 user users  42858 Jun  9 16:17 presentation.log\n"
         "-rw-r--r-- 1 user users   2516 Jun  9 16:17 presentation.nav\n"
         "-rw-r--r-- 1 user users    183 Jun  9 16:17 presentation.out\n"
         "-rw-r--r-- 1 user users 349607 Jun  9 16:17 presentation.pdf\n"
         "-rw-r--r-- 1 user users      0 Jun  9 16:17 presentation.snm\n"
         "-rw-r--r-- 1 user users   9284 Jun  9 17:05 presentation.tex\n"
         "-rw-r--r-- 1 user users    229 Jun  9 16:17 presentation.toc\n"
         "\n"
         "user@workstation-312:~/Documents/presentation $ ");
  echo();
  timeout(-1);
  while (getch() != KEY_F(1));
  timeout(0);
  noecho();
  clear();
#if WITH_SDL
  Mix_ResumeMusic();
#endif
}

/*
  Save and exit the game.
 */
void save(tetris_game *game, WINDOW *w)
{
  FILE *f;

  wclear(w);
  box(w, 0, 0); // return the border
  wmove(w, 1, 1);
  wprintw(w, "Save and exit? [Y/n] ");
  wrefresh(w);
  timeout(-1);
  if (getch() == 'n') {
    timeout(0);
    return;
  }
  f = fopen("tetris.save", "w");
  tg_save(game, f);
  fclose(f);
  tg_delete(game);
  endwin();
  printf("Game saved to \"tetris.save\".\n");
  printf("Resume by passing the filename as an argument to this program.\n");
  exit(EXIT_SUCCESS);
}

/*
  Do the NCURSES initialization steps for color blocks.
 */
void init_colors(void)
{
  start_color();
  //init_color(COLOR_ORANGE, 1000, 647, 0);
  init_pair(TC_CELLI, COLOR_CYAN, COLOR_BLACK);
  init_pair(TC_CELLJ, COLOR_BLUE, COLOR_BLACK);
  init_pair(TC_CELLL, COLOR_WHITE, COLOR_BLACK);
  init_pair(TC_CELLO, COLOR_YELLOW, COLOR_BLACK);
  init_pair(TC_CELLS, COLOR_GREEN, COLOR_BLACK);
  init_pair(TC_CELLT, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(TC_CELLZ, COLOR_RED, COLOR_BLACK);
}

/*
  Main tetris game!
 */
int run_game(parameters param)
{
  tetris_game *tg;
  tetris_move move = TM_NONE;
  bool running = true;
  WINDOW *board, *next[NEXT_N], *hold, *score;

  tg = tg_create(ROW, COL);

  // NCURSES initialization:
  if(GUI){
      initscr();             // initialize curses
      cbreak();              // pass key presses to program, but not signals
      noecho();              // don't echo key presses to screen
      keypad(stdscr, TRUE);  // allow arrow keys
      timeout(0);            // no blocking on getch()
      curs_set(0);           // set the cursor to invisible
      init_colors();         // setup tetris colors
      // Create windows for each section of the interface.
      board = newwin(tg->rows + 2, 2 * tg->cols + 2, 0, 0);
      for (int i = 0; i < NEXT_N; i++)
        next[i]  = newwin(6, 10, i * 6, 2 * (tg->cols + 1) + 1 + 16);
      hold  = newwin(6, 10, 7, 2 * (tg->cols + 1) + 1);
      score = newwin(6, 10, 14, 2 * (tg->cols + 1 ) + 1);
  }

  // solver init
  int moves[MAXIMUM_MOVES];
  int action_ptr = 0;
  moves[0] = TM_NONE;

  time_t start;
  start = time(NULL);
  // Game loop
  while (running && time(NULL) - start < TIME_LIMIT) {
    running = tg_tick(tg, move);

    if(GUI){
        display_board(board, tg);
        for (int i = 0; i < NEXT_N; i++)
          display_piece(next[i], tg->next[i]);
        display_piece(hold, tg->stored);
        display_score(score, tg);
        doupdate();
    }

    sleep_milli(DELAY);


    if(USE_SOLVER){
        if(moves[action_ptr] == TM_NONE){
            tetris_block result;
            if (DFS) {
                result = dfs_solver(tg, param);
            } else if (BFS) {
              int height, hole, bumpiness;
              result = solve(tg, height, hole, bumpiness);
            }
            get_moves(tg->falling, result, moves);
            action_ptr = 0;
        }
        move = moves[action_ptr++];

    } else {
        switch (getch()) {
        case KEY_LEFT:
          move = TM_LEFT;
          break;
        case KEY_RIGHT:
          move = TM_RIGHT;
          break;
        case KEY_UP:
          move = TM_CLOCK;
          break;
        case KEY_DOWN:
          move = TM_DROP;
          break;
        case 'q':
          running = false;
          move = TM_NONE;
          break;
        case 'p':
          if(GUI){
              wclear(board);
              box(board, 0, 0);
              wmove(board, tg->rows/2, (tg->cols*COLS_PER_CELL-6)/2);
              wprintw(board, "PAUSED");
              wrefresh(board);
              timeout(-1);
              getch();
              timeout(0);
              move = TM_NONE;
          }
          break;
        case 'b':
          boss_mode();
          move = TM_NONE;
          break;
        case 's':
          save(tg, board);
          move = TM_NONE;
          break;
        case ' ':
          move = TM_HOLD;
          break;
        default:
          move = TM_NONE;
        }
    }

  }

  // Deinitialize NCurses
  if(GUI){
      wclear(stdscr);
      endwin();
  }

  // Output ending message.
  printf("Height: %f, Hole: %f, Bumpiness: %f\n", param.weights[0], param.weights[1], param.weights[2]);
  printf("You finished with %d points on level %d.\n", tg->points, tg->level);

  // Deinitialize Tetris
  tg_delete(tg);
  return tg->points;
}

typedef struct {
    parameters param;
    int score;
} candidate;

void random_params(parameters *param){
    for(int i = 0; i < sizeof(param->weights); i++)
        param->weights[i] = (float)(rand() % 100) / 100.0;
}

int compare (const void * num1, const void * num2) {
    if(((candidate*)num1)->score > ((candidate*)num2)->score)
        return -1;
    else
        return 1;
}

const parameters generate_child(parameters *parent1, parameters *parent2){
    parameters child;
    for(int i = 0; i < sizeof(child.weights); i++){
        if(rand() % 2 == 0)
            child.weights[i] = parent1->weights[i];
        else
            child.weights[i] = parent2->weights[i];

        // mutation
        if(rand() % 5 == 0)
            child.weights[i] += (float)(rand() % 20) / 100; 
    }

    return child;
}
int genetic_algorithm(){
    candidate pool[10]; 
    // init
    for(int i = 0; i < 10; i++){
        random_params(&(pool[i].param));
    }
    int count = 0; 
    while(1){
        for(int i = 0; i < 10; i++){
            pool[i].score = 0;
            for(int j = 0; j < REPEAT; j++)
                pool[i].score += run_game(pool[i].param);
        }


        qsort(pool, 10, sizeof(candidate), compare);

        parameters *best = &(pool->param);
        printf("Generation %d Best Score %d height %f hole %f bumpiness %f\n", count, pool->score / REPEAT, best->weights[0], best->weights[1], best->weights[2]);

        // generate children
        for(int i = 0; i < 3; i++){
            int j = i;
            while(j == i){
                j = rand() % 3;
            }
            const parameters *p1 = &(pool[i].param), *p2 = &(pool[j].param);
            pool[i + 3].param = generate_child(p1, p2);
        }

        for(int i = 3 + 3; i < 10; i++){
            random_params(&(pool[i].param));
        }

        count ++;
    }


} 

int main(int argc, char **argv) {
  struct timeval time; 
  gettimeofday(&time,NULL);
  srand((time.tv_sec * 1000) + (time.tv_usec / 1000));
  omp_set_num_threads(NUM_OF_THREADS);
  //run_game(solver_params);
  genetic_algorithm();
}

