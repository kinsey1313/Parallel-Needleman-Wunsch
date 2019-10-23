/* Needleman MPI */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include "queue.h"

#include <time.h>

#define T_ROW_ONLY 1
#define T_COL_ONLY 2

#define BLOCK_SIZE 10

// uint64_t GetTimeStamp() {
//     struct timeval tv;
//     gettimeofday(&tv,NULL);
//     return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
// }

void print_mat(int** arr, int n_rows, int n_cols);
void print_arr(int* arr, int size);
void calc_block(int** block, int b_height, int b_width, int off_col, int off_width);
void stitch_both(int** block, int* col_1_row, int b_height, int b_width);
void stitch_column(int** block, int* col, int b_height, int b_width);
void stitch_row(int** block, int* row, int b_height, int b_width);

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init (&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char* a;
    char* b;
    if (rank==0) {
        //Do the file reading cuz
        a = "GTACA";
        b = "GTACA";
    }

    MPI_Bcast(&a, strlen(a), MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b, strlen(b), MPI_CHAR, 0, MPI_COMM_WORLD);

    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //uint64_t start = GetTimeStamp();
    //printf("%d", A);
    nwmpi(a, b, rank, size);

    //printf("Time: %ld us\n", (uint64_t) (GetTimeStamp() - start));

    MPI_Finalize();
    return 0;

}

void nwmpi(char* a, char* b, int rank, int size) {
    /* ========== MASTER =========== */
    if(rank==0) {
        int** scores = (int**)malloc(sizeof(int*)*len_a);

        for(int i = 0; i < len_a; i++) {
            scores[i] = (int*)calloc(len_b, sizeof(int));
        }

        //TODO Initialise NW values to correct ones
    }
}









void copy_block_master(){
    //Master only function, copies the received block onto master
    //Copies block onto master process
}

void calc_block(int** block, int b_height, int b_width, int off_col, int off_width){
    for (int i = 1; i < b_height; i++){
        for (int j = 1; j < b_width; j++){
            //do algo here using openmp
            //block[i][j] = i + j;
        }
    }

}

void stitch_column(int** block, int* col, int b_height, int b_width){
    //Stitches column onto the first index of our block;
    //also adds the zeros TODO do this in one loop
    
    for (int i = 0; i <b_height; i++){
        block[i][0] = col[i];
    }
    for (int i =0; i < b_width; i++){
        block[0][i]=0;
    }
}

void stitch_row(int** block, int* row, int b_height, int b_width){
    //Stitches the row onto the block
    //TODO do in one loop as above i.e. max of
    //IDK which one is quicker
    // int i = 0
    // int max_len = max2(b_height, b_width);
    // while(i < max_len){
    //     if(i < b_height){
    //         block[0][i] = col[i];
    //     }
    //     if(i < max_width){
    //         block[i][0] = 0;
    //     }
    //     i++;
    // }
    
    for(int i = 0; i<b_width; i++){
        block[0][i] = row[i];
    }
    for (int i = 0; i < b_height; i++){
        block[i][0] = 0;
    }
}

void stitch_both(int** block, int* col_1_row, int b_height, int b_width){
    int max_len = b_height + b_width + 1 ;
    //make col_1_row in 
    for (int i = b_height-1; i >= 0; i--){
        block[i][0] = col_1_row[i];
    }
    block[0][0] = col_1_row[b_height];
    for(int i = b_height+1; i < max_len; i++){
        block[0][i] = col_1_row[i];
    }
}


void print_mat(int** arr, int n_rows, int n_cols){
    printf("\n");
    for(int i = 0; i < n_rows; i++){
        for (int j = 0; j < n_cols; j++){
            //printf("%d\t", arr[i][j]);
            printf("%d\t", arr[i][j]);
        }
        printf("\n");
    }
    
}

void print_arr(int* arr, int size){

    for (int i = 0; i < size; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
}


void get_last_row(int** scores, int *out, int b_width,int b_height, int off_row, int off_col){
    
    for (int i = 0; i < b_width; i++){
        out[i] = scores[off_row + b_height][off_col + i];
    }
}


void get_last_col(int** scores, int *out, int b_width,int b_height, int off_row, int off_col){
    for (int i = 0; i < b_height; i++){
        out[i] = scores[off_row + i][off_col+b_width];
    }
}



void request_row(){
    //get the required row from master


}

void request_column(){
    //get the required column from master

}


void send_result(){
    //Sends the result from worker to master

}

void receive_result(){
    //receive the result in master and stitches together

}


void get_adjacent_process(){
    //master sends process which is either always right or always below the
    //current process
}

int max2(int a, int b){
    if(a>b){
        return a;
    }
    return b;
}