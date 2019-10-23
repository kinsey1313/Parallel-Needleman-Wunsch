/* Util that holds some misc functions */

#define SENDING_BLOCK_COUNT 6

MPI_Datatype register_send_block(int block_height, int block_width);
block_t* create_block(int height, int width, int off_row, int off_col);
sending_block_t* create_send_block(block_t* block, int direction);
sending_block_t* malloc_send_block(int height, int width);
void send_job(sending_block_t* send_block, int destination, MPI_Datatype mpi_send_block_t);
// void recv_job();

void copy_final_column(int* buf, block_t* block);
void copy_final_row(int* buf, block_t* block);
int max2(int a, int b);
