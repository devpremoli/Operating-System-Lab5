#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

//int matA[MAX][MAX];
//int matB[MAX][MAX];
int **matA, **matB;

//int matSumResult[MAX][MAX];
//int matDiffResult[MAX][MAX];
//int matProductResult[MAX][MAX];
int **matSumResult, **matDiffResult, **matProductResult;

int MAX;

typedef struct {
    int startRow;
    int endRow;
} ThreadArg;

// Function to dynamically allocate a matrix
int** allocateMatrix(int size) {
    int **matrix = malloc(size * sizeof(int *));
    if (matrix == NULL) {
        perror("Failed to allocate memory for matrix");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < size; i++) {
        matrix[i] = malloc(size * sizeof(int));
        if (matrix[i] == NULL) {
            perror("Failed to allocate memory for matrix row");
            exit(EXIT_FAILURE);
        }
    }
    return matrix;
}

// Function to free a dynamically allocated matrix
void freeMatrix(int **matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}


/*
void fillMatrix(int matrix[MAX][MAX]) {
    for(int i = 0; i<MAX; i++) {
        for(int j = 0; j<MAX; j++) {
            matrix[i][j] = rand()%10+1;
        }
    }
}
 */
void fillMatrix(int **matrix, int size) {
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            matrix[i][j] = rand() % 10 + 1;
        }
    }
}

/*
void printMatrix(int matrix[MAX][MAX]) {
    for(int i = 0; i<MAX; i++) {
        for(int j = 0; j<MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
 */
void printMatrix(int **matrix, int size) {
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}


// Fetches the appropriate coordinates from the argument, and sets
// the cell of matSumResult at the coordinates to the sum of the
// values at the coordinates of matA and matB.
void* computeSum(void* args) {
    ThreadArg *arg = (ThreadArg *)args;
    for (int i = arg->startRow; i < arg->endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            matSumResult[i][j] = matA[i][j] + matB[i][j];
        }
    }
    free(arg);
    return NULL;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matSumResult at the coordinates to the difference of the
// values at the coordinates of matA and matB.
void* computeDiff(void* args) {
    ThreadArg *arg = (ThreadArg *)args;
    for (int i = arg->startRow; i < arg->endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
        }
    }
    free(arg);
    return NULL;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matSumResult at the coordinates to the inner product
// of matA and matB.
void* computeProduct(void* args) {
    ThreadArg *arg = (ThreadArg *)args;
    for (int i = arg->startRow; i < arg->endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            matProductResult[i][j] = 0;
            for (int k = 0; k < MAX; k++) {
                matProductResult[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    free(arg);
    return NULL;
}

// Spawn a thread to fill each cell in each result matrix.
// How many threads will you spawn?
int main(int argc, char *argv[]) {

    // Check for proper command line argument usage
    if (argc < 2) {
        printf("Usage: %s [matrix size]\n", argv[0]);
        return 1;
    }

    // Convert command line argument to integer and assign it to MAX
    MAX = atoi(argv[1]);
    if (MAX <= 0) {
        printf("Invalid matrix size. Please enter a positive integer.\n");
        return 1;
    }

    // Dynamically allocate matrices
    matA = allocateMatrix(MAX);
    matB = allocateMatrix(MAX);
    matSumResult = allocateMatrix(MAX);
    matDiffResult = allocateMatrix(MAX);
    matProductResult = allocateMatrix(MAX);

    srand(time(0));  // Do Not Remove. Just ignore and continue below.

    // 0. Get the matrix size from the command line and assign it to MAX

    // 1. Fill the matrices (matA and matB) with random values.
    fillMatrix(matA, MAX);
    fillMatrix(matB, MAX);

    // 2. Print the initial matrices.
    /*
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);
     */
    printf("Matrix A:\n");
    printMatrix(matA, MAX);
    printf("Matrix B:\n");
    printMatrix(matB, MAX);

    // 3. Create pthread_t objects for our threads.
    pthread_t sumThreads[MAX], diffThreads[MAX], prodThreads[MAX];

    // 4. Create a thread for each cell of each matrix operation.
    //
    // You'll need to pass in the coordinates of the cell you want the thread
    // to compute.
    //
    // One way to do this is to malloc memory for the thread number i, populate the coordinates
    // into that space, and pass that address to the thread. The thread will use that number to calcuate
    // its portion of the matrix. The thread will then have to free that space when it's done with what's in that memory.
    for (int i = 0; i < MAX; i++) {
        ThreadArg *sumArg = malloc(sizeof(ThreadArg));
        ThreadArg *diffArg = malloc(sizeof(ThreadArg));
        ThreadArg *prodArg = malloc(sizeof(ThreadArg));

        // Calculate the row range for each thread
        int rowsPerThread = MAX / MAX; // This could be optimized for unbalanced load
        sumArg->startRow = diffArg->startRow = prodArg->startRow = i * rowsPerThread;
        sumArg->endRow = diffArg->endRow = prodArg->endRow = (i + 1) * rowsPerThread;

        pthread_create(&sumThreads[i], NULL, computeSum, sumArg);
        pthread_create(&diffThreads[i], NULL, computeDiff, diffArg);
        pthread_create(&prodThreads[i], NULL, computeProduct, prodArg);
    }

    // 5. Wait for all threads to finish.
    for (int i = 0; i < MAX; i++) {
        pthread_join(sumThreads[i], NULL);
        pthread_join(diffThreads[i], NULL);
        pthread_join(prodThreads[i], NULL);
    }

    // 6. Print the results.
    /*
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);
     */
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult, MAX);
    printf("Difference:\n");
    printMatrix(matDiffResult, MAX);
    printf("Product:\n");
    printMatrix(matProductResult, MAX);

    // Free allocated memory
    freeMatrix(matA, MAX);
    freeMatrix(matB, MAX);
    freeMatrix(matSumResult, MAX);
    freeMatrix(matDiffResult, MAX);
    freeMatrix(matProductResult, MAX);

    return 0;

}
