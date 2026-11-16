/*
 * Sudoku Solver using Backtracking
 * Language: C
 *
 * Idea: find an empty cell (0), try digits 1-9, place a digit only if it is
 * safe (not in same row, column or 3x3 box), then recurse. If no digit works,
 * undo (backtrack) the previous choice and try the next digit.
 */

#include <stdio.h>

#define N 9

/* 0 means the cell is empty */
int board[N][N] = {
    {5, 3, 0, 0, 7, 0, 0, 0, 0},
    {6, 0, 0, 1, 9, 5, 0, 0, 0},
    {0, 9, 8, 0, 0, 0, 0, 6, 0},
    {8, 0, 0, 0, 6, 0, 0, 0, 3},
    {4, 0, 0, 8, 0, 3, 0, 0, 1},
    {7, 0, 0, 0, 2, 0, 0, 0, 6},
    {0, 6, 0, 0, 0, 0, 2, 8, 0},
    {0, 0, 0, 4, 1, 9, 0, 0, 5},
    {0, 0, 0, 0, 8, 0, 0, 7, 9}
};

/* Print the board in a readable grid */
void printBoard(void)
{
    for (int i = 0; i < N; i++) {
        if (i % 3 == 0 && i != 0)
            printf("------+-------+------\n");
        for (int j = 0; j < N; j++) {
            if (j % 3 == 0 && j != 0)
                printf("| ");
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }
}

/* Return 1 if placing num at (row, col) breaks no Sudoku rule, else 0 */
int isSafe(int row, int col, int num)
{
    /* Check the row and the column */
    for (int i = 0; i < N; i++) {
        if (board[row][i] == num) return 0;
        if (board[i][col] == num) return 0;
    }

    /* Check the 3x3 box: find its top-left corner first */
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[startRow + i][startCol + j] == num)
                return 0;

    return 1;
}

/* Find the first empty cell. Stores its position and returns 1,
   or returns 0 if the board is full. */
int findEmpty(int *row, int *col)
{
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (board[i][j] == 0) {
                *row = i;
                *col = j;
                return 1;
            }
        }
    }
    return 0;
}

/* Backtracking solver. Returns 1 if solved, 0 if no solution. */
int solve(void)
{
    int row, col;

    /* Base case: no empty cell left => puzzle solved */
    if (!findEmpty(&row, &col))
        return 1;

    for (int num = 1; num <= 9; num++) {
        if (isSafe(row, col, num)) {
            board[row][col] = num;      /* choose */

            if (solve())                /* explore next empty cell */
                return 1;

            board[row][col] = 0;        /* undo choice = backtrack */
        }
    }
    return 0;   /* no digit fits here, force previous call to backtrack */
}

int main(void)
{
    printf("Sudoku puzzle:\n");
    printBoard();

    if (solve()) {
        printf("\nSolved Sudoku:\n");
        printBoard();
    } else {
        printf("\nNo solution exists.\n");
    }
    return 0;
}
