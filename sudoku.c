#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 27

int sudoku[9][9] = {
    {6, 2, 4, 5, 3, 9, 1, 8, 7},
    {5, 1, 9, 7, 2, 8, 6, 3, 4},
    {8, 3, 7, 6, 1, 4, 2, 9, 5},
    {1, 4, 3, 8, 6, 5, 7, 2, 9},
    {9, 5, 8, 2, 4, 7, 3, 6, 1},
    {7, 6, 2, 3, 9, 1, 4, 5, 8},
    {3, 7, 1, 9, 5, 6, 8, 4, 2},
    {4, 9, 6, 1, 8, 2, 5, 7, 3},
    {2, 8, 5, 4, 7, 3, 9, 1, 6}
};

int valid[NUM_THREADS] = {0};

typedef struct {
    int row;
    int col;
    int thread_idx;
} parameters;

int check_validity(int arr[]) {
    int counts[10] = {0};
    for(int i=0; i<9; i++) {
        if(arr[i] < 1 || arr[i] > 9 || counts[arr[i]] > 0) return 0;
        counts[arr[i]]++;
    }
    return 1;
}

void *check_column(void *param) {
    parameters *data = (parameters *) param;
    int temp[9];
    for(int i=0; i<9; i++) temp[i] = sudoku[i][data->col];
    if(check_validity(temp)) valid[data->thread_idx] = 1;
    free(data);
    pthread_exit(NULL);
}

void *check_row(void *param) {
    parameters *data = (parameters *) param;
    int temp[9];
    for(int i=0; i<9; i++) temp[i] = sudoku[data->row][i];
    if(check_validity(temp)) valid[data->thread_idx] = 1;
    free(data);
    pthread_exit(NULL);
}

void *check_subgrid(void *param) {
    parameters *data = (parameters *) param;
    int temp[9], k = 0;
    for(int i=data->row; i<data->row+3; i++) {
        for(int j=data->col; j<data->col+3; j++) {
            temp[k++] = sudoku[i][j];
        }
    }
    if(check_validity(temp)) valid[data->thread_idx] = 1;
    free(data);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_count = 0;

    for (int i = 0; i < 9; i++) {
        parameters *data = (parameters *) malloc(sizeof(parameters));
        data->row = i; data->col = 0; data->thread_idx = thread_count;
        pthread_create(&threads[thread_count++], NULL, check_row, data);
    }
    for (int j = 0; j < 9; j++) {
        parameters *data = (parameters *) malloc(sizeof(parameters));
        data->row = 0; data->col = j; data->thread_idx = thread_count;
        pthread_create(&threads[thread_count++], NULL, check_column, data);
    }
    for (int i = 0; i < 9; i+=3) {
        for (int j = 0; j < 9; j+=3) {
            parameters *data = (parameters *) malloc(sizeof(parameters));
            data->row = i; data->col = j; data->thread_idx = thread_count;
            pthread_create(&threads[thread_count++], NULL, check_subgrid, data);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);

    int is_valid = 1;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (valid[i] == 0) is_valid = 0;
    }

    if (is_valid) printf("Sudoku puzzle is VALID.\n");
    else printf("Sudoku puzzle is INVALID.\n");

    return 0;
}