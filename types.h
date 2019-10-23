/* Types.h */

// Linked list of blocks
typedef struct block
{
    int off_row;
    int off_col;
    int** matrix;
    int* top_row;
    int* left_col;
    int height;
    int width;
    struct block* next;  
} block_t;

typedef struct sendingblock
{
    int direction;
    int off_row;
    int off_col;
    int height;
    int width;
    int edge[]; //Could either be col or row
} sending_block_t;