/*
 * Sudoku Game in C (terminal version)
 *
 * - Generates a new random puzzle every time (using backtracking)
 * - Every puzzle has exactly one solution
 * - You play by typing:  row column number     e.g.  3 5 7
 * - Commands: h = hint, c = check, s = solve (backtracking), n = new, q = quit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define N 9

static int solution[N][N];   /* the full correct grid            */
static int board[N][N];      /* what the player currently sees    */
static int given[N][N];      /* 1 = cell was part of the puzzle   */
static int hintsUsed = 0;

/* ---------- core backtracking helpers (same idea as the solver) ---------- */

/* 1 if num can be placed at (row, col) without breaking any rule */
static int isSafe(int g[N][N], int row, int col, int num)
{
    for (int i = 0; i < N; i++) {
        if (g[row][i] == num) return 0;
        if (g[i][col] == num) return 0;
    }
    int sr = row - row % 3, sc = col - col % 3;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (g[sr + i][sc + j] == num) return 0;
    return 1;
}

static int findEmpty(int g[N][N], int *row, int *col)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (g[i][j] == 0) { *row = i; *col = j; return 1; }
    return 0;
}

static void shuffle(int *a, int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = a[i]; a[i] = a[j]; a[j] = t;
    }
}

/* Plain backtracking solver: tries 1..9 in order */
static int solve(int g[N][N])
{
    int row, col;
    if (!findEmpty(g, &row, &col)) return 1;
    for (int num = 1; num <= 9; num++) {
        if (isSafe(g, row, col, num)) {
            g[row][col] = num;
            if (solve(g)) return 1;
            g[row][col] = 0;           /* backtrack */
        }
    }
    return 0;
}

/* Backtracking again, but digits are tried in random order.
   Used on an empty grid to create a random full solution. */
static int solveRandom(int g[N][N])
{
    int row, col;
    if (!findEmpty(g, &row, &col)) return 1;
    int nums[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    shuffle(nums, 9);
    for (int i = 0; i < 9; i++) {
        if (isSafe(g, row, col, nums[i])) {
            g[row][col] = nums[i];
            if (solveRandom(g)) return 1;
            g[row][col] = 0;
        }
    }
    return 0;
}

/* Counts solutions (stops at limit). Always picks the empty cell with the
   fewest options first, which keeps it fast. */
static int countSolutions(int g[N][N], int limit)
{
    int bestR = -1, bestC = -1, bestCnt = 10;
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (g[r][c] != 0) continue;
            int cnt = 0;
            for (int n = 1; n <= 9; n++)
                if (isSafe(g, r, c, n)) cnt++;
            if (cnt == 0) return 0;
            if (cnt < bestCnt) { bestCnt = cnt; bestR = r; bestC = c; }
        }
    }
    if (bestR < 0) return 1;           /* no empty cell: one full solution */

    int total = 0;
    for (int n = 1; n <= 9; n++) {
        if (isSafe(g, bestR, bestC, n)) {
            g[bestR][bestC] = n;
            total += countSolutions(g, limit);
            g[bestR][bestC] = 0;
            if (total >= limit) break;
        }
    }
    return total;
}

/* ---------- puzzle generation ---------- */

static void generate(int removeTarget)
{
    memset(solution, 0, sizeof solution);
    solveRandom(solution);
    memcpy(board, solution, sizeof board);

    int pos[N * N];
    for (int i = 0; i < N * N; i++) pos[i] = i;
    shuffle(pos, N * N);

    int removed = 0;
    for (int i = 0; i < N * N && removed < removeTarget; i++) {
        int r = pos[i] / N, c = pos[i] % N;
        int saved = board[r][c];
        board[r][c] = 0;
        int tmp[N][N];
        memcpy(tmp, board, sizeof tmp);
        if (countSolutions(tmp, 2) == 1) removed++;   /* still unique: keep removed */
        else board[r][c] = saved;                     /* not unique: put it back   */
    }

    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            given[r][c] = (board[r][c] != 0);
    hintsUsed = 0;
}

/* ---------- display ---------- */

/* Puzzle numbers show as  5   your numbers show as [5]   empty shows as . */
static void printBoard(void)
{
    printf("\n    ");
    for (int c = 0; c < N; c++) {
        printf(" %d ", c + 1);
        if (c % 3 == 2) printf(" ");
    }
    printf("\n");
    for (int r = 0; r < N; r++) {
        if (r % 3 == 0) printf("   +---------+---------+---------+\n");
        printf(" %d |", r + 1);
        for (int c = 0; c < N; c++) {
            if (board[r][c] == 0)      printf(" . ");
            else if (given[r][c])      printf(" %d ", board[r][c]);
            else                       printf("[%d]", board[r][c]);
            if (c % 3 == 2) printf("|");
        }
        printf("\n");
    }
    printf("   +---------+---------+---------+\n");
}

static void printHelp(void)
{
    printf("\nHow to play\n");
    printf("  row col num   place a number, e.g.  3 5 7  (row 3, column 5, number 7)\n");
    printf("  row col 0     erase one of your numbers, e.g.  3 5 0\n");
    printf("  h             hint: fills one cell for you\n");
    printf("  c             check: tells you how many of your numbers are wrong\n");
    printf("  s             solve: the backtracking solver finishes the puzzle\n");
    printf("  n             new game        q  quit        ?  show this help\n");
}

/* ---------- game logic ---------- */

static int countEmpty(void)
{
    int e = 0;
    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            if (board[r][c] == 0) e++;
    return e;
}

static int countWrong(void)
{
    int w = 0;
    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            if (board[r][c] != 0 && board[r][c] != solution[r][c]) w++;
    return w;
}

static int isWon(void)
{
    return memcmp(board, solution, sizeof board) == 0;
}

static void giveHint(void)
{
    int cand[N * N], k = 0;
    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            if (board[r][c] != solution[r][c]) cand[k++] = r * N + c;
    if (k == 0) return;
    int pick = cand[rand() % k];
    int r = pick / N, c = pick % N;
    board[r][c] = solution[r][c];
    hintsUsed++;
    printf("Hint: row %d, column %d is %d.\n", r + 1, c + 1, board[r][c]);
}

static int askYesNo(const char *question)
{
    char line[64];
    for (;;) {
        printf("%s (y/n): ", question);
        if (!fgets(line, sizeof line, stdin)) return 0;
        char ch = 0;
        for (char *p = line; *p; p++)
            if (!isspace((unsigned char)*p)) { ch = (char)tolower((unsigned char)*p); break; }
        if (ch == 'y') return 1;
        if (ch == 'n') return 0;
    }
}

static int chooseDifficulty(void)
{
    char line[64];
    for (;;) {
        printf("\nChoose difficulty:  1 = Easy   2 = Medium   3 = Hard : ");
        if (!fgets(line, sizeof line, stdin)) return -1;
        int d = atoi(line);
        if (d == 1) return 40;
        if (d == 2) return 48;
        if (d == 3) return 54;
        printf("Please type 1, 2 or 3.\n");
    }
}

/* Plays one puzzle. Returns 1 if the player wants another game, 0 to exit. */
static int playRound(void)
{
    int target = chooseDifficulty();
    if (target < 0) return 0;

    printf("\nMaking a puzzle...\n");
    generate(target);
    printHelp();

    char line[128];
    for (;;) {
        printBoard();
        printf("Empty cells left: %d\n", countEmpty());
        printf("\n> ");
        if (!fgets(line, sizeof line, stdin)) return 0;

        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;

        if (isalpha((unsigned char)*p) || *p == '?') {
            switch (tolower((unsigned char)*p)) {
            case 'q':
                printf("Thanks for playing!\n");
                return 0;
            case 'n':
                return 1;
            case '?':
                printHelp();
                break;
            case 'h':
                giveHint();
                if (isWon()) {
                    printBoard();
                    printf("\n*** The puzzle is complete! ***\n");
                    printf("Hints used: %d\n", hintsUsed);
                    return askYesNo("\nPlay another game?");
                }
                break;
            case 'c': {
                int w = countWrong();
                if (w == 0) printf("Everything you have entered so far is correct.\n");
                else        printf("%d of your numbers %s wrong.\n", w, w == 1 ? "is" : "are");
                break;
            }
            case 's': {
                int tmp[N][N];
                for (int r = 0; r < N; r++)
                    for (int c = 0; c < N; c++)
                        tmp[r][c] = given[r][c] ? board[r][c] : 0;
                solve(tmp);                       /* backtracking from the original puzzle */
                memcpy(board, tmp, sizeof board);
                printf("\nSolved by backtracking:\n");
                printBoard();
                return askYesNo("\nPlay another game?");
            }
            default:
                printf("Unknown command. Type ? for help.\n");
            }
        } else {
            int r, c, n;
            if (sscanf(line, "%d %d %d", &r, &c, &n) != 3 ||
                r < 1 || r > 9 || c < 1 || c > 9 || n < 0 || n > 9) {
                printf("Type three numbers: row (1-9), column (1-9), number (0-9). Example: 3 5 7\n");
                continue;
            }
            r--; c--;
            if (given[r][c]) {
                printf("That cell is part of the puzzle and can't be changed.\n");
                continue;
            }
            if (n == 0) {
                board[r][c] = 0;
                continue;
            }
            int old = board[r][c];
            board[r][c] = 0;
            if (!isSafe(board, r, c, n)) {
                board[r][c] = old;
                printf("%d is already in that row, column or 3x3 box.\n", n);
                continue;
            }
            board[r][c] = n;

            if (isWon()) {
                printBoard();
                printf("\n*** Congratulations, you solved it! ***\n");
                printf("Hints used: %d\n", hintsUsed);
                return askYesNo("\nPlay another game?");
            }
            if (countEmpty() == 0)
                printf("The grid is full, but something is wrong. Type c to check.\n");
        }
    }
}

int main(void)
{
    srand((unsigned)time(NULL));
    printf("=====================================\n");
    printf("        SUDOKU  (C, backtracking)\n");
    printf("=====================================\n");
    while (playRound())
        ;
    return 0;
}