/* Util that holds some misc functions */

#define SENDING_BLOCK_COUNT 7

MPI_Datatype register_send_block(int block_height, int block_width);
block_t* create_block(int height, int width, int off_row, int off_col);
sending_block_t* create_send_block(block_t* block, int direction, int slave_sender);
void update_send_block(block_t* block, sending_block_t* send_block, int direction, int slave_sender);
block_t* create_block_from_send(sending_block_t* send_block);
sending_block_t* malloc_send_block(int height, int width);
block_t* master_next_block(sending_block_t* left, sending_block_t* up);
void free_block(block_t* block);
void free_send_block(sending_block_t* block);
void send_job(sending_block_t* send_block, int destination, MPI_Datatype mpi_send_block_t);
// void recv_job();

int get_other_direction(int direction);
void copy_final_column(int* buf, block_t* block);
void copy_first_column(block_t* block, int* buf);
void copy_final_row(int* buf, block_t* block);
void copy_first_row(block_t* block, int* buf); 
int max2(int a, int b);
int max3(int a, int b, int c);
