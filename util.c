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
    int edge_len = max2(block_height, block_width)+2;
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
    block->matrix = malloc(sizeof(int*) * (height+1));
    for(int i=0; i<height+1; i++) {
        block->matrix[i] = malloc((width+1) * sizeof(int));
    }
    return block;
}

sending_block_t* create_send_block(block_t* block, int direction, int slave_sender) {
    int edge_len = max2(block->height, block->width) + 2;
    sending_block_t *send_block = malloc(sizeof(*send_block) + sizeof(int) * edge_len);
    update_send_block(block, send_block, direction, slave_sender);
    return send_block;
}

//Updates existing send block with details 
void update_send_block(block_t* block, sending_block_t* send_block, int direction, int slave_sender) {
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
    send_block->height = block->height;
    send_block->width = block->width;
}

//Create a block to work on based on the send_block that was received
block_t* create_block_from_send(sending_block_t* send_block, sending_block_t* other_work, MPI_Datatype mpi_send_block_t) {
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
            MPI_Status status;
            MPI_Recv(other_work, 1, mpi_send_block_t, slave_sender, INTER_SLAVE,
                                 MPI_COMM_WORLD, &status);
            copy_first_row(block, other_work->edge);
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
            MPI_Status status;
            MPI_Recv(other_work, 1, mpi_send_block_t, slave_sender, INTER_SLAVE,
                                 MPI_COMM_WORLD, &status);
            copy_first_column(block, other_work->edge);
        }
    }

    return block;
    
}

//Used by receivers to create a buffer for the incoming block
sending_block_t* malloc_send_block(int height, int width) {
    int edge_len = max2(height, width) + 2;
    sending_block_t* send_block = malloc(sizeof(*send_block) + sizeof(int) * edge_len);
    return send_block;
}

//Creates a new block for master to solve based on the received jobs
block_t* master_next_block(sending_block_t* left, sending_block_t* up, int len_a, int len_b) {
    int height = left->height;
    int width = up->width;
    int off_col = left->off_col + height;
    int off_row = up->off_row + width;
    height = min2(height, (len_b - off_col));
    width = min2(width, (len_a - off_row));
    block_t* block = create_block(height, width, off_row, off_col);
    //Now fill the edge values from the sent blocks
    copy_first_column(block, left->edge);
    copy_first_row(block, up->edge);
    return block;
}

block_t* slave_next_block(block_t* block, sending_block_t* other_work, int direction, int slave_sender, int len_a, int len_b, MPI_Datatype mpi_send_block_t) {
    //copy appropriate values into our block, then receive from our slave sender
    int off_row = block->off_row;
    int off_col = block->off_col;
    int height = block->height;
    int width = block->width;
    MPI_Status status;
    //Move offsets along
    if(direction==GOING_DOWN) {
        off_col += height;
        // printf("off_col %d and len_b %d\n", off_col, len_b);
        if(off_col>=len_b) { //We are done
            return NULL;
        }
        height = min2(height, (len_b - off_col));
    }
    if(direction==GOING_RIGHT) {
        off_row += width;
        // printf("off_row %d and len_a %d\n", off_row, len_a);
        if(off_row>=len_a) { //We are done
            return NULL;
        }
        width = min2(width, (len_a - off_row));
    }

    block_t* new_block = create_block(height, width, off_row, off_col);

    // Now we need to get the values we need
    if(direction==GOING_DOWN) {
        //Copy last row from previous into first row of this one
        copy_row_block(block, new_block);
        if(slave_sender==MASTER) {
            // We're on the edge, so initialise column 
            for(int i=0; i<height+1; i++) {
                new_block->matrix[i][0] = -1 * new_block->off_col - i;
            }
        }
        else {
            //Copy last column from other guy into our first one
            MPI_Recv(other_work, 1, mpi_send_block_t, slave_sender, INTER_SLAVE,
                                 MPI_COMM_WORLD, &status);
            copy_first_column(new_block, other_work->edge);
            printf("Received work from slave sender %d\n", slave_sender);
        }

    }
    if(direction==GOING_RIGHT) {
        copy_col_block(block, new_block);
        if(slave_sender==MASTER) {
            // We're on the edge, so intialise row
            for(int i=0; i<width+1; i++) {
                new_block->matrix[0][i] = -1 * new_block->off_row - i;
            }
        }
        else {
            // Need to receive the row from slave_sender
            MPI_Recv(other_work, 1, mpi_send_block_t, slave_sender, INTER_SLAVE,
                                 MPI_COMM_WORLD, &status);
            copy_first_row(new_block, other_work->edge);
        }
    }
    return new_block;

}

void free_block(block_t* block) {
    if(block==NULL) {
        return;
    }
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

int get_other_direction(int direction) {
    if(direction==GOING_DOWN) {
        return GOING_RIGHT;
    }
    if(direction==GOING_RIGHT) {
        return GOING_DOWN;
    }
    printf("Invalid direction %d\n", direction);
    return direction;
}

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

void copy_row_block(block_t* src, block_t* dest) {
    //Copy final row from source to first row in dest
    int width = src->width;
    int height = src->height;
    for(int i=0; i<width+1; i++) {
        dest->matrix[0][i] = src->matrix[height][i];
    }
}

void copy_col_block(block_t* src, block_t* dest) {
    //Copy final col from source to first col in dest
    int width = src->width;
    int height = src->height;
    for(int i=0; i<height+1; i++) {
        dest->matrix[i][0] = src->matrix[i][width];
    }
}

int max2(int a, int b){
    if(a>b){
        return a;
    }
    return b;
}

int min2(int a, int b) {
    if(a<b) {
        return a;
    }
    return b;
}

int max3(int x, int y, int z){
    return x > y ? (x > z ? x : z) : (y > z ? y : z);

}

char *get_input_str(char* filename, size_t size){
//The size is extended by the input with the value of the provisional
    FILE* fp = fopen(filename, "r");
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);//size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){
        str[len++]=ch;
        //printf("%c ", ch);
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}