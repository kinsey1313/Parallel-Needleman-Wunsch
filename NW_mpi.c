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

    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size<5) {
        printf("Error, the algorithm is only defined for >=5 nodes\n");
        exit(0);
    }

    char* a;
    char* b;

    int len_a;
    int len_b;
    if (rank==0) {
        printf("Size is %d\n", size);
        //Do the file reading cuz
        a = "AGTCGGTA"; //size 8 for now
        b = "AGTTCATG"; //size 8 for now
        len_a = (int) strlen(a);
        len_b = (int) strlen(b);
    }

    //DELETE

    

    int block_height = calc_block_height(len_a, len_b, size);
    int block_width = calc_block_width(len_a, len_b, size);
    block_t* block = create_block(block_height, block_width, 0, 0);
    for(int i=0; i<block->height+1; i++) {
            block->matrix[i][0] = i*-1;
        }

    for (int i=0; i<block->width+1; i++) {
        block->matrix[0][i] = i*-1;
    }
    
    calc_block(block, a,b);
    print_block(block);
    exit(0);

    ////DELETE

    
    

    MPI_Bcast(&len_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&len_b, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(&a, len_a, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b, len_b, MPI_CHAR, 0, MPI_COMM_WORLD);

    

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
        // First block for master
        block_t* block = create_block(block_height, block_width, 0, 0);
        block_t* start_block = block; //Keep a pointer to the first one
        // Create and initialise queue of workers


        struct Queue* worker_queue = createQueue(size-1);
        init_queue(worker_queue, size-1);

        //Initialise the sides
        for(int i=0; i<block->height+1; i++) {
            block->matrix[i][0] = i*-1;
        }

        for (int i=0; i<block->width+1; i++) {
            block->matrix[0][i] = i*-1;
        }

        calc_block(block, a, b);
        printf("Master printing block:\n");
        print_block(block);

        // // Now we take the processes out and send them the job
        int worker_right = get_next_worker(worker_queue);
        sending_block_t* job_right = create_send_block(block, GOING_RIGHT, MASTER);
        send_job(job_right, worker_right, mpi_send_block_t);

        int worker_down = get_next_worker(worker_queue);
        sending_block_t* job_down = create_send_block(block, GOING_DOWN, MASTER);
        send_job(job_down, worker_down, mpi_send_block_t);

        //Now receive the results of those two
        MPI_Status status;
        MPI_Recv(job_right, 1, mpi_send_block_t, worker_right, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(job_down, 1, mpi_send_block_t, worker_down, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        //And use the results to do the next block.
        block_t* next_block = master_next_block(job_right, job_down);
        block->next = next_block;
        block = next_block; // Shift the pointers along
        printf("Master printing block:\n");
        print_block(next_block);


        free_send_block(job_right);
        free_send_block(job_down);
        free_block(block);
        free_queue(worker_queue); 
    }
    /*========SLAVE NODE============*/
    else {
        MPI_Status status;
        MPI_Request request;
        sending_block_t* send_block = malloc_send_block(block_height, block_width);
        MPI_Recv(send_block, 1, mpi_send_block_t, MASTER, 0, MPI_COMM_WORLD, &status);
        int direction = send_block->direction; //The way the slave will continue on
        int slave_sender = send_block->slave_sender; //Where to receive the other values from
        block_t* block = create_block_from_send(send_block);
        calc_block(block, a, b);
        printf("Slave printing block:\n");
        print_block(block);
        int other_direction = get_other_direction(direction);
        update_send_block(block, send_block, other_direction, slave_sender);
        MPI_Isend(send_block, 1, mpi_send_block_t, MASTER, 0, MPI_COMM_WORLD, &request);


        free_send_block(send_block);
        free_block(block);
    }
}

void calc_block(block_t* block, char* str_a, char* str_b) {
    //TODO
    //TODO Make sure it uses the top row or left col if needed. These are not part 
    // Of the block for good reason
    wunch_omp(block->matrix, &str_a[block->off_col], 
        &str_b[block->off_row], block->height, block->width);

    print_block(block);

}

void wunch_omp(int** scores, char* str_a, char* str_b, int len_a, int len_b){
    // Calculates along the diagonal according to NW algorithm. uses
    // Two for loops one for first anti diagonal half of the matrix
    //len_a-=1;
    //len_b-=1;

    for (int k = 0; k < len_a; k++){
        #pragma omp parallel for
        for (int j = 0; j <=k; j++){
            int i = k - j;
            //printf("%d %d |", i+1, j+1);
            wunch_score(scores, str_a, str_b, i+1, j+1);
        }
        printf("\n");
    }
    

    for (int k = len_b - 2; k >=0; k--){
        #pragma omp parallel for
        for (int j = 0; j <=k; j++){
            int i = k - j;
            //printf("%d %d |", len_b - j, len_a - i);
            wunch_score(scores,str_a, str_b, len_a - i, len_b-j);
        }
        printf("\n");
    }
}

void wunch_score(int** scores, char* str_a, char* str_b, int i, int j){
    int match = 1;
    int mismatch = -1;
    int gap = -1;
    int s;
    int val;

    if(str_a[i] == str_b[j]){
        s = match;
    }else{
        s = mismatch;
    }
    val = max3(scores[i-1][j] + gap, 
                            scores[i][j-1] + gap, 
                            scores[i-1][j-1] + s);
    scores[i][j] = val;
}

int get_next_worker(struct Queue* worker_queue) {
    int worker;
    MPI_Status status;
    if(isEmpty(worker_queue)) {
        //blocking recv to get a ready worker
        MPI_Recv(&worker, 1, MPI_INT, MPI_ANY_SOURCE, FINISHED_WORK, MPI_COMM_WORLD, &status);
    }
    else {
        worker = dequeue(worker_queue);
    }
    return worker;
}


void print_block(block_t* block) {
    printf("\n");
    for(int i = 0; i < block->height+1; i++){
        for (int j = 0; j < block->width+1; j++){
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