#include <stdio.h>

int main() {
    
    int a = 0;
    int b = 5;
    int c = 3;
    int d = 8;

    int i = 0;

    for(i = a; i<=b; i++){

        int j = 0;

        if(i==(a+b)/2) continue;

        for(j = c; j < d; j++){

            // Do stuff here
            printf("Hello World!");

            if(j % 2 == 0){
                printf("This is an even number.");
            }
        }
        
    }

    return 0;
}