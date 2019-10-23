/* util.c */
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "NW_mpi.h"
#include "util.h"



// Registers an MPI struct for block sending
MPI_Datatype register_send_block(int block_height, int block_width) {
    int edge_len = max2(block_height, block_width)+1;
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
    block->next = NULL;
    block->matrix = malloc(sizeof(int*) * height+1);
    for(int i=0; i<height+1; i++) {
        block->matrix[i] = calloc(width+1, sizeof(int));
    }
    return block;
}

sending_block_t* create_send_block(block_t* block, int direction, int slave_sender) {
    int height = block->height;
    int width = block->width;
    int edge_len = max2(height, width) + 1;
    sending_block_t *send_block = malloc(sizeof(*send_block) + sizeof(int) * edge_len);
    if (direction==GOING_RIGHT) {
        copy_final_column(send_block->edge, block);
    } 
    if(direction==GOING_DOWN) {
        copy_final_row(send_block->edge, block);
    }
    send_block->direction = direction;
    send_block->slave_sender = slave_sender;
    send_block->off_col = block->off_col;
    send_block->off_row = block->off_row;
    send_block->height = height;
    send_block->width = width;
    return send_block;
}

//Create a block to work on based on the send_block that was received
block_t* create_block_from_send(sending_block_t* send_block) {
    int height = send_block->height;
    int width = send_block->width;
    int off_row = send_block->off_row;
    int off_col = send_block->off_col;
    int slave_sender = send_block->slave_sender;
    block_t* block = create_block(height, width, off_row, off_col);
    int direction = send_block->direction;
    if(direction==GOING_RIGHT) {
        //Increment offset
        block->off_row += width;
        //Copy into first column, including corner value
        copy_first_column(block, send_block->edge);
        if(slave_sender==MASTER) { //Case where we're on the edge
            // initialise first row values accordingly
            for(int i=0; i<width+1; i++) {
                block->matrix[0][i] = -1 * block->off_row - i;
            }
        }
        else {
            //TODO
            //Need to receive the top row from our slave sender
            MPI_Status status;
            MPI_Recv(block->matrix[0], width+1, MPI_INT, slave_sender, MPI_ANY_TAG,
                                 MPI_COMM_WORLD, &status); //idk if this will work
        }
    }
    if(direction==GOING_DOWN) {
        //Increment offset
        block->off_col += height;
        //Copy into first row, including corner value
        copy_first_row(block, send_block->edge);
        if(slave_sender==MASTER) { //Case where we're on the edge
            // initialise first col values accordingly
            for(int i=0; i<height+1; i++) {
                block->matrix[i][0] = -1 * block->off_col - i;
            }
        }
        else {
            //TODO
            //Need to receive the top col from our slave sender
            MPI_Status status;
            int* buf = malloc(sizeof(int) * height+1);
            MPI_Recv(buf, height+1, MPI_INT, slave_sender, MPI_ANY_TAG,
                                MPI_COMM_WORLD, &status);
            copy_first_column(block, buf);
            free(buf);
        }
    }

    return block;
    
}

//Used by receivers to create a buffer for the incoming block
sending_block_t* malloc_send_block(int height, int width) {
    int edge_len = max2(height, width) + 1;
    sending_block_t* send_block = malloc(sizeof(*send_block) + sizeof(int) * edge_len);
    return send_block;
}

void free_block(block_t* block) {
    for(int i=0; i<block->height+1; i++) {
        free(block->matrix[i]);
    }
    free(block->matrix);
    if(block->next != NULL) {
        free_block(block->next);
    }
    free(block);
}

void free_send_block(sending_block_t* block) {
    free(block);
}

/*=========MISC HELPERS=========*/
void copy_final_column(int* buf, block_t* block) {
    int width = block->width+1;
    int height = block->height+1;
    for(int i=0; i<height; i++) {
        buf[i] = block->matrix[i][width-1];
    }
}

void copy_first_column(block_t* block, int* buf) {
    int height = block->height+1;
    for(int i=0; i<height; i++) {
        block->matrix[i][0] = buf[i];
    }
}

void copy_final_row(int* buf, block_t* block) {
    int width = block->width+1;
    int height = block->height+1;
    // memcpy(buf, block->matrix[height-1], width);
    for(int i=0; i<width; i++) {
        buf[i] = block->matrix[height-1][i];
    }
}

void copy_first_row(block_t* block, int* buf) {
    int width = block->width+1;
    for(int i=0; i<width; i++) {
        block->matrix[0][i] = buf[i];
    }
}

int max2(int a, int b){
    if(a>b){
        return a;
    }
    return b;
}