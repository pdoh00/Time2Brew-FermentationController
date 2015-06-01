/* 
 * File:   PID.h
 * Author: aaron
 *
 * Created on March 28, 2015, 3:39 PM
 */

#ifndef PID_H
#define	PID_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "IIR.h"

    typedef struct {
        float lastError;
        float ITerm;
        float Output;
        float kp, ki, kd;
        float Setpoint;
        float outMax, outMin;
        float Input;
        float error;
        float DTerm;
        float D_AdaptiveBand;
        IIR_State D_Filter;
    } PID_CTX;

    void PID_Compute(PID_CTX *ctx);
    void PID_Initialize(PID_CTX *ctx);
    void PID_SetTunings(PID_CTX *ctx, float Kp, float Ki, float Kd, float D_FilterGain, float D_FilterCoeff, float D_AdaptiveBand);
    void PID_SetOutputLimits(PID_CTX *ctx, double Min, double Max);


#ifdef	__cplusplus
}
#endif

#endif	/* PID_H */

