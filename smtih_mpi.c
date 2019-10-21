/* Smith MPI */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

#include <time.h>

#define BLOCK_SIZE 10

uint64_t GetTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init (&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char* a;
    char* b;
    if (rank==0) {
        //Do the file reading cuz
        a = "GTAGGAC";
        b = "GTACCAG";
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);

    uint64_t start = GetTimeStamp();

    smith(a, b, rank, size)

    printf("Time: %ld us\n", (uint64_t) (GetTimeStamp() - start));

    MPI_Finalize();
    return 0;

}

/* Master slave MPI smith waterman */
void smith(char* a, char* b, int rank, int size) {
    if (rank==0) {
        // Set up the matrix. Only master has the full thing. 
        int len_a = strlen(a) + 1;
        int len_b = strlen(b) + 1;

        int** scores = (int**)malloc(sizeof(int*)*len_a);
        
        for(int i = 0; i < len_a; i++) {
            scores[i] = (int*)calloc(len_b, sizeof(int));
        }

        //Now that that's done, master does the first block itself in parallel

        do_first_block();

        //Now send 

        
    }
}

