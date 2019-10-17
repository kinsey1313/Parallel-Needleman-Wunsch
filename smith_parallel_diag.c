#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>

#define SMALL -1000

#define ROW 10

#define COL 10


void smith(char* str_a, char* str_b);
void print_arr(int** arr, int len_a, int len_b);
int max3(int x, int y, int z);
int max2(int x, int y);
void diagonalOrder(int matrix[ROW][COL]) ;


int main(){

    char* a = "GATTACACACC";
    char* b = "GATTACA";


    clock_t start, end;

    printf("%s\n", a);
    

    start = clock();
    smith(a, b);

    end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("%f \n", cpu_time_used);

    return 0;
}

void smith(char* str_a, char* str_b){
    int len_a = strlen(str_a)+1;
    int len_b = strlen(str_b)+1;
    
    //Array Setup --
    int** scores = (int**)malloc(sizeof(int*)*len_a);

    for (int i = 0; i < len_a; i++){
        scores[i] = (int*)malloc(sizeof(int)*len_b);
    }
    for(int r = 1; r<len_a; r++){
        for(int c = 1; c < len_b; c++){
            scores[r][c] = 0;
        }
    }

    for(int i = 0; i < len_a; i++)
        scores[i][0] = 0;
    for(int i = 0; i < len_b; i++)
        scores[0][i] = 0;
    //--

    //print_arr(scores, len_a, len_b);
    // printf("%d\n", biggest(5,2,3));

    // Smith Algorithm -- 
    int match = 2;
    int mismatch = -1;
    int gap = -2;
    int s;
    int val;



    print_arr(scores, len_a, len_b);
}

void diagonalOrder(int matrix[ROW][COL]) 
{ 
    // There will be ROW+COL-1 lines in the output 
    for (int line=1; line<=(ROW + COL -1); line++) 
    { 
        /* Get column index of the first element in this line of output. 
           The index is 0 for first ROW lines and line - ROW for remaining 
           lines  */
        int start_col =  max(0, line-ROW); 
  
        /* Get count of elements in this line. The count of elements is 
           equal to minimum of line number, COL-start_col and ROW */
         int count = min(line, (COL-start_col), ROW); 
  
        /* Print elements of this line */
        for (int j=0; j<count; j++){
            int x = 
            printf("%5d ", matrix[minu(ROW, line)-j-1][start_col+j]);
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

int max2(int x, int y){
    return x > y ? x : y;
}

int minu(int a, int b) 
{ return (a < b)? a: b; } 