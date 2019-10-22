#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>


#define MAX_STRING 1000

void smith_score(int** scores, char* str_a, char* str_b, int i, int j);
void print_arr(int** arr, int len_a, int len_b);
int max2(int x, int y);
int max3(int x, int y, int z);
char *get_input_str(char* filename, size_t size);
void smith(int** scores, char* str_a, char* str_b, int len_a, int len_b);

uint64_t GetTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

int main(){
    int arr[3][4] = {{1,2,3,4}, {4,5,6,4}, {7,8,9,4}};
    int dim = 4;
    
    char* str_a = get_input_str("out_genes.txt", MAX_STRING);
    char* str_b = get_input_str("out_mutes.txt", MAX_STRING);

    int len_a = strlen(str_a)+1;
    int len_b = strlen(str_a)+1;
    printf("len of string a : %d", len_a);

    
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
        scores[i][0] = -i;
    for(int i = 0; i < len_b; i++)
        scores[0][i] = -i;
    len_a-=1;
    len_b-=1;
    
    uint64_t start = GetTimeStamp ();
    smith(scores,str_a,str_b,len_a,len_b);
    printf ("Time: %ld us\n", (uint64_t) (GetTimeStamp() - start));
    
    //print_arr(scores, len_a+1, len_b+1);


    // int WIDTH = 3;
    // int HEIGHT = 4;

    // for(int k = 0 ; k <= WIDTH + HEIGHT - 2; k++ ) {
    //     for( int j = 0 ; j <= k ; j++ ) {
    //         int i = k - j;
    //         if( i < HEIGHT && j < WIDTH ) {
    //             printf("%d %d |", i, j);
    //         }

    //     }
    //     printf("\n");
    // }

    // return 0;
}

void smith(int** scores, char* str_a, char* str_b, int len_a, int len_b){

    
    for (int k = 0; k < len_a; k++){
        #pragma omp parallel for schedule(runtime)
        for (int j = 0; j <=k; j++){
            int i = k - j;
            //printf("%d %d |", i+1, j+1);
            smith_score(scores,str_a, str_b, i+1, j+1);
        }
        //printf("\n");
    }

    
    for (int k = len_b - 2; k >=0; k--){
        #pragma omp parallel for schedule(runtime)
        for (int j = 0; j <=k; j++){
            int i = k - j;
            //printf("%d %d |", len_b - j, len_b - i);
            smith_score(scores,str_a, str_b, len_b - j, len_b - i);
        }
        //printf("\n");
    }
}

void smith_score(int** scores, char* str_a, char* str_b, int i, int j){
    int match = 2;
    int mismatch = -1;
    int gap = -2;
    int s;
    int val;

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

void print_arr(int** arr, int len_a, int len_b){

    for(int i = 0; i < len_a; i++){
        for (int j = 0; j < len_b; j++){
            printf("%d\t", arr[i][j]);
        }
        printf("\n");
    }

}

int max2(int x, int y){
    return x > y ? x : y;
}

int max3(int x, int y, int z){
    return x > y ? (x > z ? x : z) : (y > z ? y : z);
}

char *get_input_str(char* filename, size_t size){
//The size is extended by the input with the value of the provisional
    FILE* fp = fopen(filename, "r");
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);//size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){
        str[len++]=ch;
        //printf("%c ", ch);
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}




