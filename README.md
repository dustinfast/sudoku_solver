#SudokuSolverAI
Demonstrates an "artificially intelligent" recursive backtracking algorithm with constraint propagation by solving any valid Sudoku puzzle in an average time of less than .1 seconds.

##Usage
From your terminal:
1. Compile with "g++ sud_solve.cpp -o sudoku.exe" (or equivalent).
2. Run with ./sudoku.exe (or simply "sudoku" in windows) .
3. Eenter sudoku puzzle name as prompted -
   Ex: sud_hard.txt
   Ex: more_puzz/extremelydifficult.txt

Note: Custom puzzles may be added - see existing puzzles for necessary format.

##Algorithm
A Most-Constrained-Value heuristic was used and the algorithm operates by traversing the search space (as a tree) in a depth-first and post-order manner. Generating child nodes by giving precedence to the variable (i.e. Sudoku cell) with the greatest Most-Constrained-Value and backtracking when any variable is found to have zero remaining unconstrained values, the algorithm continues until a goal state (i.e. a correctly solved Sudoku puzzle) is reached or the search space is exhausted (i.e. no solution exists.)
The processing of a node in this tree represents the assignment of a valid value to a previously unpopulated variable. When this occurs, that value is propagated as a constraint to every adjacent variable (adjacency is defined here as occupying the same row, column, or box of a Sudoku grid). If, during this process, an unpopulated variable is found to have 8 constraints on it, it is populated with it’s last (i.e. it’s 9th) remaining value if doing so would not violate any existing constraints. If it would, the algorithm backtracks.