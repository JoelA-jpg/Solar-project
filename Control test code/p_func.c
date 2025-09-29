#include <stdlib.h>
#include <math.h>
#include "p_func.h"

//Generates a 1-2 ms pulse for servo motor.
double p_func(double V_diff, double K){
    double output = K * V_diff*0.4*500;//normalisation for 2.5V Ain, 500 us Aut
    //prevent out of bounds
    if(output>500){
        output = 500;
    }
    if(output<-500){
        output = -500;
    } 
    return 1.5 + 0.001*output;      
}