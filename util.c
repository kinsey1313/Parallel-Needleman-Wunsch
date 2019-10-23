/* util.c */
#include <mpi.h>
#include <stdlib.h>
#include "types.h"
#include "NW_mpi.h"
#include "util.h"


// Registers an MPI struct for block sending
void register_send_block(int block_height, int block_width) {
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
}


/*======MALLOC STUFF==============*/
void create_block(block_t* block, int height, int width, int off_row, int off_col) {
    block = malloc(sizeof(block_t));
    block->height = height;
    block->width = width;
    block->off_row = off_row;
    block->off_col = off_col;
    block->top_row = malloc(sizeof(int) * width);
    block->left_col = malloc(sizeof(int) * height);
    block->matrix = malloc(sizeof(int*) * height);
    for(int i=0; i<width; i++) {
        block->matrix[i] = malloc(sizeof(int) * width);
    }
    // I think we still need a value for the corner.
}

int max2(int a, int b){
    if(a>b){
        return a;
    }
    return b;
}