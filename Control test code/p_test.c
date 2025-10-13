#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "p_func.h"

int main(){
    const int N=20;
    int16_t K = 5;
    double input[N];
    uint16_t input_ADC[N];
    uint16_t output[N];
    uint16_t pos = 1200;

    for(int i=0;i<N;i++){
        input[i] = 2.5 + 1.5*sin(i);
        input_ADC[i] =  (uint16_t)(input[i] * (1023.0 / 5.0) + 0.5);
        pos = p_func(pos, input_ADC[i], K);
        output[i] = pos;
    }

    for (int i = 0; i < N; i++) {
        printf("%10.5f  %10.5d  %10.5d\n", input[i], input_ADC[i], output[i]);
    }

    return 0;
}