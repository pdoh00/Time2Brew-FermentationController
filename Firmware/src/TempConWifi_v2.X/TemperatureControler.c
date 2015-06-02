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
        if (OneWireIsBusShorted()) continue;
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

int GetProfileTotalDuration(const char *FileName, unsigned long *retVal) {
    unsigned char step[8];
    int bytesRead;
    ff_File Handle;
    ff_File *ptrHandle = &Handle;
    int res;

    res = ff_OpenByFileName(ptrHandle, FileName, 0);
    if (res != FR_OK) {
        return res;
    }

    unsigned long totalProfileRunTime = 0;
    int temp1, temp2;
    unsigned long StepDuration;
    unsigned long bRemain = ptrHandle->Length;
    int stepIdx = 0;
    while (bRemain >= 8) {
        res = ff_Read(ptrHandle, step, 8, &bytesRead);
        if (res != FR_OK) {
            return res;
        }
        if (bytesRead != 8) {
            return FR_ERR_OTHER;
        }
        bRemain -= 8;

        Unpack(step, "IIl",
                &temp1,
                &temp2,
                &StepDuration);

        totalProfileRunTime += StepDuration;
        stepIdx++;
    }
    *(retVal) = totalProfileRunTime;

    return FR_OK;
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

    dest->totalProfileRunTime = 0;
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

        dest->totalProfileRunTime += dest->ProfileSteps[stepIdx].Duration;
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
            dest->equipmentConfig.Target_Kd,
            dest->equipmentConfig.Target_D_FilterGain,
            dest->equipmentConfig.Target_D_FilterCoeff,
            dest->equipmentConfig.Target_D_AdaptiveBand);

    PID_SetTunings(&dest->ProcessPID,
            dest->equipmentConfig.Process_Kp,
            dest->equipmentConfig.Process_Ki,
            dest->equipmentConfig.Process_Kd,
            dest->equipmentConfig.Process_D_FilterGain,
            dest->equipmentConfig.Process_D_FilterCoeff,
            dest->equipmentConfig.Process_D_AdaptiveBand);

    PID_SetOutputLimits(&dest->TargetPID, -100, 100);
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
    Log(".signature=%xi\r\n", recovery_record.signature);
    Log(".TargetPID_Output=%f2\r\n", recovery_record.TargetPID_Output);
    Log(".Profile_FileEntryAddress=%xl\r\n\r\n", recovery_record.Profile_FileEntryAddress);
    nvsRAM_Write((BYTE*) & recovery_record, 0, sizeof (recovery_record));

    uint16_t calcChecksum = fletcher16((BYTE*) & recovery_record, sizeof (recovery_record));
    nvsRAM_Write((BYTE*) & calcChecksum, sizeof (recovery_record), 2);
}

int RestoreRecoveryRecord() {
    char filename[128];
    int res;
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
    Log("   .CoolWhenCanTurnOff=%ul\r\n", recovery_record.CoolWhenCanTurnOff);
    Log("   .CoolWhenCanTurnOn=%ul\r\n", recovery_record.CoolWhenCanTurnOn);
    Log("   .Equipment_FileEntryAddress=%xl\r\n", recovery_record.Equipment_FileEntryAddress);
    Log("   .HeatWhenCanTurnOff=%ul\r\n", recovery_record.HeatWhenCanTurnOff);
    Log("   .HeatWhenCanTurnOn=%ul\r\n", recovery_record.HeatWhenCanTurnOn);
    Log("   .ManualSetPoint=%i\r\n", recovery_record.ManualSetPoint);
    Log("   .ProcessPID_Output=%f2\r\n", recovery_record.ProcessPID_Output);
    Log("   .ProfileStartTime=%ul\r\n", recovery_record.ProfileStartTime);
    Log("   .SystemMode=%ub\r\n", recovery_record.SystemMode);
    Log("   .TargetPID_Output=%f2\r\n", recovery_record.TargetPID_Output);
    Log("   .TimeTurnedOn=%ul\r\n", recovery_record.TimeTurnedOn);
    Log("   .signature=%xi\r\n", recovery_record.signature);
    Log("   .TargetPID_Output=%f2\r\n", recovery_record.TargetPID_Output);
    Log("   .Profile_FileEntryAddress=%xl\r\n", recovery_record.Profile_FileEntryAddress);

    //Restore the Profile and Equipment Names
    ff_File Profile, Equip;

    if (recovery_record.SystemMode == SYSTEMMODE_Profile) {
        Log("Finding Active Profile File @ Entry Address: %xl...", recovery_record.Profile_FileEntryAddress);
        if (recovery_record.Profile_FileEntryAddress) {
            res = ff_OpenByEntryAddress(&Profile, recovery_record.Profile_FileEntryAddress);
            if (res != FR_OK) {
                Log("FAIL: res=%s", Translate_DRESULT(res));
                return 0;
            }
        } else {
            Log("Error: Entry Address is Blank\r\n");
            return 0;
        }

        sprintf(temp.ActiveProfileName, "%s", Profile.FileName);
        Log("Loading Active Profile: '%s'...", Profile.FileName);
        res = LoadProfile(temp.ActiveProfileName, scratch, &temp);
        if (res != FR_OK) {
            Log("FAIL: res=%s\r\n", Translate_DRESULT(res));
            return 0;
        }

        sprintf(filename, "trnd.%s", &Profile.FileName[5]);
        Log("Opening Trend Record '%s'...", filename);
        res = ff_OpenByFileName(&ProfileTrendFile, filename, 0);
        if (res != FR_OK) {
            Log("FAIL: res=%s\r\n", Translate_DRESULT(res));
            return 0;
        }
    } else {
        temp.ActiveProfileName[0] = 0;
    }

    if (recovery_record.Equipment_FileEntryAddress) {
        Log("Loading Equipment Profile @ EntryAddress:%xl...", recovery_record.Equipment_FileEntryAddress);
        res = ff_OpenByEntryAddress(&Equip, recovery_record.Equipment_FileEntryAddress);
        if (res != FR_OK) {
            Log("FAIL: res=%s\r\n", Translate_DRESULT(res));
            return 0;
        }
        sprintf(temp.EquipmentName, "%s", Equip.FileName);
    } else {
        Log("Loading Deafult Equipment Profile...");
        sprintf(temp.EquipmentName, "equip.default");
    }

    //Load the Equipment Profile
    Log("Filename='%s'...", temp.EquipmentName);
    res = SetEquipmentProfile(temp.EquipmentName, scratch, &temp);
    if (res != FR_OK) {
        Log("FAIL: res=%s\r\n", Translate_DRESULT(res));
        return 0;
    }

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
    Log("Recoery Success!\r\n");
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
    int res;
    sprintf(scratch, "prfl.%s", ProfileName);
    if (LoadProfile(scratch, msg, &globalstate) != 1) {
        return -1;
    }

    sprintf(globalstate.ActiveProfileName, "inst.%s.%lu", ProfileName, globalstate.SystemTime);
    SaveActiveProfile();

    sprintf(scratch, "trnd.%s.%lu", ProfileName, globalstate.SystemTime);
    ff_Delete(scratch);
    res = ff_OpenByFileName(&ProfileTrendFile, scratch, 1);
    if (res != FR_OK) {
        sprintf(msg, "Error: unable to allocate profile trend record='%s' res=%s", scratch, Translate_DRESULT(res));
        return -1;
    }
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
    BYTE OriginalMode = globalstate.SystemMode;
    DISABLE_INTERRUPTS;
    globalstate.SystemMode = SYSTEMMODE_IDLE;
    ENABLE_INTERRUPTS;

    if (OriginalMode == SYSTEMMODE_Profile) {
        PROFILE_STEP *current = &globalstate.ProfileSteps[globalstate.StepIdx];
        current->EndTemperature = globalstate.StepTemperature;
        current->Duration -= globalstate.StepTimeRemaining;
        globalstate.StepCount = globalstate.StepIdx + 1;
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
    BYTE tbcBuff[8];
    int bytesWritten;
    unsigned long trendFileTargetPosition = 0;


    if (globalstate.SystemMode == SYSTEMMODE_ProfileEnded) TerminateProfile(scratch);

    while (1) {
        DISABLE_INTERRUPTS;
        if (trendBufferRead == trendBufferWrite) {
            ENABLE_INTERRUPTS;
            return;
        }
        TREND_RECORD *thisRecord = &trendBuffer[trendBufferRead];
        trendBufferRead++;
        if (trendBufferRead >= TRENDBUFFER_SIZE) trendBufferRead = 0;
        ENABLE_INTERRUPTS;

        Pack(tbcBuff, "IIIBB",
                thisRecord->Probe0,
                thisRecord->Probe1,
                thisRecord->SetPoint,
                thisRecord->Output, thisRecord->Relay);


        trendFileTargetPosition = thisRecord->time * 8;

        Log("Record,%ul,%i,%i,%i,%ub,%xb,%xl,%xl,%xl",
                thisRecord->time,
                thisRecord->Probe0,
                thisRecord->Probe1,
                thisRecord->SetPoint,
                thisRecord->Output, thisRecord->Relay,
                trendFileTargetPosition,
                ProfileTrendFile.CurrentSector,
                ProfileTrendFile.SectorOffset);

        if (ProfileTrendFile.Position != trendFileTargetPosition) {
            ff_Seek(&ProfileTrendFile, trendFileTargetPosition, ff_SeekMode_Absolute);
        }
        ff_Append(&ProfileTrendFile, tbcBuff, 8, &bytesWritten);

        Log(",%xl,%xl,%xl\r\n",
                ProfileTrendFile.Position,
                ProfileTrendFile.CurrentSector,
                ProfileTrendFile.SectorOffset);
    }

}

void FixedOffTimePWM(MACHINE_STATE *state) {
    if (state->Output > 0) {
        if (state->CoolRelay) {
            //Oops - The controller is calling for heat but we are currently cooling!
            if (state->SystemTime > state->CoolWhenCanTurnOff) {
                state->HeatRelay = 0;
                state->CoolRelay = 0;
                state->CoolWhenCanTurnOn = state->SystemTime + state->equipmentConfig.CoolMinTimeOff;
                Log(",COOLTURNEDOFF");
            } else {
                Log(",WAITCOOLOFF");
            }
        } else if (state->HeatRelay) { //Currently Heating...
            float TimeOn = state->SystemTime - state->TimeTurnedOn;
            float Duty = 100 * (TimeOn / (TimeOn + state->equipmentConfig.HeatMinTimeOff));
            if (Duty > state->Output) {
                if (state->SystemTime > state->HeatWhenCanTurnOff) {
                    state->HeatRelay = 0;
                    state->CoolRelay = 0;
                    state->HeatWhenCanTurnOn = state->SystemTime + state->equipmentConfig.HeatMinTimeOff;
                    Log(",HEATTURNEDOFF");
                } else {
                    Log(",WAITHEATOFF");
                }
            } else {
                Log(",HEATING,%f2", Duty);
            }
        } else {
            float EstOnTime = state->equipmentConfig.HeatMinTimeOn;
            if (state->Output < 99) {
                float CmdDuty = (float) state->Output / (float) 100;
                EstOnTime = (CmdDuty * state->equipmentConfig.HeatMinTimeOff) / (1 - CmdDuty);
            }
            if (EstOnTime >= state->equipmentConfig.HeatMinTimeOn) {
                if (state->SystemTime > state->HeatWhenCanTurnOn) {
                    state->TimeTurnedOn = state->SystemTime;
                    state->HeatRelay = 1;
                    state->CoolRelay = 0;
                    state->HeatWhenCanTurnOff = state->SystemTime + state->equipmentConfig.HeatMinTimeOn;
                    Log(",HEATTURNEDON");
                } else {
                    Log(",WAITHEATON");
                }
            } else {
                Log(",HEAT_TOOSHORT,%f1", EstOnTime);
            }
        }
    } else if (state->Output < 0) {
        if (state->HeatRelay) {
            //Oops - The controller is calling for COOL but we are currently Heating!
            if (state->SystemTime > state->HeatWhenCanTurnOff) {
                state->HeatRelay = 0;
                state->CoolRelay = 0;
                state->HeatWhenCanTurnOn = state->SystemTime + state->equipmentConfig.HeatMinTimeOff;
                Log(",HEATTURNEDOFF");
            } else {
                Log(",WAITHEATOFF");
            }
        } else if (state->CoolRelay) { //Currently Cooling...
            float TotalTimeOn = state->SystemTime - state->TimeTurnedOn;
            float Duty = 100 * (TotalTimeOn / (TotalTimeOn + state->equipmentConfig.CoolMinTimeOff));
            if (Duty > -state->Output) {
                if (state->SystemTime > state->CoolWhenCanTurnOff) {
                    state->HeatRelay = 0;
                    state->CoolRelay = 0;
                    state->CoolWhenCanTurnOn = state->SystemTime + state->equipmentConfig.CoolMinTimeOff;
                    Log(",COOLTURNEDOFF");
                } else {
                    Log(",COOLWAITOFF");
                }
            } else {
                Log(",COOLING,%f2:", Duty);
            }
        } else {
            float EstOnTime = state->equipmentConfig.CoolMinTimeOn * 2;
            if (state->Output > -99) {
                float CmdDuty = (float) state->Output / (float) (-100);
                EstOnTime = (CmdDuty * state->equipmentConfig.CoolMinTimeOff) / (1 - CmdDuty);
            }
            if (EstOnTime >= state->equipmentConfig.CoolMinTimeOn) {
                if (state->SystemTime > state->CoolWhenCanTurnOn) {
                    state->TimeTurnedOn = state->SystemTime;
                    state->CoolRelay = 1;
                    state->HeatRelay = 0;
                    state->CoolWhenCanTurnOff = state->SystemTime + state->equipmentConfig.CoolMinTimeOn;
                    Log(",COOLTURNEDON");
                } else {
                    Log(",COOLWAITON");
                }
            } else {
                Log(",COOL_TOOSHORT,%f1", EstOnTime);
            }
        }
    } else {
        if (state->CoolRelay) {

            if (state->SystemTime > state->CoolWhenCanTurnOff) {
                state->HeatRelay = 0;
                state->CoolRelay = 0;
                state->CoolWhenCanTurnOn = state->SystemTime + state->equipmentConfig.CoolMinTimeOff;
                Log(",COOLTURNEDOFF");
            } else {
                Log(",WAITCOOLOFF");
            }
        }
        if (state->HeatRelay) {

            if (state->SystemTime > state->HeatWhenCanTurnOff) {
                state->HeatRelay = 0;
                state->CoolRelay = 0;
                state->HeatWhenCanTurnOn = state->SystemTime + state->equipmentConfig.HeatMinTimeOff;
                Log(",HEATTURNEDOFF");
            } else {
                Log(",WAITHEATOFF");
            }
        }
    }
}

void TemperatureController_Interrupt() {
    static int OnOffMode = -1;
    static unsigned long lastTimeSeconds = 0;
    static unsigned long lastTimeMinutes = 0;
    if (isTemperatureController_Initialized == 0) return;
    int CommandedTemperature;
    float ThresholdTemperature;
    BYTE relay = 0;
    if (globalstate.SystemTime == lastTimeSeconds) return; //Nothing to do...

    lastTimeSeconds = globalstate.SystemTime;

    ReadTemperature(&globalstate);


    Log("TICK,%ul,%i,%i", globalstate.SystemTime, globalstate.Probe0Temperature, globalstate.Probe1Temperature);

    if (globalstate.Probe0Temperature < -250 || globalstate.Probe1Temperature <-250 || globalstate.Probe0Temperature > 1250 || globalstate.Probe1Temperature > 1250) {
        Log(",INVALID_TEMP\r\n");
        return;
    }

    if (globalstate.SystemMode == SYSTEMMODE_IDLE || globalstate.SystemMode == SYSTEMMODE_ProfileEnded) {
        CommandedTemperature = -32767;
        globalstate.Output = 0;
        globalstate.HeatRelay = 0;
        globalstate.CoolRelay = 0;
        Log(",Idle");
        goto OnExit;
    } else if (globalstate.SystemMode == SYSTEMMODE_Profile) {

        unsigned long elapsedTime = globalstate.SystemTime - globalstate.ProfileStartTime;
        globalstate.totalElapsedProfileTime = elapsedTime;

        unsigned long thisTimeMinutes = elapsedTime / 60;
        //unsigned long thisTimeMinutes = elapsedTime;

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

        Log(",PRFL,%ul,%ui,%ul,%i", elapsedTime, globalstate.StepIdx, globalstate.StepTimeRemaining, globalstate.StepTemperature);

        if (ProfileComplete) {
            globalstate.SystemMode = SYSTEMMODE_ProfileEnded;
            CommandedTemperature = -32767;
            globalstate.Output = 0;
            goto OnExit;
        }
    } else {
        Log(",Man,");
        CommandedTemperature = globalstate.ManualSetPoint;
        Log(",%i", CommandedTemperature);
    }

    switch (globalstate.equipmentConfig.RegulationMode) {
        case REGULATIONMODE_ComplexPID:

            globalstate.TargetPID.Setpoint = CommandedTemperature;
            PID_Compute(&globalstate.TargetPID);

            globalstate.ProcessPID.Setpoint = globalstate.TargetPID.Output;
            PID_Compute(&globalstate.ProcessPID);

            globalstate.Output = (char) globalstate.ProcessPID.Output;
            Log(",Cpid,%f2,%f2,%f2,%f2,%f2,%f2,%f2,%f2",
                    globalstate.TargetPID.DTerm,
                    globalstate.TargetPID.ITerm,
                    globalstate.TargetPID.Output,
                    globalstate.TargetPID.error,
                    globalstate.ProcessPID.DTerm,
                    globalstate.ProcessPID.ITerm,
                    globalstate.ProcessPID.Output,
                    globalstate.ProcessPID.error);
            break;
        case REGULATIONMODE_SimplePID:
            globalstate.ProcessPID.Setpoint = CommandedTemperature;
            PID_Compute(&globalstate.ProcessPID);
            globalstate.Output = (char) globalstate.ProcessPID.Output;
            Log(",Spid,%f2,%f2,%f2,%f2",
                    globalstate.ProcessPID.error,
                    globalstate.ProcessPID.ITerm,
                    globalstate.ProcessPID.DTerm,
                    globalstate.ProcessPID.Output);
            break;
        case REGULATIONMODE_SimpleThreashold:
            Log(",SimpT, NOT IMPLEMENTED");
            //            globalstate.Output = OnOffRegulation(CommandedTemperature, globalstate.ProcessPID.Input, &globalstate);
            //            Log(",%b", globalstate.Output);
            //            if (globalstate.Output > 0) {
            //                Log(",HEAT");
            //                globalstate.Output = 100;
            //            } else if (globalstate.Output < 0) {
            //                Log(",COOL");
            //                globalstate.Output = -100;
            //            } else {
            //                Log(",OFF");
            //                globalstate.Output = 0;
            //            }
            break;
        case REGULATIONMODE_ComplexThreshold:
            globalstate.TargetPID.Setpoint = CommandedTemperature;
            PID_Compute(&globalstate.TargetPID);

            Log(",CplxT,%f2,%f2,%f2,%f2",
                    globalstate.TargetPID.error,
                    globalstate.TargetPID.ITerm,
                    globalstate.TargetPID.DTerm,
                    globalstate.TargetPID.Output);

            ThresholdTemperature = globalstate.TargetPID.Input + globalstate.TargetPID.Output;

            if (globalstate.TargetPID.Output < (globalstate.equipmentConfig.coolDifferential * 0.1)) {
                OnOffMode = -1;
            } else if (globalstate.TargetPID.Output > (globalstate.equipmentConfig.heatDifferential * 0.5)) {
                OnOffMode = 1;
            }

            if (OnOffMode < 0) {
                ThresholdTemperature -= (globalstate.equipmentConfig.coolDifferential * 0.5);
            } else {
                ThresholdTemperature += (globalstate.equipmentConfig.heatDifferential * 0.5);
            }

            if (ThresholdTemperature > globalstate.equipmentConfig.TargetOutput_Max) {
                ThresholdTemperature = globalstate.equipmentConfig.TargetOutput_Max;
            } else if (ThresholdTemperature < globalstate.equipmentConfig.TargetOutput_Min) {
                ThresholdTemperature = globalstate.equipmentConfig.TargetOutput_Min;
            }

            Log(",%f2", ThresholdTemperature);

            if (OnOffMode < 0) { //Cooling Mode
                if (globalstate.Output < 0) {
                    //Cooling is ON so only turn off after we've crossed the setpoint.
                    if (globalstate.ProcessPID.Input < ThresholdTemperature) globalstate.Output = 0;
                } else {
                    globalstate.Output = 0;
                    //Cooling is OFF so wait until the temperature rises above the setpoint+differential to turn on...
                    if (globalstate.ProcessPID.Input > (ThresholdTemperature + globalstate.equipmentConfig.coolDifferential)) globalstate.Output = -100;
                }
            } else { //Heating Mode

                if (globalstate.Output > 0) {
                    //Heating is ON so only turn off after we've crossed the setpoint.
                    if (globalstate.ProcessPID.Input > ThresholdTemperature) globalstate.Output = 0;
                } else {
                    globalstate.Output = 0;
                    //Heating is OFF so wait until the temperature falls below the setpoint+differential to turn on...
                    if (globalstate.ProcessPID.Input < (ThresholdTemperature - globalstate.equipmentConfig.heatDifferential)) globalstate.Output = 100;
                }
            }

            if (globalstate.Output > 0) {
                Log(",HEAT");
                globalstate.Output = 100;
            } else if (globalstate.Output < 0) {
                Log(",COOL");
                globalstate.Output = -100;
            } else {
                Log(",OFF");
                globalstate.Output = 0;
            }
            break;
    }

    //Calculate the PWM
    FixedOffTimePWM(&globalstate);

OnExit:
    TemperatureControllerIsAlive = 1;
    SET_HEAT(globalstate.HeatRelay);
    SET_COOL(globalstate.CoolRelay);
    Log("\r\n");

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
