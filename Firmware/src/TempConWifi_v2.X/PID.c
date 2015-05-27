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

void PID_SetTunings(PID_CTX *ctx, double Kp, double Ki, double Kd) {
    ctx->kp = Kp;
    ctx->ki = Ki;
    ctx->kd = Kd;
}

void PID_Initialize(PID_CTX *ctx) {
    ctx->lastInput = ctx->Input;
    if (ctx->ki != 0) {
        ctx->ITerm = ctx->Output;
    } else {
        ctx->ITerm = 0;
    }
    if (ctx->ITerm > ctx->outMax) ctx->ITerm = ctx->outMax;
    else if (ctx->ITerm < ctx->outMin) ctx->ITerm = ctx->outMin;
}

void PID_Compute(PID_CTX *ctx) {
    /*Compute all the working error variables*/
    ctx->error = ctx->Setpoint - ctx->Input;
    ctx->ITerm += (ctx->ki * ctx->error);
    if (ctx->ITerm > ctx->outMax) ctx->ITerm = ctx->outMax;
    else if (ctx->ITerm < ctx->outMin) ctx->ITerm = ctx->outMin;
    float dInput = (ctx->Input - ctx->lastInput);

    /*Compute PID Output*/
    ctx->Output = (ctx->kp * ctx->error) + (ctx->ITerm) - ctx->kd * dInput;
    if (ctx->Output > ctx->outMax) ctx->Output = ctx->outMax;
    else if (ctx->Output < ctx->outMin) ctx->Output = ctx->outMin;

    /*Remember some variables for next time*/
    ctx->lastInput = ctx->Input;
}

