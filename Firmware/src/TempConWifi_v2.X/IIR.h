/* 
 * File:   IIR.h
 * Author: aaron
 *
 * Created on December 30, 2014, 3:17 PM
 */

typedef struct {
    float Gain;
    float Coeff1;
    float xv[2];
    float yv[2];
    float Output;
} IIR_State;

float IIR_FilterF(IIR_State* State, float sample);

