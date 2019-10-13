#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>

#define SMALL -1000


void smith(char* str_a, char* str_b);
void print_arr(int** arr, int len_a, int len_b);
int max3(int x, int y, int z);
int max2(int x, int y);


int main(){

    char* a = "GCATGCU";
    char* b = "GATTACA";


    clock_t start, end;

    printf("%s\n", a);

    start = clock();
    smith(a, b);

    end = clock();
    long cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

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
    for(int i = 1; i < len_a; i++){
        for (int j = 1; j < len_b; j++){
            
            if(str_b[i-1] == str_a[j-1]){
                s = match;
            }else{
                s = mismatch;
            }
            val = max3(scores[i-1][j] + gap, 
                            scores[i][j-1] + gap, 
                            scores[i-1][j-1] + s);
            scores[i][j] = max2(val, 0);
        }
    }
    print_arr(scores, len_a, len_b);
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