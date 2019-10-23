/* Needleman MPI */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include "queue.h"
#include "types.h"
#include "util.h"
#include "NW_mpi.h"
#include <time.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init (&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char* a;
    char* b;

    int len_a;
    int len_b;
    if (rank==0) {
        //Do the file reading cuz
        a = "AGTCGGTA"; //size 8 for now
        b = "AGTTCATG"; //size 8 for now
        len_a = (int) strlen(a);
        len_b = (int) strlen(b);
    }

    MPI_Bcast(&len_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&len_b, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(&a, len_a, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b, len_b, MPI_CHAR, 0, MPI_COMM_WORLD);

    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //uint64_t start = GetTimeStamp();
    //printf("%d", A);
    nwmpi(a, b, len_a, len_b, rank, size);

    //printf("Time: %ld us\n", (uint64_t) (GetTimeStamp() - start));

    MPI_Finalize();
    return 0;

}

void nwmpi(char* a, char* b, int len_a, int len_b, int rank, int size) {
    
    int n_blocks = calc_n_blocks(len_a, len_b, size);
    int block_height = calc_block_height(len_a, len_b, size);
    int block_width = calc_block_width(len_a, len_b, size);

    MPI_Datatype mpi_send_block_t = register_send_block(block_height, block_width);

    /* ========== MASTER NODE =========== */
    if(rank==0) {
        // All the blocks master will hold
        block_t* block = create_block(block_height, block_width, 0, 0);
        // Create and initialise queue of workers
        struct Queue* worker_queue = createQueue(size-1);
        init_queue(worker_queue, size-1);

        //Initialise the sides
        for(int i=0; i>block->height; i++) {
            block->left_col[i] = i*-1;
        }

        for (int i=0; i>block->width; i++) {
            block->top_row[i] = i*-1;
        }

        calc_block(block);
        print_block(block);

        // // Now we take the processes out and send them the job
        int worker_right = dequeue(worker_queue);
         
        sending_block_t* job_right = create_send_block(block, GOING_RIGHT);
        printf("%d, %d, %d, %d, %d\n", job_right->direction, job_right->off_row, job_right->off_col, job_right->height, job_right->width);
        print_arr(job_right->edge, job_right->height);
        printf("Master done printing\n");
        // printf("Am I seg faulting here?\n");
        send_job(job_right, worker_right, mpi_send_block_t);
        // int proc_down = dequeue(worker_queue);
        free(job_right);

        free(block);
        free(worker_queue); 
    }
    /*========SLAVE NODE============*/
    else {
        MPI_Status status;
        sending_block_t* job = malloc_send_block(block_height, block_width);
        MPI_Recv(job, 1, mpi_send_block_t, MASTER, 0, MPI_COMM_WORLD, &status);
        printf("%d, %d, %d, %d, %d\n", job->direction, job->off_row, job->off_col, job->height, job->width);
        printf("well we did it fellas\n");
        print_arr(job->edge, job->height);
    }
}

void calc_block(block_t* block) {
    //TODO
    //TODO Make sure it uses the top row or left col if needed. These are not part 
    // Of the block for good reason
    for(int i=0; i<block->height; i++) {
        for(int j=0; j<block->width; j++) {
            block->matrix[i][j] = i+j;
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


void print_block(block_t* block) {
    printf("\n");
    for(int i = 0; i < block->height; i++){
        for (int j = 0; j < block->width; j++){
            printf("%d\t", block->matrix[i][j]);
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

int calc_block_width(int len_a, int len_b, int size) {
    return len_a / 4;
}

int calc_block_height(int len_a, int len_b, int size) {
    return len_b / 4;
}

// There are n * n blocks, this is actually sqrt(total blocks)
int calc_n_blocks(int len_a, int len_b, int size) {
    return 4;
}