#include "IIR.h"

float IIR_FilterF(IIR_State* State, float sample) {
    State->xv[0] = State->xv[1];
    State->xv[1] = sample * State->Gain;
    State->yv[0] = State->yv[1];
    State->yv[1] = (State->xv[0] + State->xv[1]) + (State->yv[0] * State->Coeff1);
    State->Output = State->yv[1];
    return State->Output;
}
