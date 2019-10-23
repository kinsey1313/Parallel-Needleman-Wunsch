/* TAGS */
#define GOING_RIGHT 1
#define GOING_DOWN 2
#define FINISHED_WORK 3

/* RANKS */
#define MASTER 0

int get_next_worker(struct Queue* worker_queue);
void nwmpi(char* a, char* b, int len_a, int len_b, int rank, int size);
void print_mat(int** arr, int n_rows, int n_cols);
void print_block(block_t* block);
void print_arr(int* arr, int size);
// void calc_block(int** block, int b_height, int b_width, int off_col, int off_width);
void calc_block(block_t* block);
void stitch_both(int** block, int* col_1_row, int b_height, int b_width);
void stitch_column(int** block, int* col, int b_height, int b_width);
void stitch_row(int** block, int* row, int b_height, int b_width);
int calc_block_width(int len_a, int len_b, int size);
int calc_block_height(int len_a, int len_b, int size);
int calc_n_blocks(int len_a, int len_b, int size);

