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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

char trans_desc1[] = "32x32 Optimize by divide 8x8 1";
void trans1(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i1, j1;
    for (i = 0; i < 32; i += 8) {
        for (j = 0; j < 32; j += 8) {
            for (i1 = i; i1 < i + 8; i1++) {
                for (j1 = j; j1 < j + 8; j1++) {
                    B[j1][i1] = A[i1][j1];
                }
            }
        }
    }
}

char trans_desc2[] = "32x32 Optimize by divide 8x8 2";
void trans2(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i1, j1;
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    for (i = 0; i < 32; i += 8) {
        for (j = 0; j < 32; j += 8) {
            for (i1 = i; i1 < i + 8; i1++) {
                if (i == j) {
                    tmp0 = A[i1][j];
                    tmp1 = A[i1][j + 1];
                    tmp2 = A[i1][j + 2];
                    tmp3 = A[i1][j + 3];
                    tmp4 = A[i1][j + 4];
                    tmp5 = A[i1][j + 5];
                    tmp6 = A[i1][j + 6];
                    tmp7 = A[i1][j + 7];
                    B[j][i1] = tmp0;
                    B[j + 1][i1] = tmp1;
                    B[j + 2][i1] = tmp2;
                    B[j + 3][i1] = tmp3;
                    B[j + 4][i1] = tmp4;
                    B[j + 5][i1] = tmp5;
                    B[j + 6][i1] = tmp6;
                    B[j + 7][i1] = tmp7;
                }
                else {
                    for (j1 = j; j1 < j + 8; j1++) {
                        B[j1][i1] = A[i1][j1];
                    }
                }
            }
        }
    }
}

char trans_desc3[] = "64x64 Optimize by divide 8x8 1";
void trans3(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i1, j1;
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    for (i = 0; i < 32; i += 8) {
        for (j = 0; j < 32; j += 8) {
            for (i1 = i; i1 < i + 8; i1++) {
                if (i == j) {
                    tmp0 = A[i1][j];
                    tmp1 = A[i1][j + 1];
                    tmp2 = A[i1][j + 2];
                    tmp3 = A[i1][j + 3];
                    tmp4 = A[i1][j + 4];
                    tmp5 = A[i1][j + 5];
                    tmp6 = A[i1][j + 6];
                    tmp7 = A[i1][j + 7];
                    B[j][i1] = tmp0;
                    B[j + 1][i1] = tmp1;
                    B[j + 2][i1] = tmp2;
                    B[j + 3][i1] = tmp3;
                    B[j + 4][i1] = tmp4;
                    B[j + 5][i1] = tmp5;
                    B[j + 6][i1] = tmp6;
                    B[j + 7][i1] = tmp7;
                }
                else {
                    for (j1 = j; j1 < j + 8; j1++) {
                        B[j1][i1] = A[i1][j1];
                    }
                }
            }
        }
    }
}

char trans_desc4[] = "64x64 Optimize by divide 4x4 1";
void trans4(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i1, j1;
    for (i = 0; i < 64; i += 4) {
        for (j = 0; j < 64; j += 4) {
            for (i1 = i; i1 < i + 4; i1++) {
                for (j1 = j; j1 < j + 4; j1++) {
                    B[j1][i1] = A[i1][j1];
                }
            }
        }
    }
}

char trans_desc5[] = "64x64 Optimize by divide 4x4 2";
void trans5(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i1, j1;
    int tmp0, tmp1, tmp2, tmp3;
    for (i = 0; i < 64; i += 4) {
        for (j = 0; j < 64; j += 4) {
            if (i == j) {
                for (i1 = i; i1 < i + 4; i1++) {
                    tmp0 = A[i1][j];
                    tmp1 = A[i1][j + 1];
                    tmp2 = A[i1][j + 2];
                    tmp3 = A[i1][j + 3];
                    B[j][i1] = tmp0;
                    B[j + 1][i1] = tmp1;
                    B[j + 2][i1] = tmp2;
                    B[j + 3][i1] = tmp3;
                }
            }
            else {
                for (j1 = j; j1 < j + 4; j1++) {
                    B[j1][i1] = A[i1][j1];
                }
            }
        }
    }
}

char trans_desc6[] = "64x64 Optimize by divide 4x4 3";
void trans6(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i1;
    int tmp0, tmp1, tmp2, tmp3;
    for (i = 0; i < 64; i += 4) {
        for (j = 0; j < 64; j += 4) {
            for (i1 = i; i1 < i + 4; i1++) {
                tmp0 = A[i1][j];
                tmp1 = A[i1][j + 1];
                tmp2 = A[i1][j + 2];
                tmp3 = A[i1][j + 3];
                B[j][i1] = tmp0;
                B[j + 1][i1] = tmp1;
                B[j + 2][i1] = tmp2;
                B[j + 3][i1] = tmp3;
            }
        }
    }
}

char trans_desc7[] = "64x64 Optimize by divide 4x4 4";
void trans7(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i1, j1;
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    for (i = 0; i < 64; i += 8) {
        for (j = 0; j < 64; j += 8) {
            for (i1 = i; i1 < i + 4; i1++) {
                tmp0 = A[i1][j];
                tmp1 = A[i1][j + 1];
                tmp2 = A[i1][j + 2];
                tmp3 = A[i1][j + 3];
                tmp4 = A[i1][j + 4];
                tmp5 = A[i1][j + 5];
                tmp6 = A[i1][j + 6];
                tmp7 = A[i1][j + 7];
                B[j][i1] = tmp0;
                B[j + 1][i1] = tmp1;
                B[j + 2][i1] = tmp2;
                B[j + 3][i1] = tmp3;
                B[j][i1 + 4] = tmp4;
                B[j + 1][i1 + 4] = tmp5;
                B[j + 2][i1 + 4] = tmp6;
                B[j + 3][i1 + 4] = tmp7;
            }
            for (j1 = j; j1 < j + 4; j1++) {
                tmp0 = A[i + 4][j1];
                tmp1 = A[i + 5][j1];
                tmp2 = A[i + 6][j1];
                tmp3 = A[i + 7][j1];
                tmp4 = B[j1][i + 4];
                tmp5 = B[j1][i + 5];
                tmp6 = B[j1][i + 6];
                tmp7 = B[j1][i + 7];
                B[j1][i + 4] = tmp0;
                B[j1][i + 5] = tmp1;
                B[j1][i + 6] = tmp2;
                B[j1][i + 7] = tmp3;
                B[j1 + 4][i] = tmp4;
                B[j1 + 4][i + 1] = tmp5;
                B[j1 + 4][i + 2] = tmp6;
                B[j1 + 4][i + 3] = tmp7;
            }
            for (i1 = i + 4; i1 < i + 8; i1++) {
                tmp0 = A[i1][j + 4];
                tmp1 = A[i1][j + 5];
                tmp2 = A[i1][j + 6];
                tmp3 = A[i1][j + 7];
                B[j + 4][i1] = tmp0;
                B[j + 5][i1] = tmp1;
                B[j + 6][i1] = tmp2;
                B[j + 7][i1] = tmp3;
            }
        }
    }
}

char trans_desc8[] = "61x67 Optimize by divide 16x16 1";
void trans8(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i1, j1, tmp;
    for (i = 0; i + 16 < N; i+=16) {
        for (j = 0; j + 16 < M; j+=16) {
            for (i1 = i; i1 < i + 16; i1++) {
                for (j1 = j; j1 < j + 16; j1++) {
                    tmp = A[i1][j1];
                    B[j1][i1] = tmp;
                }
            }
        }
    }
    for (; i1 < N; i1++) {
        for (j1 = 0; j1 < M; j1++) {
            tmp = A[i1][j1];
            B[j1][i1] = tmp;
        }
    }
    for (i1 = 0; i1 < i; i1++) {
        for (j1 = j; j1 < M; j1++) {
            tmp = A[i1][j1];
            B[j1][i1] = tmp;
        }
    }
}

char trans_desc9[] = "61x67 Optimize by divide 16x16 2";
void trans9(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i1, j1, tmp;
    for (i = 0; i < N; i+=16) {
        for (j = 0; j < M; j+=16) {
            for (i1 = i; i1 < N && i1 < i + 16; i1++) {
                for (j1 = j; j1 < M && j1 < j + 16; j1++) {
                    tmp = A[i1][j1];
                    B[j1][i1] = tmp;
                }
            }
        }
    }
}

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32 && N == 32)
        trans2(M, N, A, B);
    else if (M == 64 && N == 64)
        trans7(M, N, A, B);
    else
        trans9(M, N, A, B);
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

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

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
    // registerTransFunction(trans, trans_desc); 
    // registerTransFunction(trans1, trans_desc1);
    // registerTransFunction(trans2, trans_desc2);
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

