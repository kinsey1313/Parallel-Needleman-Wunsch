# PMC-Assignment2

Parallel and Multicore Assignment 2
Jack Stinson and Kinsey Reeves



**Compilation and Execution** 

For overall combined NW using openMP and MPI (Main implementation):

`mpicc NW_mpi.c queue.c util.c -fopenmp -std=c99 -o NW_mpi`

and 

`mpiexec -np 10 ./NW_mpi data/m_single_100000.txt data/g_100000.txt`

Where the text files contain only a single line of the base sequence.



For NW-omp / Sequential only (for testing purposes)

`gcc NW_omp.c -o NW_omp -fopenmp`

and

`./NW_omp data/m_single_100000.txt data/g_100000.txt`


The Blocking which is spoken about in the report can be changed by changing 
TILING_NUMBER in NW_mpi.h

A high level description of the code can be found in description.txt

