#ifndef P_FUNC_H
#define P_FUNC_H

#include <stdint.h>

static inline uint16_t p_funcy(uint16_t pos, uint16_t V_meas, int16_t K){
    int16_t diff = (int16_t)(V_meas - 512);
    
    int32_t num   = (int32_t)2 * (int32_t)K * (int32_t)diff;
    int32_t delta = (num + (num >= 0 ? 2 : -2)) / 100;
    int32_t output = pos + delta;
    //prevent out of bounds
    if(output>1700){
        output = 1700;
    }
    if(output<800){
        output = 800;
    } 
    return (uint16_t)output;      
}

static inline uint16_t p_funcx(uint16_t pos, uint16_t V_meas, int16_t K){
    int16_t diff = (int16_t)(V_meas - 512);
    
    int32_t num   = (int32_t)2 * (int32_t)K * (int32_t)diff;
    int32_t delta = (num + (num >= 0 ? 2 : -2)) / 40;
    int32_t output = pos + delta;
    //prevent out of bounds
    if(output>2500){
        output = 2500;
    }
    if(output<500){
        output = 500;
    } 
    return (uint16_t)output;      
}

<<<<<<< HEAD
static inline uint16_t MMPT_pulse(uint16_t P_delta, uint16_t I_delta, uint16_t V_old, uint16_t V_new,  uint16_t MPPT_pw){

=======
//hystersis bang bang control for 360 degree servo
static inline uint16_t p_bangbang(uint16_t V_meas, int16_t K, int16_t tol){
    uint16_t output = 1500;
    
    //move towards center if outside tolerance
    if(V_meas>512+tol){
        output = 1500+K;
    }
    else if(V_meas<512-tol){
        output = 1500-K;
    } 
    return output;      
}


static inline uint16_t MMPT_pulse(uint16_t P_old, uint16_t P_new, uint16_t MPPT_pw){
       
>>>>>>> da6ae16e11ca6501f4761edf10c11baec6f3f5a0
}


#endif
