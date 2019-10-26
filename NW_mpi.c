/* Needleman MPI */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include "queue.h"
#include <unistd.h>
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
        a = "ATATCAKSJDFKLJAS;DFDJOIENEFAKSNDFKASNDIFOANRILAKSNDFKNAGOIANRGILANFNSRFOIENRFDKFNSDKJFGEOIRNGSDKFNGDSG"; //size 8 for now
        b = "ATATAASDFINGSDFKNGSDFGJISEORNGSDLKFNGSERIONGSDKFLGSDIRGJISEORJGSDKRGJS;IDFJGSKDLFJG;SIDRJGSLDKFJGSIDRJ"; //size 8 for now
        len_a = (int) strlen(a);
        len_b = (int) strlen(b);
    }

    MPI_Bcast(&len_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&len_b, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(&a, len_a, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b, len_b, MPI_CHAR, 0, MPI_COMM_WORLD);

    

    //uint64_t start = GetTimeStamp();
    //printf("%d", A);
    nwmpi(a, b, len_a, len_b, rank, size);

    //printf("Time: %ld us\n", (uint64_t) (GetTimeStamp() - start));

    printf("Peace we out \n");
    MPI_Finalize();
    sleep(1);
    return 0;

}

void nwmpi(char* a, char* b, int len_a, int len_b, int rank, int size) {
    
    int n_blocks_master = calc_n_blocks(len_a, len_b, size);
    int block_height = calc_block_height(len_a, len_b, size);
    int block_width = calc_block_width(len_a, len_b, size);
    printf("We begin with %d by %d blocks of height %d and width %d\n", n_blocks_master, n_blocks_master, block_height, block_width);

    MPI_Status status;
    MPI_Request request;
    MPI_Datatype mpi_send_block_t = register_send_block(block_height, block_width);

    /* ========== MASTER NODE =========== */
    if(rank==0) {
        // First block for master
        block_t* block = create_block(block_height, block_width, 0, 0);
        block_t* start_block = block; //Keep a pointer to the first one
        // Create and initialise queue of workers
        struct Queue* worker_queue = createQueue(size-1);
        init_queue(worker_queue, size-1);
        
        //Malloc two send_blocks to send work away
        sending_block_t* job_right = malloc_send_block(block->height, block->width);
        sending_block_t* job_down = malloc_send_block(block->height, block->width);
        int worker_right=0;
        int worker_down=0;
        int old_right;
        int old_down;

        for(int block_num=0; block_num<n_blocks_master; block_num++) { 
            // sleep(2);
            
            if(block_num==0) { //Initialise the sides
                for(int i=0; i<block->height+1; i++) {
                    block->matrix[i][0] = i*-1;
                }
                for (int i=0; i<block->width+1; i++) {
                    block->matrix[0][i] = i*-1;
                }
            }

            else { //Receive the values from our workers
                MPI_Status status;
                // printf("Master is waiting to receive work from workers %d and %d\n", worker_right, worker_down);
                MPI_Recv(job_right, 1, mpi_send_block_t, worker_right, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                MPI_Recv(job_down, 1, mpi_send_block_t, worker_down, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                // printf("Master Received the work\n");
                
                block->next = master_next_block(job_right, job_down, len_a, len_b);
                block = block->next;
            }

            calc_block(block);
            printf("Master printing block %d with row off %d and col off %d\n", block_num, block->off_row, block->off_col);
            print_block(block);

            if(block_num==n_blocks_master-1) { //If the last block, we don't need to send it to anyone
                printf("Master is all done\n");
                break;
            }

            // // Now we take the processes out and send them the job

            old_right = worker_right;
            worker_right = get_next_worker(worker_queue);
            update_send_block(block, job_right, GOING_RIGHT, old_right);
            send_job(job_right, worker_right, mpi_send_block_t);

            old_down = worker_down;
            worker_down = get_next_worker(worker_queue);
            update_send_block(block, job_down, GOING_DOWN, old_down);
            send_job(job_down, worker_down, mpi_send_block_t);


            if(block_num>0) { //Send the old workers their slave_receivers
                // printf("Sending slave receiver %d to worker %d \n ", worker_right, old_right);
                MPI_Isend(&worker_right, 1, MPI_INT, old_right, SLAVE_RECEIVER, MPI_COMM_WORLD, &request);
                // printf("Sending slave receiver %d to worker %d \n ", worker_down, old_down);
                MPI_Isend(&worker_down, 1, MPI_INT, old_down, SLAVE_RECEIVER, MPI_COMM_WORLD, &request);
            }
            // sleep(2);
        }

        // Tell all slaves to die
        for(int i=1; i<size; i++) {
            MPI_Isend(NULL, 0, MPI_INT, i, DIE_SLAVE, MPI_COMM_WORLD, &request);
        }
        free_send_block(job_right);
        free_send_block(job_down);
        free_block(block);
        free_queue(worker_queue); 
    }
    /*========SLAVE NODE============*/
    else {
        /* Initialise */
        block_t* start_block = NULL; //Pointer to first one in linked list
        sending_block_t* send_block = malloc_send_block(block_height, block_width);
        sending_block_t* other_work = malloc_send_block(block_height, block_width); //For work received from slave_sender

        /* Waiting for work */

        while(1) {
            sleep(1);
            MPI_Probe(MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if(status.MPI_TAG == DIE_SLAVE) { //Goodbye, slave
                break;
            }
            MPI_Recv(send_block, 1, mpi_send_block_t, MASTER, 0, MPI_COMM_WORLD, &status);
            int direction = send_block->direction; //The way the slave will continue on
            int slave_sender = send_block->slave_sender; //Where to receive the other values from
            block_t* block = create_block_from_send(send_block, other_work, mpi_send_block_t);
            printf("Slave %d is beginning work on block with row_off %d and col_off %d\n", rank, block->off_row, block->off_col);

            // Maintain the linked list
            if(start_block==NULL) {
                start_block = block;
            }

            calc_block(block);
            // printf("Slave printing block %d with offsets row %d and col %d:\n", rank, block->off_row, block->off_col);
            print_block(block); 
            

            // Send first phase results back to master
            int other_direction = get_other_direction(direction);
            update_send_block(block, send_block, other_direction, slave_sender);
            MPI_Isend(send_block, 1, mpi_send_block_t, MASTER, 0, MPI_COMM_WORLD, &request);

            //Now we continue on in our direction
            block_t* next_block = slave_next_block(block, direction, slave_sender, len_a, len_b);
            int in_first = 1;
            int slave_receiver;
            while(next_block!=NULL) {
                sleep(1);

                //Maintain linked list
                block->next = next_block;
                block = next_block;

                //Calculate block
                calc_block(block);
                // printf("Slave printing block %d with offsets row %d and col %d:\n", rank, block->off_row, block->off_col);
                print_block(block); 

                if(in_first==1) { //Have to find out from master who our slave_receiver is
                    //TODO this is written under the assumption that blocks that don't need
                    // A slave receiver will not his this block. If you get a bug later,
                    // That's probably why
                    MPI_Recv(&slave_receiver, 1, MPI_INT, MASTER, SLAVE_RECEIVER, MPI_COMM_WORLD, &status);
                    in_first = 0;
                }

                update_send_block(block, send_block, other_direction, slave_sender);
                MPI_Isend(send_block, 1, mpi_send_block_t, slave_receiver, 0, MPI_COMM_WORLD, &request);
                next_block = slave_next_block(block, direction, slave_sender, len_a, len_b); 
            }

        printf("Slave %d is finished and ready for more\n", rank);
        MPI_Send(&rank, 1, MPI_INT, MASTER, FINISHED_WORK, MPI_COMM_WORLD);
        }

        printf("Slave %d killed\n", rank);
        sleep(1);
        free_send_block(send_block);
        free_block(start_block);
        sleep(2);
    }

}

void calc_block(block_t* block) {
    //TODO
    for(int i=1; i<=block->height; i++) {
        for(int j=1; j<=block->width; j++) {
            block->matrix[i][j] = block->matrix[i-1][j-1]+1;
        }
    }
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
    // printf("\n");
    // for(int i = 0; i < block->height+1; i++){
    //     for (int j = 0; j < block->width+1; j++){
    //         printf("%d\t", block->matrix[i][j]);
    //     }
    //     printf("\n");
    // }
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
    if(len_a%4 || len_b%4) {
        return 5;
    }
    return 4;
}