# Partridge Puzzle Solver

**This project is no longer under active development.**

## About

The partridge puzzle asks you to pack a square of side `n(n+1)/2` with one `1x1` square, two `2x2` squares, three `3x3` squares, and so on up to `n` squares of size `nxn`, using every piece and leaving no gaps. The name comes from the growing quantities, one of one size, two of the next, similar to the counting pattern in "The Twelve Days of Christmas". This project solves the puzzle for `n = 9` (a `45x45` board), using backtracking with an aggressive board representation to keep the search fast.

This project was inspired by [this video](https://www.youtube.com/watch?v=eqyuQZHfNPQ)

## Implementation

- Rather than storing the whole 2d board, the solver tracks the current fill line as a doubly linked list of nodes, one node per flat segment along the top of the placed pieces, similar to a skyline. Each node stores its x position, height, and width.
- The board fills from the bottom up. At each step the solver picks the lowest, narrowest open gap in the skyline and tries placing a piece there.
- Narrower gaps are prioritized over wider ones. Small pieces are the most plentiful, so they're best saved for filling in tight leftover slivers later, while bigger pieces need to be placed while there's still open room for them.
- Before trying a size, the solver checks whether any pieces of that size are still available, so it never recurses down a branch it already knows is dead.
- Placing a piece is done by inserting one or two new nodes into the linked list (merging with a neighboring node instead, if that neighbor is already at the same height, so the list never grows more than it needs to). The two new nodes live on the call stack rather than the heap, so backtracking after a failed recursive call is just restoring the handful of pointers that were changed, no separate undo list needed.
- Once every square is placed, the board is rendered to the console as ascii art.

## Usage

1. Build with any c compiler, eg `gcc partridge.c -o partridge`.
2. Run the executable. It solves for a fixed board size set in the source (`NUM_CUBES`), no input file needed.
3. The first solution found is printed as an ascii board, along with the time taken to solve it.
