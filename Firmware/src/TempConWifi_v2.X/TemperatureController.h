/* 
 * File:   TemperatureController.h
 * Author: THORAXIUM
 *
 * Created on March 22, 2015, 11:38 PM
 */

#ifndef TEMPERATURECONTROLLER_H
#define	TEMPERATURECONTROLLER_H

#include "PID.h"
#include "FlashFS.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        SYSTEMMODE_IDLE = 0,
        SYSTEMMODE_Manual = 1,
        SYSTEMMODE_Profile = 2,
        SYSTEMMODE_ProfileEnded = 3
    } SYSTEMMODE;

    typedef enum {
        REGULATIONMODE_SimpleThreashold = 0,
        REGULATIONMODE_SimplePID = 1,
        REGULATIONMODE_ComplexThreshold = 2,
        REGULATIONMODE_ComplexPID = 3
    } REGULATIONMODE;

    typedef enum {
        PROBE_ASSIGNMENT_SENSOR = 0,
        PROBE_ASSIGNMENT_PROCESS = 1,
        PROBE_ASSIGNMENT_TARGET = 2
    } PROBE_ASSIGNMENT;

    typedef struct {
        unsigned long time;
        int Probe0;
        int Probe1;
        int SetPoint;
        char Output;
        unsigned char Relay;
    } TREND_RECORD;

    typedef struct {
        int StartTemperature, EndTemperature;
        unsigned long Duration;
    } PROFILE_STEP;

    typedef struct {
        unsigned char RegulationMode;
        unsigned char Probe0Assignment;
        unsigned char Probe1Assignment;
        unsigned long HeatMinTimeOn;
        unsigned long HeatMinTimeOff;
        unsigned long CoolMinTimeOn;
        unsigned long CoolMinTimeOff;
        float Process_Kp;
        float Process_Ki;
        float Process_Kd;
        float Target_Kp;
        float Target_Ki;
        float Target_Kd;
        float TargetOutput_Max;
        float TargetOutput_Min;
        float Process_D_FilterGain;
        float Process_D_FilterCoeff;
        float Process_D_AdaptiveBand;
        float Target_D_FilterGain;
        float Target_D_FilterCoeff;
        float Target_D_AdaptiveBand;
        int coolDifferential;
        int heatDifferential;
        int coolTransition;
        int heatTransition;
        unsigned int CheckSum;
    } EQUIPMENT_PROFILE;

    typedef struct {
        char lastOutput;
    } ON_OFF_REGULATION_STATE;

    typedef struct {
        unsigned long SystemTime;
        unsigned char SystemMode; //System Mode: 0 = Sensor Only, 1 = Manual Setpoint, 2=Profile Active
        char EquipmentName[64];
        char ActiveProfileName[64];
        unsigned char HeatRelay;
        unsigned char CoolRelay;
        signed char Output;
        PID_CTX TargetPID;
        PID_CTX ProcessPID;
        unsigned long TimeTurnedOn;
        unsigned long HeatWhenCanTurnOn;
        unsigned long HeatWhenCanTurnOff;
        unsigned long CoolWhenCanTurnOn;
        unsigned long CoolWhenCanTurnOff;
        unsigned int StepIdx;
        int StepTemperature;
        unsigned long StepTimeRemaining;
        unsigned long totalElapsedProfileTime;
        unsigned long totalProfileRunTime;
        unsigned char StepCount;
        PROFILE_STEP ProfileSteps[64];
        int Probe0Temperature;
        int Probe1Temperature;
        unsigned long ProfileStartTime;
        int ManualSetPoint;
        EQUIPMENT_PROFILE equipmentConfig;
        unsigned long signature;
        ON_OFF_REGULATION_STATE SimpleRegState;
    } MACHINE_STATE;

    extern MACHINE_STATE globalstate;
    extern ff_File ProfileTrendFile;

    int ExecuteProfile(const char *fname, char *msg);
    int LoadEquipmentProfile(const char *FileName, char *msg, EQUIPMENT_PROFILE *dest);
    int SetEquipmentProfile(const char *FileName, char *msg, MACHINE_STATE *dest);
    int TerminateProfile();
    int TruncateProfile(unsigned char *NewProfileData, int len, char *msg);
    unsigned long SecondsFromEpoch(int y, int m, int d, int hour, int minute, int second);
    int SetManualMode(int Setpoint, char *msg);
    void TemperatureController_Interrupt();
    void TrendBufferCommitt();
    int TemperatureController_Initialize();
    int rawReadTemp(int ProbeIDX);
    void InitializeRecoveryRecord();
    int LoadProfile(const char *FileName, char *msg, MACHINE_STATE *dest);
    void WriteRecoveryRecord(MACHINE_STATE *source);
    int GetProfileTotalDuration(const char *FileName, unsigned long *retVal);

#ifdef	__cplusplus
}
#endif

#endif	/* TEMPERATURECONTROLLER_H */

