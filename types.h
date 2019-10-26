/* Types.h */

// Linked list of blocks
typedef struct block
{
    int off_row;
    int off_col;
    int** matrix; // of size (height+1) * (width+1)
    int height;
    int width;
    struct block* next;  
} block_t;

typedef struct sendingblock
{
    int direction;
    int slave_sender;
    int off_row;
    int off_col;
    int height;
    int width;
    int edge[]; //Could either be col or row, including the corner
} sending_block_t;