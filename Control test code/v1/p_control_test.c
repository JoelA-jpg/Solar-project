#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "p_func.h"

int main(){
    const int N=20;
    double K = 2;
    double input[N];
    double output[N];

    for(int i=0;i<N;i++){
        input[i] = 2.5 + 1.5*sin(i);
        output[i] = p_func(input[i]-2.5, K);
    }

    for (int i = 0; i < N; i++) {
        printf("%10.5f  %10.5f\n", input[i], output[i]);
    }

    return 0;
}