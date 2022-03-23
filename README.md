# 15618 Final Project Proposal
#### Title: Parallel Multi-player Tetris solver (Yen Li Laih, Wei Wei Lin)

### SUMMARY

We are going to implement a parallelized Tetris solver for a tetris game for multiple players using various algorithms and perform a detailed analysis of the speedup and performance of the different algorithms. In order to increase difficulty, multiple pieces can be dropped simultaneously. 

### BACKGROUND

Tetris is a puzzle game that involves randomly generated pieces of various shapes that descend onto the board. The player has to complete horizontal lines with the pieces so that the lines would disappear. If the stacked pieces reach the top of the board, the player loses. There are many search algorithms such as the minimax algorithm, and genetic algorithms that can be applied to solve this game. These search algorithms would benefit from parallel processing and speedup the search for the optimal solution of a single step. Also to increase difficulty, we would allow multiple pieces to be dropped simultaneously, this would largely increase synchronization overhead between different threads for all algorithms. 

### THE CHALLENGE

The workload of Tetris is mostly computation-intensive since we would need to search through a huge search space to find the optimal solution under a time constraint. Additionally, in order to increase the search difficulty, we now allow multiple pieces to be dropped simultaneously. Under these circumstances, we would need to synchronize between different threads since the board state now is changed by multiple threads and might interfere with the searching process. We would need to understand the characteristics of different algorithms, how they perform under parallelization, and also how to synchronize the searching process.

### RESOURCES

We plan to implement various solvers from scratch in C/C++, referencing online resources and research on different algorithms. We will also implement a tetris game to customize the workload of the game, such as larger/3D boards, or more pieces if necessary. We will parallelize our solver with OpenMP and perform experiments on the GHC machines.

### GOALS AND DELIVERABLES

- Goals we plan to achieve (100%): single-player game with significant speedup and speedup performance analysis of two algorithms.
- Goals we hope to achieve (125%): two/multiplayer game with implementation and speedup performance analysis of two algorithms.
- Goals we at least hope to achieve (75%): single-player game with single algorithm implementation and speedup performance analysis.
- Demo to show at the poster session: We hope to add a UI to the game to visualize the speedup between the benchmark and parallelized versions with different algorithms. We will also show speedup graphs for different numbers of threads for different algorithms. If we finish the multiplayer version, we can also play the game with the AI.
- We hope to learn about the effect of parallelization on different search algorithms and analyze how to reduce communication overhead between threads for the multiplayer version.

### PLATFORM CHOICE

The C/C++ languages are suitable for the game solver implementation because we can use OpenMP to parallelize our implementation easily. We also had prior experience with the language through the previous labs.

### SCHEDULE

| Week | Plan |
| -----| ----- |
| Week1 (3/21 - 3/28) | Finish proposal, finish a small Tetris game for benchmarking our solver. |
| Week2 (3/28 - 4/4) | Survey various algorithms and implement our single threads solvers. |
| Week3 (4/4 - 4/11) | Parallelize our solvers and compare their results, analyze why different algorithms scale differently. |
| Week4 (4/11 - 4/18) | Parallelize our solvers and compare their results, analyze why different algorithms scale differently. |
| Week5 (4/18 - 4/25) | Allow multiple-piece dropping, modify solver to support this mode (have to handle a lot of synchronization) |
| Week6 (4/25 - 4/29) | Work on the final report |
