#include <stdio.h>



int main(int argc, char* argv[]){

    int rows = 2;
    int cols = 2;

    int i_sum = 0;
    int k;
    int m;

    while (i_sum < cols+1){
        for (k = 0, m = i_sum; k <= i_sum;k++){
            printf("%d %d|", m, k);

            m--;
        }
        i_sum++;
        printf("\n");
    }
    while(i_sum < rows + 1){
        
    }




    return 0;
}