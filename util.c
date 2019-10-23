/* util.c */
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "NW_mpi.h"
#include "util.h"



// Registers an MPI struct for block sending
MPI_Datatype register_send_block(int block_height, int block_width) {
    int edge_len = max2(block_height, block_width);
    int count = SENDING_BLOCK_COUNT;
    int array_of_blocklengths[count];
    MPI_Aint array_of_displacements[count];
    MPI_Datatype array_of_types[count];

    for(int i=0; i<count; i++) {
        array_of_blocklengths[i] = 1;
        array_of_displacements[i] = i * sizeof(int);
        array_of_types[i] = MPI_INT;
    }
    array_of_blocklengths[count-1] = edge_len;

    MPI_Datatype mpi_send_block_t;
    MPI_Type_create_struct(count, array_of_blocklengths,
                array_of_displacements, array_of_types,
                &mpi_send_block_t);

    MPI_Type_commit(&mpi_send_block_t);
    return mpi_send_block_t;
}

/*=======SEND/RECV STUFF==========*/
void send_job(sending_block_t* send_block, int destination, MPI_Datatype mpi_send_block_t) {
    MPI_Request request;
    MPI_Isend(send_block, 1, mpi_send_block_t, destination, 0, MPI_COMM_WORLD, &request);
}

/*======MALLOC STUFF==============*/
block_t* create_block(int height, int width, int off_row, int off_col) {
    block_t* block = malloc(sizeof(block_t));
    block->height = height;
    block->width = width;
    block->off_row = off_row;
    block->off_col = off_col;
    block->top_row = calloc(width, sizeof(int));
    block->left_col = calloc(height, sizeof(int));
    block->matrix = malloc(sizeof(int*) * height);
    for(int i=0; i<height; i++) {
        block->matrix[i] = calloc(width, sizeof(int));
    }
    // I think we still need a value for the corner.
    return block;
}

sending_block_t* create_send_block(block_t* block, int direction) {
    int height = block->height;
    int width = block->width;
    int edge_len = max2(height, width);
    sending_block_t *send_block = malloc(sizeof(*send_block) + sizeof(int) * edge_len);
    if (direction==GOING_RIGHT) {
        copy_final_column(send_block->edge, block);
    } 
    if(direction==GOING_DOWN) {
        copy_final_row(send_block->edge, block);
    }
    send_block->direction = direction;
    send_block->off_col = block->off_col;
    send_block->off_row = block->off_row;
    send_block->height = height;
    send_block->width = width;
    return send_block;
}

//Used by receivers to create a buffer for the incoming block
sending_block_t* malloc_send_block(int height, int width) {
    int edge_len = max2(height, width);
    sending_block_t* send_block = malloc(sizeof(*send_block) + sizeof(int) * edge_len);
    return send_block;
}



/*=========MISC HELPERS=========*/
void copy_final_column(int* buf, block_t* block) {
    int width = block->width;
    int height = block->height;
    for(int i=0; i<height; i++) {
        buf[i] = block->matrix[i][width-1];
    }
}

void copy_final_row(int* buf, block_t* block) {
    int width = block->width;
    int height = block->height;
    memcpy(buf, block->matrix[height-1], width);
}

int max2(int a, int b){
    if(a>b){
        return a;
    }
    return b;
}