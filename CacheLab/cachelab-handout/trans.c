/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, x, y;
    int block_size;
    int temp;
    int buff1, buff2, buff3, buff4, buff5, buff6, buff7;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    if (M == 32 && N == 32) {
        block_size = 8;
        for (i = 0; i <= N - block_size; i += block_size ) {
            for (j = 0; j <= M - block_size; j += block_size) {
                for (x = i; x < i + block_size; x++) {
                    for (y = j; y < j + block_size; y++) {
                        if (x != y) {
                            temp = A[x][y];
                            B[y][x] = temp;
                        }
                    }

                    if (i == j) {
                        temp = A[x][x];
                        B[x][x] = temp;
                    
                    }
                }
            }
        }
    }


    if (M == 64 && N == 64) {
        block_size = 8;
        for (i = 0; i <= N - block_size; i += block_size) {
            for (j = 0; j <= M - block_size; j += block_size) {
                // the subblock on the top-left    
                for (x = i; x < i + block_size/2; x ++) {
                    //miss
                    temp  = A[x][j];
                    //hit
                    buff1 = A[x][j+1];
                    buff2 = A[x][j+2];
                    buff3 = A[x][j+3];
                    buff4 = A[x][j+4];
                    buff5 = A[x][j+5];
                    buff6 = A[x][j+6];
                    buff7 = A[x][j+7];
                
                    //B[][x+4] are not in the right place now
                    B[j][x] = temp;
                    B[j][x+4] = buff4;
                    
                    B[j+1][x] = buff1;
                    B[j+1][x+4] = buff5;
                    
                    B[j+2][x] = buff2;
                    B[j+2][x+4] = buff6;

                    B[j+3][x] = buff3;
                    B[j+3][x+4] = buff7;
                }

                // the subblock on the diagonal
                for (x = 0; x < block_size/2; x++) {
                    buff1 = B[j+x][i+4];
                    buff2 = B[j+x][i+5];
                    buff3 = B[j+x][i+6];
                    buff4 = B[j+x][i+7];

                    buff5 = A[i+4][j+x];
                    buff6 = A[i+5][j+x];
                    buff7 = A[i+6][j+x];
                    temp  = A[i+7][j+x];

                    B[j+x][i+4] = buff5;
                    B[j+x][i+5] = buff6;
                    B[j+x][i+6] = buff7;
                    B[j+x][i+7] = temp;

                    B[j+x+4][i] = buff1;
                    B[j+x+4][i+1] = buff2;
                    B[j+x+4][i+2] = buff3;
                    B[j+x+4][i+3] = buff4;
                }

                // the subblock on the button-right
                for (x = i + 4; x < i + block_size; x++) {
                    buff1 = A[x][j+4];
                    buff2 = A[x][j+5];
                    buff3 = A[x][j+6];
                    buff4 = A[x][j+7];

                    B[j+4][x] = buff1;
                    B[j+5][x] = buff2;
                    B[j+6][x] = buff3;
                    B[j+7][x] = buff4;
                }
                  
            }
        }
    }

    if (M == 61 && N == 67) {
          block_size = 16;
          for (i = 0; i <= N-1; i += block_size ) {
               for (j = 0; j <= M - 1; j += block_size/4) {
                    for (x = i; x < i + block_size && x < N; x++) {
                         buff1 = A[x][j];
                         buff2 = A[x][j+1];
                         buff3 = A[x][j+2];
                         buff4 = A[x][j+3];

                         B[j][x] = buff1;
                         B[j+1][x] = buff2;
                         B[j+2][x] = buff3;
                         B[j+3][x] = buff4;
                    }
               }
          }
   }
    ENSURES(is_transpose(M, N, A, B));
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

