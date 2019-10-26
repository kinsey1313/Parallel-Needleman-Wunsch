/* Util that holds some misc functions */

#define SENDING_BLOCK_COUNT 7

MPI_Datatype register_send_block(int block_height, int block_width);
block_t* create_block(int height, int width, int off_row, int off_col);
sending_block_t* create_send_block(block_t* block, int direction, int slave_sender);
void update_send_block(block_t* block, sending_block_t* send_block, int direction, int slave_sender);
block_t* create_block_from_send(sending_block_t* send_block, sending_block_t* other_work, MPI_Datatype mpi_send_block_t);
sending_block_t* malloc_send_block(int height, int width);
block_t* master_next_block(sending_block_t* left, sending_block_t* up, int len_a, int len_b);
block_t* slave_next_block(block_t* block, sending_block_t* other_work, int direction, int slave_sender, int len_a, int len_b, MPI_Datatype mpi_send_block_t);
void free_block(block_t* block);
void free_send_block(sending_block_t* block);
void send_job(sending_block_t* send_block, int destination, MPI_Datatype mpi_send_block_t);

int get_other_direction(int direction);
void copy_final_column(int* buf, block_t* block);
void copy_first_column(block_t* block, int* buf);
void copy_final_row(int* buf, block_t* block);
void copy_first_row(block_t* block, int* buf); 
void copy_col_block(block_t* src, block_t* dest);
void copy_row_block(block_t* src, block_t* dest);
int max2(int a, int b);

int min2(int a, int b);

int max3(int a, int b, int c);

