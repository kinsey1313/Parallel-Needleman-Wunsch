/* Util that holds some misc functions */

#define SENDING_BLOCK_COUNT 8

void register_send_block(int block_height, int block_width);
void create_block(block_t* block, int height, int width, int off_row, int off_col);
int max2(int a, int b);
