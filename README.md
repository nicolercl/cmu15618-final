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

## MILESTONE
According to the schedule, our milestone goal is to implement or find a tetris game and implement single-threaded solvers then parallelize them. Currently, we have completed the tasks according to the plan. We found a tetris game implemented in C on github and used its code for the gameplay. First, we implemented single-threaded BFS and DFS solvers that calculate 4 steps ahead for the best orientation and position to place a tetris block, and incorporated the solvers into the tetris game. Next, we parallelized both solvers using OpenMP on the GHC machines. For the DFS version, we parallelize the first layer nodes of the DFS search space. Initially, this produces sub-linear speedup with regard to the number of threads. However, we found out that the bottleneck was the C random function, because it requires mutual exclusion according to the source code, as shown in the screenshot below. 

Therefore, we removed it when running the solver. This produced linear speedup up to 40 threads (on the psc machine), which is the number of first layer nodes. The graphs for search time and speedup with the increase of threads is shown below. However, it was difficult to parallelize more threads using the current method because the number of first layer nodes is limited to 40, so we decided to use Cilk to further parallelize our DFS approach.

For the BFS approach, we created separate queues for the different threads and distributed the first layer nodes to different queues to run BFS. However, this approach suffers from memory problems because we need to store all possible states for BFS.
We believe we will be able to reach the goals described in the original proposal, but we might alter some of them. Instead of implementing multi-piece drop to increase the complexity of the problem, we want to further speedup our solution. For example, to get greater speedup with more threads, we are going to implement another parallelized DFS version with Cilk. One of the reasons is to facilitate using more threads (up to 128), as mentioned above. Another reason is that there are actually a lot of redundant states being computed by the naive DFS where we expand and compute every orientation and position pair of a block. For example, for the straight tetriminos, there are actually only 2 unique states out of the 4 orientations, vertical and horizontal, and there is only 1 unique state for the square tetriminos, but we are calculating them as 4 different states. If we prune these duplicate states using our current implementation, it would create workload imbalance between threads. Therefore, we decided to switch to Cilk, where the work-stealing would alleviate the imbalance automatically.
Currently, our plan for the demo is to show our solver playing the game with GUI. We would also show performance speedup graphs of our different algorithms.
