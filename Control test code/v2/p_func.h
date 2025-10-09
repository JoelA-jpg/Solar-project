#ifndef P_FUNC_H
#define P_FUNC_H

static inline double p_func(double V_diff, double K){
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
#endif