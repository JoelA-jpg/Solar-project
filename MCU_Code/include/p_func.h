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
    if(output<1100){
        output = 1100;
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

//hystersis bang bang control for 360 degree servo
static inline uint16_t p_bangbang(uint16_t V_meas, int16_t K, int16_t tol){
    uint16_t output = 1600;
    
    //move towards center if outside tolerance
    if(V_meas>512+tol){
        output = output+K;
    }
    else if(V_meas<512-tol){
        output = output-K;
    } 
    return output;      
}


static inline uint16_t MPPT_pulse(uint16_t P_old, uint16_t P_new, uint16_t I_old,
    uint16_t I_new, uint16_t V_old, uint16_t V_new, uint16_t MPPT_pw, uint16_t step,
    uint16_t period){
    int16_t d_P = P_old - P_new;
    int16_t d_I = I_old - I_new;
    int16_t d_V = V_old - V_new;

    if(d_V){ // (d_V > 2 || d_V < -2)
        //int16_t ineq = d_P/d_V;
        if((d_P > 0 && d_V > 0) || (d_P < 0 && d_V < 0)){ // (d_P > 2*d_V) alt if ((d_P > 0 && d_V > 0) || (d_P < 0 && d_V < 0))
            MPPT_pw = MPPT_pw + step;
        }
        else if ((d_P > 0 && d_V < 0) || (d_P < 0 && d_V > 0)){ // (d_P < -2*d_V) alt if ((d_P > 0 && d_V < 0) || (d_P < 0 && d_V > 0))
            MPPT_pw = MPPT_pw - step;
        }
    }
    else{
        if (d_I > 0){
            MPPT_pw = MPPT_pw + step;
        }
        else if (d_I < 0){
            MPPT_pw = MPPT_pw - step;
        }
    }

    //prevent out of bounds
    if(MPPT_pw > (2*period)/3){
        MPPT_pw = (2*period)/3;
    }
    else if(MPPT_pw < period/3){
        MPPT_pw = period/3;
    }

    return MPPT_pw;
}


#endif
