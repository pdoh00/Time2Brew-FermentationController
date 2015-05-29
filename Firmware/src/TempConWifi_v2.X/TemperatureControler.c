#include <p33Exxxx.h>
#include <stdio.h>
#include <string.h>
#include "FlashFS.h"
#include "TemperatureController.h"
#include "SystemConfiguration.h"
#include "OneWireTemperature.h"
#include "Pack.h"
#include "Settings.h"
#include "PID.h"
#include "http_server.h"
#include "RTC.h"
#include "Http_API.h"
#include "fletcherChecksum.h"

ff_File ProfileTrendFile;

#define TRENDBUFFER_SIZE 16
TREND_RECORD trendBuffer[TRENDBUFFER_SIZE];
int trendBufferRead, trendBufferWrite;

MACHINE_STATE globalstate;
char scratch[256];

char isTemperatureController_Initialized = 0;

struct {
    unsigned long Profile_FileEntryAddress;
    unsigned long Equipment_FileEntryAddress;
    unsigned long Trend_FileEntryAddress;
    unsigned long CoolWhenCanTurnOff;
    unsigned long CoolWhenCanTurnOn;
    unsigned long HeatWhenCanTurnOff;
    unsigned long HeatWhenCanTurnOn;
    int ManualSetPoint;
    float ProcessPID_Output;
    unsigned long ProfileStartTime;
    unsigned char SystemMode;
    float TargetPID_Output;
    unsigned long TimeTurnedOn;
    unsigned int signature;
    unsigned int chkSum;
    unsigned int BootMode;
} recovery_record;

int rawReadTemp(int ProbeIDX) {
    int ret;

    union {
        int i;
        unsigned char ub[2];
    } temp;

    int retry = 3;
    ret = -32767;
    while (retry--) {
        if (OneWireReset(ProbeIDX) != 0) continue;
        OneWireWriteByte(ProbeIDX, 0xCC);
        OneWireWriteByte(ProbeIDX, 0xBE);
        if (OneWireReadByte(ProbeIDX, &temp.ub[0]) != 1) continue;
        if (OneWireReadByte(ProbeIDX, &temp.ub[1]) != 1) continue;
        ret = (int) ((float) temp.i * 0.625);
        if (ret < 1250 && ret>-550) break;
        ret = -32767;
    }

    //Send the command to start conversion...
    OneWireReset(ProbeIDX);
    OneWireWriteByte(ProbeIDX, 0xCC);
    OneWireWriteByte(ProbeIDX, 0x44);
    return ret;
}

int ReadTemperature(MACHINE_STATE *dest) {
    dest->Probe0Temperature = rawReadTemp(0);
    dest->Probe1Temperature = rawReadTemp(1);

    switch (dest->equipmentConfig.Probe0Assignment) {
        case 1:
            dest->ProcessPID.Input = dest->Probe0Temperature;
            break;
        case 2:
            dest->TargetPID.Input = dest->Probe0Temperature;
    }

    switch (dest->equipmentConfig.Probe1Assignment) {
        case 1:
            dest->ProcessPID.Input = dest->Probe1Temperature;
            break;
        case 2:
            dest->TargetPID.Input = dest->Probe1Temperature;
    }

    return 1;
}

int LoadProfile(const char *FileName, char *msg, MACHINE_STATE *dest) {
    unsigned char step[8];
    int bytesRead;
    ff_File Handle;
    ff_File *ptrHandle = &Handle;
    int res;

    Log("Loading Profile: \"%s\"\r\n", FileName);

    res = ff_OpenByFileName(ptrHandle, FileName, 0);
    if (res != FR_OK) {
        sprintf(msg, "LoadProfile Error: f_open res=%s", Translate_DRESULT(res));
        return res;
    }

    unsigned long bRemain = ptrHandle->Length;
    int stepIdx = 0;
    while (bRemain >= 8) {
        res = ff_Read(ptrHandle, step, 8, &bytesRead);
        if (res != FR_OK) {
            sprintf(msg, "LoadProfile Error: StepIdx=%d f_read res=%s", stepIdx, Translate_DRESULT(res));
            return res;
        }
        if (bytesRead != 8) {
            sprintf(msg, "LoadProfile Error: StepIdx=%d bytesRead !=8", stepIdx);
            return FR_ERR_OTHER;
        }
        bRemain -= 8;

        Unpack(step, "IIl", &dest->ProfileSteps[stepIdx].StartTemperature,
                &dest->ProfileSteps[stepIdx].EndTemperature,
                &dest->ProfileSteps[stepIdx].Duration);

        Log(" -StepIdx:%i,%i,%i,%ul\r\n", stepIdx,
                dest->ProfileSteps[stepIdx].StartTemperature,
                dest->ProfileSteps[stepIdx].EndTemperature,
                dest->ProfileSteps[stepIdx].Duration);

        stepIdx++;
    }
    dest->StepCount = stepIdx;
    Log("Done\r\n");
    return FR_OK;
}

int SetEquipmentProfile(const char *FileName, char *msg, MACHINE_STATE *dest) {
    EQUIPMENT_PROFILE temp;
    int res;
    res = LoadEquipmentProfile(FileName, msg, &temp);
    if (res != FR_OK) return res;

    DISABLE_INTERRUPTS;
    sprintf(dest->EquipmentName, "%s", FileName);
    memcpy((BYTE*) & dest->equipmentConfig, (BYTE *) & temp, sizeof (temp));
    PID_SetTunings(&dest->TargetPID,
            dest->equipmentConfig.Target_Kp,
            dest->equipmentConfig.Target_Ki,
            dest->equipmentConfig.Target_Kd);

    PID_SetTunings(&dest->ProcessPID,
            dest->equipmentConfig.Process_Kp,
            dest->equipmentConfig.Process_Ki,
            dest->equipmentConfig.Process_Kd);

    PID_SetOutputLimits(&dest->TargetPID,
            dest->equipmentConfig.TargetOutput_Min,
            dest->equipmentConfig.TargetOutput_Max);

    PID_SetOutputLimits(&dest->ProcessPID, -100, 100);

    ENABLE_INTERRUPTS;
    return FR_OK;
}

int LoadEquipmentProfile(const char *FileName, char *msg, EQUIPMENT_PROFILE *dest) {
    EQUIPMENT_PROFILE temp;
    int bRead;
    ff_File Handle;
    ff_File *ptrHandle = &Handle;
    int res;

    res = ff_OpenByFileName(ptrHandle, FileName, 0);
    if (res != FR_OK) return res;

    res = ff_Read(ptrHandle, (BYTE *) & temp, sizeof (EQUIPMENT_PROFILE), &bRead);
    if (res != FR_OK) return res;

    uint16_t chksum = fletcher16((BYTE *) & temp, sizeof (EQUIPMENT_PROFILE) - 2); //Checksum all but the last two bytes...

    if (temp.CheckSum != chksum) {
        sprintf(msg, "Error: Checksum Mismatch! Expected=%d Actual=%d", chksum, temp.CheckSum);
        return FR_ERR_OTHER;
    }

    memcpy((BYTE *) dest, (BYTE *) & temp, sizeof (EQUIPMENT_PROFILE));

    return FR_OK;
}

void WriteRecoveryRecord(MACHINE_STATE *source) {
    ff_File Profile, Equip;
    int res;
    if (globalstate.SystemMode == SYSTEMMODE_Profile) {
        res = ff_OpenByFileName(&Profile, source->ActiveProfileName, 0);
        if (res != FR_OK) Profile.FileEntryAddress = 0;
    } else {
        Profile.FileEntryAddress = 0;
        ProfileTrendFile.FileEntryAddress = 0;
    }

    res = ff_OpenByFileName(&Equip, source->EquipmentName, 0);
    if (res != FR_OK) {
        ff_OpenByFileName(&Equip, "equip.default", 0);
    }

    DISABLE_INTERRUPTS;
    recovery_record.CoolWhenCanTurnOff = source->CoolWhenCanTurnOff;
    recovery_record.CoolWhenCanTurnOn = source->CoolWhenCanTurnOn;
    recovery_record.HeatWhenCanTurnOff = source->HeatWhenCanTurnOff;
    recovery_record.HeatWhenCanTurnOn = source->HeatWhenCanTurnOn;
    recovery_record.ProcessPID_Output = source->ProcessPID.Output;
    recovery_record.TargetPID_Output = source->TargetPID.Output;
    recovery_record.TimeTurnedOn = source->TimeTurnedOn;
    ENABLE_INTERRUPTS;

    recovery_record.Profile_FileEntryAddress = Profile.FileEntryAddress;
    recovery_record.ManualSetPoint = source->ManualSetPoint;
    recovery_record.Equipment_FileEntryAddress = Equip.FileEntryAddress;
    recovery_record.Trend_FileEntryAddress = ProfileTrendFile.FileEntryAddress;
    recovery_record.ProfileStartTime = source->ProfileStartTime;
    recovery_record.SystemMode = source->SystemMode;
    recovery_record.signature = 0x1234;

    nvsRAM_Write((BYTE*) & recovery_record, 0, sizeof (recovery_record));

    uint16_t calcChecksum = fletcher16((BYTE*) & recovery_record, sizeof (recovery_record));
    nvsRAM_Write((BYTE*) & calcChecksum, sizeof (recovery_record), 2);

}

void InitializeRecoveryRecord() {
    ff_File equip;
    EQUIPMENT_PROFILE dummy;

    int res;

    res = LoadEquipmentProfile("equip.default", scratch, &dummy);
    if (res != FR_OK) {
        CreateDefaultEquipmentProfile();
        res = LoadEquipmentProfile("equip.default", scratch, &dummy);
        if (res != FR_OK) {
            Log("Error Opening Default Equipment Config; Err='%s' Msg='%s'", Translate_DRESULT(res), scratch);
            while (1);
        }
    }

    recovery_record.CoolWhenCanTurnOff = 0;
    recovery_record.CoolWhenCanTurnOn = 0;
    recovery_record.Equipment_FileEntryAddress = equip.FileEntryAddress;
    recovery_record.HeatWhenCanTurnOff = 0;
    recovery_record.HeatWhenCanTurnOn = 0;
    recovery_record.ManualSetPoint = 0;
    recovery_record.ProcessPID_Output = 0;
    recovery_record.ProfileStartTime = 0;
    recovery_record.Profile_FileEntryAddress = 0;
    recovery_record.SystemMode = SYSTEMMODE_IDLE;
    recovery_record.TargetPID_Output = 0;
    recovery_record.TimeTurnedOn = 0;
    recovery_record.signature = 0x1234;
    Log("----------------------\r\nInitialized Recovery Record\r\n---------------------------------------\r\n");
    Log(".CoolWhenCanTurnOff=%ul\r\n", recovery_record.CoolWhenCanTurnOff);
    Log(".CoolWhenCanTurnOn=%ul\r\n", recovery_record.CoolWhenCanTurnOn);
    Log(".Equipment_FileEntryAddress=%xl\r\n", recovery_record.Equipment_FileEntryAddress);
    Log(".HeatWhenCanTurnOff=%ul\r\n", recovery_record.HeatWhenCanTurnOff);
    Log(".HeatWhenCanTurnOn=%ul\r\n", recovery_record.HeatWhenCanTurnOn);
    Log(".ManualSetPoint=%i\r\n", recovery_record.ManualSetPoint);
    Log(".ProcessPID_Output=%f2\r\n", recovery_record.ProcessPID_Output);
    Log(".ProfileStartTime=%ul\r\n", recovery_record.ProfileStartTime);
    Log(".SystemMode=%ub\r\n", recovery_record.SystemMode);
    Log(".TargetPID_Output=%f2\r\n", recovery_record.TargetPID_Output);
    Log(".TimeTurnedOn=%ul\r\n", recovery_record.TimeTurnedOn);
    Log(".Trend_FileEntryAddress=%xl\r\n", recovery_record.Trend_FileEntryAddress);
    Log(".signature=%xi\r\n", recovery_record.signature);
    Log(".TargetPID_Output=%f2\r\n", recovery_record.TargetPID_Output);
    Log(".Profile_FileEntryAddress=%xl\r\n\r\n", recovery_record.Profile_FileEntryAddress);
    nvsRAM_Write((BYTE*) & recovery_record, 0, sizeof (recovery_record));

    uint16_t calcChecksum = fletcher16((BYTE*) & recovery_record, sizeof (recovery_record));
    nvsRAM_Write((BYTE*) & calcChecksum, sizeof (recovery_record), 2);
}

int RestoreRecoveryRecord() {
    MACHINE_STATE temp;
    uint16_t recordChecksum, calcChecksum;
    Log("---Reading Recovery Record\r\n");
    nvsRAM_Read((BYTE *) & recovery_record, 0, sizeof (recovery_record));
    nvsRAM_Read((BYTE *) & recordChecksum, sizeof (recovery_record), 2);
    calcChecksum = fletcher16((BYTE *) & recovery_record, sizeof (recovery_record));

    if (calcChecksum != recordChecksum) {
        Log("Bad Checksum Record=%xi Calculated=%xi\r\n", recordChecksum, calcChecksum);
        return 0;
    }

    if (recovery_record.signature != 0x1234) return 0;
    Log(".CoolWhenCanTurnOff=%ul\r\n", recovery_record.CoolWhenCanTurnOff);
    Log(".CoolWhenCanTurnOn=%ul\r\n", recovery_record.CoolWhenCanTurnOn);
    Log(".Equipment_FileEntryAddress=%xl\r\n", recovery_record.Equipment_FileEntryAddress);
    Log(".HeatWhenCanTurnOff=%ul\r\n", recovery_record.HeatWhenCanTurnOff);
    Log(".HeatWhenCanTurnOn=%ul\r\n", recovery_record.HeatWhenCanTurnOn);
    Log(".ManualSetPoint=%i\r\n", recovery_record.ManualSetPoint);
    Log(".ProcessPID_Output=%f2\r\n", recovery_record.ProcessPID_Output);
    Log(".ProfileStartTime=%ul\r\n", recovery_record.ProfileStartTime);
    Log(".SystemMode=%ub\r\n", recovery_record.SystemMode);
    Log(".TargetPID_Output=%f2\r\n", recovery_record.TargetPID_Output);
    Log(".TimeTurnedOn=%ul\r\n", recovery_record.TimeTurnedOn);
    Log(".Trend_FileEntryAddress=%xl\r\n", recovery_record.Trend_FileEntryAddress);
    Log(".signature=%xi\r\n", recovery_record.signature);
    Log(".TargetPID_Output=%f2\r\n", recovery_record.TargetPID_Output);
    Log(".Profile_FileEntryAddress=%xl\r\n", recovery_record.Profile_FileEntryAddress);

    //Restore the Profile and Equipment Names
    ff_File Profile, Equip;

    if (recovery_record.SystemMode == SYSTEMMODE_Profile) {
        if (recovery_record.Profile_FileEntryAddress) {
            if (ff_OpenByEntryAddress(&Profile, recovery_record.Profile_FileEntryAddress) != FR_OK) return 0;
        }
        if (recovery_record.Trend_FileEntryAddress) {
            if (ff_OpenByEntryAddress(&ProfileTrendFile, recovery_record.Trend_FileEntryAddress) != FR_OK) return 0;
        }
        sprintf(temp.ActiveProfileName, "%s", Profile.FileName);
        Log("Loading Active Profile: '%s'\r\n", Profile.FileName);
        if (LoadProfile(temp.ActiveProfileName, scratch, &temp) != FR_OK) return 0;
    } else {
        temp.ActiveProfileName[0] = 0;
    }

    if (recovery_record.Equipment_FileEntryAddress) {
        if (ff_OpenByEntryAddress(&Equip, recovery_record.Equipment_FileEntryAddress) != FR_OK) return 0;
        sprintf(temp.EquipmentName, "%s", Equip.FileName);
    } else {
        sprintf(temp.EquipmentName, "equip.default");
    }

    //Load the Equipment Profile
    Log("Loading Equipment Profile: '%s'\r\n", temp.EquipmentName);
    if (SetEquipmentProfile(temp.EquipmentName, scratch, &temp) != FR_OK) return 0;

    //Restore the other settings
    temp.CoolWhenCanTurnOff = recovery_record.CoolWhenCanTurnOff;
    temp.CoolWhenCanTurnOn = recovery_record.CoolWhenCanTurnOn;
    temp.HeatWhenCanTurnOff = recovery_record.HeatWhenCanTurnOff;
    temp.HeatWhenCanTurnOn = recovery_record.HeatWhenCanTurnOn;
    temp.ManualSetPoint = recovery_record.ManualSetPoint;
    temp.ProcessPID.Output = recovery_record.ProcessPID_Output;
    temp.ProfileStartTime = recovery_record.ProfileStartTime;
    temp.SystemMode = recovery_record.SystemMode;
    temp.TargetPID.Output = recovery_record.TargetPID_Output;
    temp.TimeTurnedOn = recovery_record.TimeTurnedOn;

    //Ensure that the relays are both off...
    temp.CoolRelay = 0;
    temp.HeatRelay = 0;
    //And the duty cycle timer is zero as well...
    temp.TimeTurnedOn = 0;

    //Get an initial temperature for the PID loops to work on
    ReadTemperature(&temp);
    Delay(1);
    ReadTemperature(&temp);

    //And initialize them so they are consistent...
    //the output and input values are required.
    PID_Initialize(&temp.ProcessPID);
    PID_Initialize(&temp.TargetPID);

    //Make it active!
    DISABLE_INTERRUPTS;
    memcpy((BYTE *) & globalstate, (BYTE *) & temp, sizeof (MACHINE_STATE));
    ENABLE_INTERRUPTS;
    return 1;
}

int SaveActiveProfile() {
    unsigned char step[8];
    int bytesWritten, res;
    ff_File Handle;
    ff_File *ptrHandle = &Handle;

    ff_Delete(globalstate.ActiveProfileName);
    res = ff_OpenByFileName(ptrHandle, globalstate.ActiveProfileName, 1);

    int x;

    int stepIdx = 0;
    for (stepIdx = 0; stepIdx < globalstate.StepCount; stepIdx++) {
        Pack(step, "IIl", globalstate.ProfileSteps[stepIdx].StartTemperature,
                globalstate.ProfileSteps[stepIdx].EndTemperature,
                globalstate.ProfileSteps[stepIdx].Duration);

        Log(" -StepIdx:%i,%i,%i,%ul\r\n", stepIdx,
                globalstate.ProfileSteps[stepIdx].StartTemperature,
                globalstate.ProfileSteps[stepIdx].EndTemperature,
                globalstate.ProfileSteps[stepIdx].Duration);

        Log("-----");
        for (x = 0; x < 8; x++) {
            Log("%xb ", step[x]);
        }
        Log("\r\n");
        res = ff_Append(ptrHandle, step, 8, &bytesWritten);
    }
    ff_UpdateLength(ptrHandle);
    Log("File Length=%ul\r\n", ptrHandle->Length);
    return 1;
}

int ExecuteProfile(const char *ProfileName, char *msg) {
    sprintf(scratch, "prfl.%s", ProfileName);
    if (LoadProfile(scratch, msg, &globalstate) != 1) {
        return -1;
    }
    sprintf(globalstate.ActiveProfileName, "inst.%s.%lu", ProfileName, globalstate.SystemTime);
    SaveActiveProfile();
    sprintf(scratch, "trnd.%s.%lu", ProfileName, globalstate.SystemTime);
    ff_Delete(scratch);
    ff_OpenByFileName(&ProfileTrendFile, scratch, 1);
    int idx;
    unsigned long totalProfileLength = 0;
    for (idx = 0; idx < globalstate.StepCount; idx++) {
        totalProfileLength += globalstate.ProfileSteps[idx].Duration;
    }
    Log("totalProfileLength-Seconds=%ul\r\n", totalProfileLength);
    totalProfileLength /= 60; //60 seconds per sample - total Sample Count
    Log("Total Sample Count=%ul\r\n", totalProfileLength);
    totalProfileLength *= 8; //Eight Bytes Per Sample - Total File Length;
    Log("Trend File Size=%ul\r\n", totalProfileLength);
    ff_Seek(&ProfileTrendFile, totalProfileLength, ff_SeekMode_Absolute);
    ff_UpdateLength(&ProfileTrendFile);
    ff_Seek(&ProfileTrendFile, 0, ff_SeekMode_Absolute);
    BuildProfileInstanceListing(ProfileName);

    DISABLE_INTERRUPTS;
    globalstate.SystemMode = SYSTEMMODE_Profile;
    globalstate.ProfileStartTime = globalstate.SystemTime;
    PID_Initialize(&globalstate.ProcessPID);
    PID_Initialize(&globalstate.TargetPID);
    ENABLE_INTERRUPTS;

    WriteRecoveryRecord(&globalstate);

    return 1;
}

int TerminateProfile(char *msg) {
    int idx;
    unsigned long totalLength = 0;
    BYTE OriginalMode = globalstate.SystemMode;
    DISABLE_INTERRUPTS;
    globalstate.SystemMode = SYSTEMMODE_IDLE;
    ENABLE_INTERRUPTS;

    if (OriginalMode == SYSTEMMODE_Profile) {
        PROFILE_STEP *current = &globalstate.ProfileSteps[globalstate.StepIdx];
        current->EndTemperature = globalstate.StepTemperature;
        current->Duration -= globalstate.StepTimeRemaining;
        globalstate.StepCount = globalstate.StepIdx + 1;
        for (idx = 0; idx < globalstate.StepCount; idx++) {
            totalLength += globalstate.ProfileSteps[idx].Duration;
        }
        totalLength /= 60;
        totalLength *= 8;
        ProfileTrendFile.Length = totalLength;
        ff_UpdateLength(&ProfileTrendFile);
        SaveActiveProfile();
    }

    globalstate.ManualSetPoint = 0;
    globalstate.ActiveProfileName[0] = 0;
    globalstate.ProfileStartTime = 0;
    globalstate.StepCount = 0;
    globalstate.StepIdx = 0;

    WriteRecoveryRecord(&globalstate);

    return 1;
}

int SetManualMode(int Setpoint, char *msg) {
    if (globalstate.SystemMode == SYSTEMMODE_Profile) {
        sprintf(msg, "Error: Profile is running");
        return -1;
    }

    DISABLE_INTERRUPTS;
    if (globalstate.SystemMode == SYSTEMMODE_IDLE) {
        PID_Initialize(&globalstate.ProcessPID);
        PID_Initialize(&globalstate.TargetPID);
        globalstate.SystemMode = SYSTEMMODE_Manual;
    }
    globalstate.ManualSetPoint = Setpoint;
    ENABLE_INTERRUPTS;

    WriteRecoveryRecord(&globalstate);

    return 1;
}

int TruncateProfile(unsigned char *NewProfileData, int len, char *msg) {
    if (globalstate.SystemMode != SYSTEMMODE_Profile) {
        sprintf(msg, "Error: Profile is not running");
        return -1;
    }

    DISABLE_INTERRUPTS;
    unsigned char *cursor = NewProfileData;
    PROFILE_STEP *current = &globalstate.ProfileSteps[globalstate.StepIdx];
    current->EndTemperature = globalstate.StepTemperature;
    current->Duration -= globalstate.StepTimeRemaining;
    int StepIdx = globalstate.StepIdx + 1;
    while (len >= 8) {
        Unpack(cursor, "IIl", &globalstate.ProfileSteps[StepIdx].StartTemperature,
                &globalstate.ProfileSteps[StepIdx].EndTemperature,
                &globalstate.ProfileSteps[StepIdx].Duration);
        StepIdx++;
        len -= 8;
    }
    globalstate.StepCount = StepIdx;
    ENABLE_INTERRUPTS;
    SaveActiveProfile();

    return 1;
}

void TrendBufferCommitt() {
    int bytesWritten;
    unsigned long trendFileTargetPosition = 0;


    if (globalstate.SystemMode == SYSTEMMODE_ProfileEnded) TerminateProfile(scratch);

    while (1) {
        DISABLE_INTERRUPTS;
        if (trendBufferRead == trendBufferWrite) {
            ENABLE_INTERRUPTS;
            return;
        }
        TREND_RECORD *current = &trendBuffer[trendBufferRead];
        trendBufferRead++;
        if (trendBufferRead >= TRENDBUFFER_SIZE) trendBufferRead = 0;
        ENABLE_INTERRUPTS;

        Pack((BYTE *) & scratch, "IIIBB",
                current->Probe0,
                current->Probe1,
                current->SetPoint,
                current->Output, current->Relay);


        trendFileTargetPosition = current->time * 8;

        Log("Record, Time=%ul P0=%i P1=%i Set=%i Out=%ub Relay=%xb | TargetPos=%xl Sector=%xl Offset=%xl",
                current->time,
                current->Probe0,
                current->Probe1,
                current->SetPoint,
                current->Output, current->Relay,
                trendFileTargetPosition,
                ProfileTrendFile.CurrentSector,
                ProfileTrendFile.SectorOffset);

        ff_Seek(&ProfileTrendFile, trendFileTargetPosition, ff_SeekMode_Absolute);
        ff_Append(&ProfileTrendFile, (BYTE *) scratch, 8, &bytesWritten);

        Log(" | NewSector=%xl NewOffset=%xl\r\n",
                ProfileTrendFile.CurrentSector,
                ProfileTrendFile.SectorOffset);


    }

}

void TemperatureController_Interrupt() {
    static unsigned long lastTimeSeconds = 0;
    static unsigned long lastTimeMinutes = 0;
    if (isTemperatureController_Initialized == 0) return;
    int CommandedTemperature;
    BYTE relay = 0;
    if (globalstate.SystemTime == lastTimeSeconds) return; //Nothing to do...

    lastTimeSeconds = globalstate.SystemTime;

    ReadTemperature(&globalstate);

    if (globalstate.SystemMode == SYSTEMMODE_IDLE || globalstate.SystemMode == SYSTEMMODE_ProfileEnded) {
        CommandedTemperature = -32767;
        globalstate.Output = 0;
        globalstate.HeatRelay = 0;
        globalstate.CoolRelay = 0;
        Log("TICK, Idle:\r\n");
        goto OnExit;
    } else if (globalstate.SystemMode == SYSTEMMODE_Profile) {
        Log("TICK, PRFL:");
        unsigned long elapsedTime = globalstate.SystemTime - globalstate.ProfileStartTime;
        unsigned long thisTimeMinutes = elapsedTime / 60;

        Log("et=%ul:", elapsedTime);
        char ProfileComplete = 1;
        int stepIdx = 0;
        for (stepIdx = 0; stepIdx < globalstate.StepCount; stepIdx++) {
            PROFILE_STEP *step = &globalstate.ProfileSteps[stepIdx];
            if (elapsedTime < step->Duration) {
                globalstate.StepIdx = stepIdx;
                globalstate.StepTimeRemaining = step->Duration - elapsedTime;
                float fractional = (float) elapsedTime / (float) step->Duration;
                float delta = (float) step->EndTemperature - (float) step->StartTemperature;
                delta *= fractional;
                delta += (float) step->StartTemperature;
                globalstate.StepTemperature = (int) delta;
                CommandedTemperature = globalstate.StepTemperature;
                Log("idx=%i,Remain=%ul,Temp=%i:", stepIdx, globalstate.StepTimeRemaining, globalstate.StepTemperature);
                ProfileComplete = 0;
                break;
            } else {
                elapsedTime -= globalstate.ProfileSteps[stepIdx].Duration;
            }
        }

        //Log Temperature HERE
        if (lastTimeMinutes != thisTimeMinutes) {
            lastTimeMinutes = thisTimeMinutes;
            trendBuffer[trendBufferWrite].Probe0 = globalstate.Probe0Temperature;
            trendBuffer[trendBufferWrite].Probe1 = globalstate.Probe1Temperature;
            trendBuffer[trendBufferWrite].SetPoint = CommandedTemperature;
            trendBuffer[trendBufferWrite].Output = globalstate.Output;
            trendBuffer[trendBufferWrite].time = thisTimeMinutes;
            relay = 0;
            if (globalstate.HeatRelay) relay |= 0b01;
            if (globalstate.CoolRelay) relay |= 0b10;
            trendBuffer[trendBufferWrite].Relay = relay;
            trendBufferWrite++;
            if (trendBufferWrite >= TRENDBUFFER_SIZE) trendBufferWrite = 0;
        }

        if (ProfileComplete) {
            Log("Complete:");
            globalstate.SystemMode = SYSTEMMODE_ProfileEnded;
            CommandedTemperature = -32767;
            globalstate.Output = 0;
            goto OnExit;
        }
    } else {
        Log("TICK, Man:");
        CommandedTemperature = globalstate.ManualSetPoint;
        Log("Temp=%i:", CommandedTemperature);
    }

    switch (globalstate.equipmentConfig.RegulationMode) {
        case REGULATIONMODE_ComplexPID:
            Log("Cpid:");
            globalstate.TargetPID.Setpoint = CommandedTemperature;
            PID_Compute(&globalstate.TargetPID);
            Log("TarOut=%f1:", globalstate.TargetPID.Output);
            globalstate.ProcessPID.Setpoint = globalstate.TargetPID.Output;
            PID_Compute(&globalstate.ProcessPID);
            Log("ProcOut=%f1:", globalstate.ProcessPID.Output);
            globalstate.Output = (char) globalstate.ProcessPID.Output;
            break;
        case REGULATIONMODE_SimplePID:
            Log("Spid:");
            globalstate.ProcessPID.Setpoint = CommandedTemperature;
            PID_Compute(&globalstate.ProcessPID);
            Log("ProcOut=%f1:", globalstate.ProcessPID.Output);
            globalstate.Output = (char) globalstate.ProcessPID.Output;
            break;
        case REGULATIONMODE_SimpleThreashold:
            Log("Simp:");
            if (globalstate.Output > 0) {
                globalstate.Output = 100;
                //Heat until we are above the setpoint by the threshold delta
                if (globalstate.ProcessPID.Input > (CommandedTemperature)) {
                    globalstate.Output = 0;
                }
            } else if (globalstate.Output < 0) {
                if (globalstate.ProcessPID.Input < (CommandedTemperature)) {
                    globalstate.Output = 0;
                }
            } else {
                if (globalstate.ProcessPID.Input > (CommandedTemperature + globalstate.equipmentConfig.ThresholdDelta)) {
                    globalstate.Output = -100;
                } else if (globalstate.ProcessPID.Input < (CommandedTemperature - globalstate.equipmentConfig.ThresholdDelta)) {
                    globalstate.Output = 100;
                }
            }
            Log("Out=%f1:", globalstate.Output);
            break;
        case REGULATIONMODE_ComplexThreshold:
            Log("Cplx:");
            globalstate.TargetPID.Setpoint = CommandedTemperature;
            PID_Compute(&globalstate.TargetPID);
            Log("TarOut=%f1:", globalstate.TargetPID.Output);

            if (globalstate.TargetPID.error > 0) {
                //The Target is too warm so we are in cooling mode...
                if (globalstate.Output < 0) {
                    //Cooling is on...so we need to look for when to turn it off
                    if (globalstate.ProcessPID.Input < (globalstate.TargetPID.Output - globalstate.equipmentConfig.ThresholdDelta)) {
                        globalstate.Output = 0;
                    } else {
                        globalstate.Output = -100;
                    }
                } else {
                    if (globalstate.ProcessPID.Input > (globalstate.TargetPID.Output + globalstate.equipmentConfig.ThresholdDelta)) {
                        globalstate.Output = -100;
                    } else {
                        globalstate.Output = 0;
                    }
                }
            } else {
                //The Target is too cold so we are in heating mode...
                if (globalstate.Output > 0) {
                    //Heating is on...so we need to look for when to turn it off
                    if (globalstate.ProcessPID.Input > (globalstate.TargetPID.Output + globalstate.equipmentConfig.ThresholdDelta)) {
                        globalstate.Output = 0;
                    } else {
                        globalstate.Output = 100;
                    }
                } else {
                    if (globalstate.ProcessPID.Input < (globalstate.TargetPID.Output - globalstate.equipmentConfig.ThresholdDelta)) {
                        globalstate.Output = 100;
                    } else {
                        globalstate.Output = 0;
                    }
                }
            }
            Log("Out=%f1:", globalstate.Output);
            break;
    }

    if (globalstate.Output > 0) {
        if (globalstate.CoolRelay) {
            //Oops - The controller is calling for heat but we are currently cooling!
            if (globalstate.SystemTime > globalstate.CoolWhenCanTurnOff) {
                Log("OKOff:");
                globalstate.HeatRelay = 0;
                globalstate.CoolRelay = 0;
                globalstate.CoolWhenCanTurnOn = globalstate.SystemTime + globalstate.equipmentConfig.CoolMinTimeOff;
            } else {
                Log("WaitOff:");
            }
        } else if (globalstate.HeatRelay) { //Currently Heating...
            float TimeOn = globalstate.SystemTime - globalstate.TimeTurnedOn;
            float Duty = 100 * (TimeOn / (TimeOn + globalstate.equipmentConfig.HeatMinTimeOff));
            Log("Duty=%f1:", Duty);
            if (Duty > globalstate.Output) {
                if (globalstate.SystemTime > globalstate.HeatWhenCanTurnOff) {
                    Log("OKOff:");
                    globalstate.HeatRelay = 0;
                    globalstate.CoolRelay = 0;
                    globalstate.HeatWhenCanTurnOn = globalstate.SystemTime + globalstate.equipmentConfig.HeatMinTimeOff;
                } else {
                    Log("WaitOff:");
                }
            } else {
                Log("More:");
            }
        } else {
            float EstOnTime = globalstate.equipmentConfig.HeatMinTimeOn * 2;
            if (globalstate.Output < 99) {
                EstOnTime = globalstate.equipmentConfig.HeatMinTimeOff / ((100 - globalstate.Output)*0.01);
            }
            Log("EstOnTm:%f1", EstOnTime);
            if (EstOnTime >= globalstate.equipmentConfig.HeatMinTimeOn) {
                if (globalstate.SystemTime > globalstate.HeatWhenCanTurnOn) {
                    Log("OKOn:");
                    globalstate.TimeTurnedOn = globalstate.SystemTime;
                    globalstate.HeatRelay = 1;
                    globalstate.CoolRelay = 0;
                    globalstate.HeatWhenCanTurnOff = globalstate.SystemTime + globalstate.equipmentConfig.HeatMinTimeOn;
                } else {
                    Log("WaitOn:");
                }
            } else {
                Log("NotMinOn:");
            }
        }
    } else if (globalstate.Output < 0) {
        if (globalstate.HeatRelay) {
            //Oops - The controller is calling for COOL but we are currently Heating!
            if (globalstate.SystemTime > globalstate.HeatWhenCanTurnOff) {
                Log("OKOff:");
                globalstate.HeatRelay = 0;
                globalstate.CoolRelay = 0;
                globalstate.HeatWhenCanTurnOn = globalstate.SystemTime + globalstate.equipmentConfig.HeatMinTimeOff;
            } else {
                Log("WaitOff:");
            }
        } else if (globalstate.CoolRelay) { //Currently Cooling...
            float TotalTimeOn = globalstate.SystemTime - globalstate.TimeTurnedOn;
            float Duty = 100 * (TotalTimeOn / (TotalTimeOn + globalstate.equipmentConfig.CoolMinTimeOff));
            Log("Duty=%f1:", Duty);
            if (Duty > -globalstate.Output) {
                if (globalstate.SystemTime > globalstate.CoolWhenCanTurnOff) {
                    Log("OKOff:");
                    globalstate.HeatRelay = 0;
                    globalstate.CoolRelay = 0;
                    globalstate.CoolWhenCanTurnOn = globalstate.SystemTime + globalstate.equipmentConfig.CoolMinTimeOff;
                } else {
                    Log("WaitOff:");
                }
            } else {
                Log("More:");
            }
        } else {
            float EstOnTime = globalstate.equipmentConfig.CoolMinTimeOn;
            if (globalstate.Output > -99) {
                EstOnTime = globalstate.equipmentConfig.CoolMinTimeOff / ((100 + globalstate.Output)*0.01);
            }
            Log("EstOnTm:%f1", EstOnTime);
            if (EstOnTime >= globalstate.equipmentConfig.CoolMinTimeOn) {
                if (globalstate.SystemTime > globalstate.CoolWhenCanTurnOn) {
                    Log("OKOn:");
                    globalstate.TimeTurnedOn = globalstate.SystemTime;
                    globalstate.CoolRelay = 1;
                    globalstate.HeatRelay = 0;
                    globalstate.CoolWhenCanTurnOff = globalstate.SystemTime + globalstate.equipmentConfig.CoolMinTimeOn;
                } else {
                    Log("WaitOn:");
                }
            } else {
                Log("NotMinOn:");
            }
        }
    } else {
        if (globalstate.CoolRelay) {
            if (globalstate.SystemTime > globalstate.CoolWhenCanTurnOff) {
                Log("OKOff:");
                globalstate.HeatRelay = 0;
                globalstate.CoolRelay = 0;
                globalstate.CoolWhenCanTurnOn = globalstate.SystemTime + globalstate.equipmentConfig.CoolMinTimeOff;
            } else {
                Log("WaitOff:");
            }
        }

        if (globalstate.HeatRelay) {
            if (globalstate.SystemTime > globalstate.HeatWhenCanTurnOff) {
                Log("OKOff:");
                globalstate.HeatRelay = 0;
                globalstate.CoolRelay = 0;
                globalstate.HeatWhenCanTurnOn = globalstate.SystemTime + globalstate.equipmentConfig.HeatMinTimeOff;
            } else {
                Log("WaitOff:");
            }
        }

    }


OnExit:
    TemperatureControllerIsAlive = 1;
    SET_HEAT(globalstate.HeatRelay);
    SET_COOL(globalstate.CoolRelay);

    if (globalstate.SystemMode != SYSTEMMODE_IDLE) {
        if (globalstate.HeatRelay) {
            Log("HEAT:\r\n");
        } else if (globalstate.CoolRelay) {
            Log("COOL:\r\n");
        } else {
            Log("\r\n");
        }
    }

}

int TemperatureController_Initialize() {
    if (isTemperatureController_Initialized == 1) return 1;
    if (RestoreRecoveryRecord() == 0) {
        //There was no recovery record or it was corrupted - default settings...
        InitializeRecoveryRecord();
        RestoreRecoveryRecord();
    }

    //Set the system time...
    globalstate.SystemTime = RTC_GetTime();
    trendBufferRead = 0;
    trendBufferWrite = 0;
    isTemperatureController_Initialized = 1;

    return 1;
}

