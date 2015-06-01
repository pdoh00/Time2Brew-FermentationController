#include "PID.h"

void PID_SetOutputLimits(PID_CTX *ctx, double Min, double Max) {
    if (Min > Max) return;
    ctx->outMin = Min;
    ctx->outMax = Max;

    if (ctx->Output > ctx->outMax) ctx->Output = ctx->outMax;
    else if (ctx->Output < ctx->outMin) ctx->Output = ctx->outMin;

    if (ctx->ITerm > ctx->outMax) ctx->ITerm = ctx->outMax;
    else if (ctx->ITerm < ctx->outMin) ctx->ITerm = ctx->outMin;
}

void PID_SetTunings(PID_CTX *ctx, float Kp, float Ki, float Kd, float D_FilterGain, float D_FilterCoeff, float D_AdaptiveBand) {
    ctx->kp = Kp;
    ctx->ki = Ki;
    ctx->kd = Kd;
    ctx->D_Filter.Coeff1 = D_FilterCoeff;
    ctx->D_Filter.Gain = 1.0 / D_FilterGain;
    ctx->D_AdaptiveBand = D_AdaptiveBand;
}

void PID_Initialize(PID_CTX *ctx) {
    if (ctx->ki != 0) {
        ctx->ITerm = ctx->Output;
    } else {
        ctx->ITerm = 0;
    }
    if (ctx->ITerm > ctx->outMax) ctx->ITerm = ctx->outMax;
    else if (ctx->ITerm < ctx->outMin) ctx->ITerm = ctx->outMin;
    ctx->D_Filter.xv[0] = 0;
    ctx->D_Filter.xv[1] = 0;
    ctx->D_Filter.yv[0] = 0;
    ctx->D_Filter.yv[1] = 0;
    ctx->D_Filter.Output = 0;
}

void PID_Compute(PID_CTX *ctx) {
    float porportional;
    /*Compute all the working error variables*/
    ctx->error = ctx->Setpoint - ctx->Input;
    porportional = ctx->error * ctx->kp;

    if (porportional > ctx->outMax) {
        porportional = ctx->outMax;
        ctx->ITerm = 0;
        ctx->DTerm = 0;
    } else if (porportional < ctx->outMin) {
        porportional = ctx->outMin;
        ctx->ITerm = 0;
        ctx->DTerm = 0;
    } else {
        ctx->ITerm += (ctx->ki * ctx->error);
        if (ctx->ITerm > ctx->outMax) ctx->ITerm = ctx->outMax;
        else if (ctx->ITerm < ctx->outMin) ctx->ITerm = ctx->outMin;

        //Calculate the Derivative Input value by getting the slope of the trend data from one minute ago
        //to now.

        float thisError = IIR_FilterF(&(ctx->D_Filter), (float) ctx->error);

        float dInput = (ctx->lastError - thisError);
        ctx->lastError = thisError;

        ctx->DTerm = ctx->kd*dInput;
        if (ctx->DTerm > ctx->outMax) ctx->DTerm = ctx->outMax;
        else if (ctx->DTerm < ctx->outMin) ctx->DTerm = ctx->outMin;

        float absError;
        if (ctx->error > 0.0) absError = ctx->error;
        else absError = -(ctx->error);

        if (absError > (ctx->D_AdaptiveBand * 2)) { //more than 5C away? Derivative braking is zero
            ctx->DTerm = 0.0;
        } else if (absError > ctx->D_AdaptiveBand) { //Feather in derivative braking from 5C to 2.5C away
            absError -= ctx->D_AdaptiveBand;
            absError = (ctx->D_AdaptiveBand - absError) / ctx->D_AdaptiveBand;
            ctx->DTerm *= absError;
        } else { //Closer than 2.5C means apply full derivative braking
            //do nothing
        }
    }

    /*Compute PID Output*/
    ctx->Output = porportional + (ctx->ITerm) + ctx->DTerm;
    if (ctx->Output > ctx->outMax) ctx->Output = ctx->outMax;
    else if (ctx->Output < ctx->outMin) ctx->Output = ctx->outMin;
}

