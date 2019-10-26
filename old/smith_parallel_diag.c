#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include <omp.h>
#include <string.h>

#define SMALL -1000

#define ROW 10

#define COL 10


void smith(char* str_a, char* str_b);
void print_arr(int** arr, int len_a, int len_b);
int max3(int x, int y, int z);
int max2(int x, int y);
void diag_order(int rows, int cols, int** matrix, char* str_a, char* str_b);
int minu(int a, int b);
int min3(int x, int y, int z);
void diag_order(scores);


int main(){

    char* b = "TGTTACGGTGTTACGGTGTT";
    char* a = "TGTTACGGTGTTACGGTGTGGT";

    clock_t start, end;

    //printf("%s\n", a);
    

    start = clock();
    //smith(a, b);

    end = clock();
    double cpu_time_used = ((double) (end - start));

    printf("%f \n", cpu_time_used);

    return 0;
}

void smith(char* str_a, char* str_b){
    int len_a = strlen(str_a)+1;
    int len_b = strlen(str_b)+1;
    
    //Array Setup --
    int** scores = (int**)malloc(sizeof(int*)*len_a);

    #pragma omp parallel for
    for (int i = 0; i < len_a; i++){
        scores[i] = (int*)malloc(sizeof(int)*len_b);
    }
    #pragma omp parallel for 
    for(int r = 1; r<len_a; r++){
        for(int c = 1; c < len_b; c++){
            scores[r][c] = 0;
        }
    }

    #pragma omp parallel for
    for(int i = 0; i < len_a; i++){
        scores[i][0] = 0;
        scores[0][i] = 0;
    }

    //print_arr(scores, len_a, len_b);
    // printf("%d\n", biggest(5,2,3));

    // Smith Algorithm -- 

    diag_order(len_a, len_b, scores, str_a, str_b);

    print_arr(scores, len_a, len_b);
}

void diag_order(int rows, int cols, int** matrix, char* str_a, char* str_b) 
{
    int match = 2;
    int mismatch = -1;
    int gap = -2;
    int s;
    int val;

    rows-=1;
    cols-=1;
    
    #pragma omp parallel for schedule(dynamic)
    for (int line=1; line<=(rows + cols -1); line++) 
    { 
        /* Get column index of the first element in this line of output. 
           The index is 0 for first ROW lines and line - ROW for remaining 
           lines  */
        int start_col =  max2(0, line-rows); 
  
        /* Get count of elements in this line. The count of elements is 
           equal to minimum of line number, COL-start_col and ROW */
        int count = min3(line, (cols-start_col), rows); 
  
        /* Print elements of this line */
        
        for (int j=0; j<count; j++){
            int x = minu(rows, line)-j-1+1;
            int y = start_col + j+1;

            printf("%d %d |", x, y);
            if(str_b[x-1] == str_a[y-1]){
                s = match;
            }else{
                s = mismatch;
            }
            int val = max3(matrix[x-1][y] + gap, 
                            matrix[x][y-1] + gap, 
                            matrix[x-1][y-1] + s);
            matrix[x][y] = max2(val, 0);
        }
        /* Ptint elements of next diagonal on next line */
        printf("\n"); 
    }
}



void print_arr(int** arr, int len_a, int len_b){

    for(int i = 0; i < len_a; i++){
        for (int j = 0; j < len_b; j++){
            printf("%d\t", arr[i][j]);
        }
        printf("\n");
    }

}

int max3(int x, int y, int z){
    return x > y ? (x > z ? x : z) : (y > z ? y : z);
}

int min3(int x, int y, int z){
    return minu(minu(x,y), z);
}

int max2(int x, int y){
    return x > y ? x : y;
}

int minu(int a, int b) 
{ return (a < b)? a: b; } 

void read_file(char* str, char*filename){
    FILE* filepointer;
    filepointer = fopen(filename, "r");
    char ch;
    int i = 0;
    while ((ch = fgetc(filepointer)) != EOF){
        str[i++] = ch;
    }
    str[i] = '\0';
}