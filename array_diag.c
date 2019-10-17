#include <stdio.h>



int main(){
    int arr[3][4] = {{1,2,3,4}, {4,5,6,4}, {7,8,9,4}};
    int dim = 4;
    
    for (int k = 0; k < dim; k++){
        for (int j = 0; j <=k; j++){
            int i = k - j;
            printf("%d %d |", i, j);
        }
        printf("\n");

    }
    for (int k = dim - 2; k >=0; k--){
        for (int j = 0; j <=k; j++){
            int i = k - j;
            printf("%d %d |", dim - j - 1, dim - i - 1);
        }
        printf("\n");
    }

    int WIDTH = 3;
    int HEIGHT = 4;

    for(int k = 0 ; k <= WIDTH + HEIGHT - 2; k++ ) {
        for( int j = 0 ; j <= k ; j++ ) {
            int i = k - j;
            if( i < HEIGHT && j < WIDTH ) {
                printf("%d %d |", i, j);
            }

        }
        printf("\n");
    }

    return 0;
}