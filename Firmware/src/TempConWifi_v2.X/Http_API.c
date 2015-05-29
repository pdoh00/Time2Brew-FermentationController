#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "Pack.h"
#include "integer.h"
#include "RTC.h"
#include "Http_API.h"
#include "ESP8266.h"
#include "MD5.h"
#include "SystemConfiguration.h"
#include "Base64.h"
#include "circularPrintF.h"
#include "Settings.h"
#include "TemperatureController.h"
#include "FlashFS.h"
#include "fletcherChecksum.h"

char msg[512];

void BuildProfileInstanceListing(const char *ProfileName) {
    ff_File DirInfo, CacheFile;
    int bWritten;
    int res;
    char Filter[128];
    char Filename[128];
    sprintf(Filename, "prflinstlisting.%s", ProfileName);
    sprintf(Filter, "inst.%s.", ProfileName);
    int FilterLength = strlen(Filter);

    ff_Delete(Filename);
    res = ff_OpenDirectoryListing(&DirInfo);
    res = ff_OpenByFileName(&CacheFile, Filename, 1);
    while (1) {
        res = ff_GetNextEntryFromDirectory(&DirInfo, msg);
        if (res == FR_EOF) {
            ff_UpdateLength(&CacheFile);
            return;
        }
        if (memcmp(msg, Filter, FilterLength) == 0) {
            sprintf(msg, "%s\r\n", msg + FilterLength);
            ff_Append(&CacheFile, (BYTE*) msg, strlen(msg), &bWritten);
        }
    }
}

void BuildProfileListing() {
    ff_File DirInfo, CacheFile;
    int bWritten;
    int res;

    res = ff_Delete("ProfileListing.txt");
    res = ff_OpenDirectoryListing(&DirInfo);
    res = ff_OpenByFileName(&CacheFile, "ProfileListing.txt", 1);

    while (1) {
        res = ff_GetNextEntryFromDirectory(&DirInfo, msg);
        if (res != FR_OK) {
            ff_UpdateLength(&CacheFile);
            return;
        }
        if (memcmp("prfl.", msg, 5) == 0) {
            sprintf(msg, "%s\r\n", msg + 5);
            ff_Append(&CacheFile, (BYTE *) msg, strlen(msg), &bWritten);
            Log("Found: %s\r\n", msg);
        }
    }
}

void BuildEquipmentProfileListing() {
    ff_File DirInfo, CacheFile;
    int bWritten;
    int res;

    res = ff_Delete("EquipmentProfileListing.txt");
    res = ff_OpenDirectoryListing(&DirInfo);
    res = ff_OpenByFileName(&CacheFile, "EquipmentProfileListing.txt", 1);

    while (1) {
        res = ff_GetNextEntryFromDirectory(&DirInfo, msg);
        if (res != FR_OK) {
            ff_UpdateLength(&CacheFile);
            return;
        }
        if (memcmp("equip.", msg, 5) == 0) {
            sprintf(msg, "%s\r\n", msg + 6);
            ff_Append(&CacheFile, (BYTE *) msg, strlen(msg), &bWritten);
            Log("Found: %s\r\n", msg);
        }
    }
}

void GET_echo(HTTP_REQUEST *req, const char *urlParameter) {
    char *echoMessage;
    int echoMessageLength;
    url_queryParse(urlParameter, "echo", &echoMessage, &echoMessageLength);
    sprintf(msg, "GET: You Said:%.*s", echoMessageLength, echoMessage);
    Send200_OK_SmallMsg(req, msg);
}

void PUT_echo(HTTP_REQUEST *req, const char *urlParameter) {
    char *echoMessage = (char *) req->Content;
    int echoMessageLength = req->ContentLength;
    url_queryParse(urlParameter, "echo", &echoMessage, &echoMessageLength);
    sprintf(msg, "PUT: You Said:%.*s", echoMessageLength, echoMessage);
    Send200_OK_SmallMsg(req, msg);
}

void GET_profile(HTTP_REQUEST *req, const char *urlParameter) {
    char ProfileName[64];

    if (url_queryParse2(urlParameter, "name", ProfileName, 62)) {
        sprintf(req->Resource, "/prfl.%s", ProfileName);
        req->ETag[0] = 0;
        req->ContentLength = 0;
        Process_GET_File(req);
    } else {
        if (ff_exists("ProfileListing.txt") == 0) {
            BuildProfileListing();
        }

        sprintf(req->Resource, "/ProfileListing.txt");
        req->ETag[0] = 0;
        req->ContentLength = 0;
        Process_GET_File(req);
    }
}

void PUT_profile(HTTP_REQUEST *req, const char *urlParameter) {
    char IsFinal, Overwrite;
    char ProfileName[64];
    ff_File handle;
    int bWritten;
    int res;
    int offset;
    char *b64_Content;
    BYTE *ContentData;
    int ContentLength;
    int b64_ContentLength;
    unsigned int chkSum;


    if (url_queryParse2(urlParameter, "isfinal", ProfileName, 2)) {
        if (ProfileName[0] == 'n') IsFinal = 0;
        else IsFinal = 1;
    } else {
        IsFinal = 1;
    }

    if (url_queryParse2(urlParameter, "overwrite", ProfileName, 2)) {
        if (ProfileName[0] == 'n') Overwrite = 0;
        else Overwrite = 1;
    } else {
        Overwrite = 1;
    }

    if (url_queryParse2(urlParameter, "offset", ProfileName, 6)) {
        if (sscanf(ProfileName, "%d", &offset) != 1) {
            sprintf(msg, "Error Offset was invalid: offset=\"%s\"", ProfileName);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        offset = 0;
    }



    if (url_queryParse2(urlParameter, "chksum", ProfileName, 6)) {
        if (sscanf(ProfileName, "%u", &chkSum) != 1) {
            sprintf(msg, "Error chksum could not parse: chksum=\"%s\"", ProfileName);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        Send500_InternalServerError(req, "Query Parameter 'chksum' was not provided.");
        return;
    }

    if (url_queryParse(urlParameter, "content", &b64_Content, &b64_ContentLength) == 0) {
        Send500_InternalServerError(req, "Parameter 'content' is required");
        return;
    }
    if (b64_ContentLength == 0) b64_ContentLength = strlen(b64_Content);

    sprintf(msg, "Base64Content Length=%d Value='%.*s'\r\n", b64_ContentLength, b64_ContentLength, b64_Content);
    Log("%s", msg);
    ContentData = (BYTE *) b64_Content;
    ContentLength = decode_Base64(b64_Content, b64_ContentLength, ContentData);
    if (ContentLength < 0) {
        Send500_InternalServerError(req, "Unable to decode base64 Content...");
        return;
    }
    Log("Decoded Length=%i\r\n", ContentLength);

    unsigned int calcChkSum = fletcher16(ContentData, ContentLength);
    if (calcChkSum != chkSum) {
        sprintf(msg, "Checksums Do NOT match: Actual=%u Expected=%u", calcChkSum, chkSum);
        Send500_InternalServerError(req, msg);
        return;
    }

    if (url_queryParse2(urlParameter, "name", ProfileName, 63) == 0) {
        Send500_InternalServerError(req, "Query Parameter 'name' was not provided.");
        return;
    }
    sprintf(msg, "prfl.%s", ProfileName);


    if (Overwrite) res = ff_Delete(msg);

    res = ff_OpenByFileName(&handle, msg, 1);
    if (res != FR_OK) {
        sprintf(msg, "Error Creating Profile File: res=%d", res);
        Send500_InternalServerError(req, msg);
        return;
    }

    if (offset > 0) {
        res = ff_Seek(&handle, offset, ff_SeekMode_Absolute);
        if (res != FR_OK) {
            sprintf(msg, "Error Seeking Profile File: res=%d", res);
            Send500_InternalServerError(req, msg);
            return;
        }
    }

    res = ff_Append(&handle, ContentData, ContentLength, &bWritten);
    if (res != FR_OK) {
        sprintf(msg, "Error Writing Profile File: res=%d", res);
        Send500_InternalServerError(req, msg);
        return;
    }

    if (IsFinal) {
        Log("Finalizing Upload...\r\n");
        res = ff_UpdateLength(&handle);
        if (res != FR_OK) {
            sprintf(msg, "Error Updating Profile File Length: res=%d", res);
            Send500_InternalServerError(req, msg);
            return;
        }

        MACHINE_STATE dummy;
        if (LoadProfile(handle.FileName, msg, &dummy) != FR_OK) {
            ff_Delete(handle.FileName);
            sprintf(msg, "Error Profile is Corrupt: res=%d, Msg=\"%s\"\r\n", res, msg);
            Send500_InternalServerError(req, msg);
            return;
        }

        BuildProfileListing();
    }
    Send200_OK_Simple(req);
    return;
}

void PUT_executeProfile(HTTP_REQUEST *req, const char *urlParameter) {
    if (globalstate.SystemMode != SYSTEMMODE_IDLE) {
        sprintf(msg, "Controller is not IDLE");
        Send500_InternalServerError(req, msg);
        return;
    }
    char ProfileName[64];
    if (url_queryParse2(urlParameter, "name", ProfileName, 63)) {
        if (ExecuteProfile(ProfileName, msg) != 1) {
            Send500_InternalServerError(req, msg);
        } else {
            BuildProfileInstanceListing(ProfileName);
            Send200_OK_Simple(req);
        }
    } else {
        Send500_InternalServerError(req, "Query Parameter 'name' was not provided.");
    }
}

void PUT_terminateProfile(HTTP_REQUEST *req, const char *urlParameter) {
    TerminateProfile();
    Send200_OK_Simple(req);
}

void PUT_truncateProfile(HTTP_REQUEST *req, const char *urlParameter) {
    char *b64_Content;
    BYTE *ContentData;
    int ContentLength;
    int b64_ContentLength;
    unsigned int chkSum;

    char temp[16];

    if (url_queryParse2(urlParameter, "chksum", temp, 6)) {
        if (sscanf(temp, "%u", &chkSum) != 1) {
            sprintf(msg, "Error chksum could not parse: chksum=\"%s\"", temp);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        Send500_InternalServerError(req, "Query Parameter 'chksum' was not provided.");
        return;
    }

    if (url_queryParse(urlParameter, "content", &b64_Content, &b64_ContentLength) == 0) {
        Send500_InternalServerError(req, "Parameter 'content' is required");
        return;
    }
    if (b64_ContentLength == 0) b64_ContentLength = strlen(b64_Content);

    ContentData = (BYTE *) b64_Content;
    ContentLength = decode_Base64(b64_Content, b64_ContentLength, ContentData);
    if (ContentLength < 0) {
        Send500_InternalServerError(req, "Unable to decode base64 Content...");
        return;
    }

    unsigned int calcChkSum = fletcher16(ContentData, ContentLength);
    if (calcChkSum != chkSum) {
        sprintf(msg, "Checksums Do NOT match: Actual=%u Expected=%u", calcChkSum, chkSum);
        Send500_InternalServerError(req, msg);
        return;
    }


    if (globalstate.SystemMode != SYSTEMMODE_Profile) {
        Send500_InternalServerError(req, "Controller is not running a profile");
        return;
    }

    int res = TruncateProfile(ContentData, ContentLength, msg);
    if (res == 1) {
        Send200_OK_Simple(req);
    } else {
        Send500_InternalServerError(req, msg);
    }
}

void GET_runHistory(HTTP_REQUEST *req, const char *urlParameter) {
    char profileName[64];
    char strInstance[14];

    if (url_queryParse2(urlParameter, "name", profileName, 63)) {
        if (url_queryParse2(urlParameter, "instance", strInstance, 13)) {
            sprintf(req->Resource, "/inst.%s.%s", profileName, strInstance);
            req->ETag = NULL;
            req->ContentLength = 0;
            Log("Passing to Get_File Resource=%s\r\n", req->Resource);
            Process_GET_File(req);
        } else {
            sprintf(req->Resource, "prflinstlisting.%s", profileName);
            if (ff_exists(req->Resource) == 0) {
                BuildProfileInstanceListing(profileName);
            }
            //No Instance provided so send a list of instances            
            sprintf(req->Resource, "/prflinstlisting.%s", profileName);
            req->ETag = NULL;
            req->ContentLength = 0;
            Process_GET_File(req);
            return;
        }
    } else {
        //No name so send a list of profiles...
        sprintf(req->Resource, "/ProfileListing.txt");
        req->ETag = NULL;
        req->ContentLength = 0;
        Process_GET_File(req);
        return;
    }
}

void GET_temperatureTrend(HTTP_REQUEST *req, const char *urlParameter) {
    char profileName[64];
    char strInstance[14];

    if (url_queryParse2(urlParameter, "name", profileName, 63)) {
        if (url_queryParse2(urlParameter, "instance", strInstance, 13)) {
            unsigned long sendLength = ProfileTrendFile.Length;
            sprintf(req->Resource, "trnd.%s.%s", profileName, strInstance);
            if ((globalstate.SystemMode == SYSTEMMODE_Profile) &&
                    (strcmp(ProfileTrendFile.FileName, req->Resource) == 0)) {
                unsigned long et = globalstate.SystemTime - globalstate.ProfileStartTime;
                et /= 60;
                et += 1;
                sendLength = et * 8;
            }
            sprintf(req->Resource, "/trnd.%s.%s", profileName, strInstance);
            req->ETag = NULL;
            req->ContentLength = 0;
            Log("Passing to Get_File Resource=%s\r\n", req->Resource);
            Process_GET_File_ex(req, 0, sendLength);
        } else {
            //No Instance provided so send a list of instances
            sprintf(req->Resource, "prflinstlisting.%s", profileName);
            if (ff_exists(req->Resource) == 0) {
                BuildProfileInstanceListing(profileName);
            }

            sprintf(req->Resource, "/prflinstlisting.%s", profileName);
            req->ETag = NULL;
            req->ContentLength = 0;
            Process_GET_File(req);
            return;
        }
    } else {
        //No name so send a list of profiles...
        sprintf(req->Resource, "/ProfileListing.txt");
        req->ETag = NULL;
        req->ContentLength = 0;
        Process_GET_File(req);
    }
}

void GET_temperature(HTTP_REQUEST *req, const char *urlParameter) {
    Log("GET_temperature urlParameter=%s\r\n", urlParameter);
    char ProbeID[3];
    if (url_queryParse2(urlParameter, "probe", ProbeID, 2)) {
        if (ProbeID[0] == '0') {
            sprintf(msg, "%d\r\n", globalstate.Probe0Temperature);
            Send200_OK_SmallMsg(req, msg);
        } else if (ProbeID[0] == '1') {
            sprintf(msg, "%d\r\n", globalstate.Probe1Temperature);
            Send200_OK_SmallMsg(req, msg);
        } else {
            sprintf(msg, "%d\r\n%d\r\n", globalstate.Probe0Temperature, globalstate.Probe1Temperature);
            Send200_OK_SmallMsg(req, msg);
        }
    } else {
        sprintf(msg, "%d\r\n%d\r\n", globalstate.Probe0Temperature, globalstate.Probe1Temperature);
        Send200_OK_SmallMsg(req, msg);
    }
}

void GET_status(HTTP_REQUEST *req, const char *urlParameter) {
    MACHINE_STATE temp;
    DISABLE_INTERRUPTS;
    memcpy((BYTE *) & temp, (BYTE *) & globalstate, sizeof (MACHINE_STATE));
    ENABLE_INTERRUPTS;
    unsigned int len = 64;

    unsigned char *packedData = req->rawbuffer;
    unsigned char *cursour = Pack(packedData, "lbbbIbIbbaiIlIlallllBffffll", temp.SystemTime, temp.SystemMode,
            temp.equipmentConfig.RegulationMode, temp.equipmentConfig.Probe0Assignment, temp.Probe0Temperature,
            temp.equipmentConfig.Probe1Assignment, temp.Probe1Temperature, temp.HeatRelay,
            temp.CoolRelay, temp.ActiveProfileName, len, temp.StepIdx,
            temp.StepTemperature, temp.StepTimeRemaining,
            temp.ManualSetPoint, temp.ProfileStartTime, temp.EquipmentName, len,
            temp.CoolWhenCanTurnOff, temp.CoolWhenCanTurnOn, temp.HeatWhenCanTurnOff, temp.HeatWhenCanTurnOn,
            temp.Output, temp.ProcessPID.ITerm, temp.ProcessPID.error,
            temp.TargetPID.ITerm, temp.TargetPID.error,
            temp.ProfileStartTime, temp.TimeTurnedOn);
    //sprintf_Base64(msg, packedData, (unsigned int) cursour - (unsigned int) packedData);
    //Send200_OK_SmallMsg(req, msg);
    Send200_OK_Data(req, packedData, (unsigned int) cursour - (unsigned int) packedData);
}

void PUT_temperature(HTTP_REQUEST *req, const char *urlParameter) {
    if (globalstate.SystemMode == SYSTEMMODE_Profile) {
        sprintf(msg, "Controller is running a profile...");
        Send500_InternalServerError(req, msg);
        return;
    }

    char strSetpoint[4];
    int Setpoint;

    if (url_queryParse2(urlParameter, "setpoint", strSetpoint, 4)) {
        if (sscanf(strSetpoint, "%d", &Setpoint) != 1) {
            Send500_InternalServerError(req, "Error: 'setpoint' is invalid");
            return;
        }
    } else {
        Send500_InternalServerError(req, "Error: 'setpoint' is not provided");
        return;
    }

    if (SetManualMode(Setpoint, msg) != 1) {
        Send500_InternalServerError(req, msg);
        return;
    }

    Send200_OK_Simple(req);

}

void PUT_UpdateCredentials(HTTP_REQUEST *req, const char *urlParameter) {
    char username[64];
    char password[64];

    if (!url_queryParse2(urlParameter, "username", username, 63)) {
        Send500_InternalServerError(req, "username is not provided or is outside of bounds...");
        return;
    }

    if (!url_queryParse2(urlParameter, "password", password, 63)) {
        Send500_InternalServerError(req, "password is not provided or is outside of bounds...");
        return;
    }

    if (UpdateCredentials(username, password) != 1) {
        Send500_InternalServerError(req, "Unable to Update Credentials...");
        return;
    }
    sprintf(msg, "%s:%s:%s", username, ESP_Config.Name, password);
    Send200_OK_Simple(req);
}

int UpdateCredentials(const char*username, const char *password) {
    char buff[192];
    sprintf(buff, "%s:%s:%s", username, ESP_Config.Name, password);
    Log("   Updating Credentials: \"%s\"", buff);
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, msg, strlen(msg));
    MD5_Final(ESP_Config.HA1, &ctx);
    if (SaveConfig()) {
        return 1;
    } else {
        return 0;
    }
}

void GET_Time(HTTP_REQUEST *req, const char *urlParameter) {
    Log("PUT_Time urlParameter=%s\r\n", urlParameter);
    char strTime[16];
    unsigned long tm;

    if (url_queryParse2(urlParameter, "time", strTime, 15)) {
        sscanf(strTime, "%lu", &tm);
        DISABLE_INTERRUPTS;
        globalstate.SystemTime = tm;
        ENABLE_INTERRUPTS;
        RTC_SetTime(tm);
        Send200_OK_Simple(req);
    } else {
        tm = RTC_GetTime();
        sprintf(msg, "%lu", tm);
        Send200_OK_SmallMsg(req, msg);
    }
}

void PUT_factorydefault(HTTP_REQUEST *req, const char *urlParameter) {
    Log("PUT_Time urlParameter=%s\r\n", urlParameter);
    char strConfirm[16];
    if (url_queryParse2(urlParameter, "confirm", strConfirm, 15)) {
        if (memcmp(strConfirm, "killcfg", 6) == 0) {
            Send200_OK_Simple(req);
            ff_Delete(WiFiConfigFilename);
            asm("RESET");
        }
    } else {
        Send500_InternalServerError(req, "confirm must be = 'killcfg'");
        return;
    }
}

void PUT_WifiConfig(HTTP_REQUEST *req, const char *urlParameter) {
    Log("PUT_WifiConfig urlParameter=%s\r\n", urlParameter);
    char token[64];
    int channel;

    if (url_queryParse2(urlParameter, "mode", token, 8)) {
        if (strcmp("SOFT_AP", token) == 0) {
            ESP_Config.Mode = SOFTAP_MODE;
        } else if (strcmp("STATION", token) == 0) {
            ESP_Config.Mode = STATION_MODE;
        } else {
            Send500_InternalServerError(req, "mode parameter is can be 'SOFT_AP' or 'STATION' only");
            return;
        }
    }

    if (url_queryParse2(urlParameter, "channel", token, 2)) {
        if (sscanf(token, "%d", &channel) != 1) {
            Send500_InternalServerError(req, "channel parameter is incorrect");
            return;
        }
        if (channel > 0 && channel <= 12) {
            ESP_Config.Channel = channel;
        } else {
            Send500_InternalServerError(req, "channel parameter is out of bounds (1-12)");
            return;
        }
    }

    if (url_queryParse2(urlParameter, "encryption", token, 12)) {
        if (strcmp("AUTH_OPEN", token) == 0) {
            ESP_Config.EncryptionMode = AUTH_OPEN;
        } else if (strcmp("AUTH_WPA_PSK", token) == 0) {
            ESP_Config.EncryptionMode = AUTH_WPA_PSK;
        } else if (strcmp("AUTH_WPA2_PSK", token) == 0) {
            ESP_Config.EncryptionMode = AUTH_WPA2_PSK;
        } else {
            Send500_InternalServerError(req, "mode parameter is can be 'AUTH_OPEN' or 'AUTH_WPA_PSK' or 'AUTH_WPA2_PSK' only");
            return;
        }
    }

    if (url_queryParse2(urlParameter, "name", token, 63)) {
        sprintf(ESP_Config.Name, "%s", token);
    }

    if (url_queryParse2(urlParameter, "password", token, 31)) {
        sprintf(ESP_Config.Password, "%s", token);
    }

    if (url_queryParse2(urlParameter, "ssid", token, 31)) {
        sprintf(ESP_Config.SSID, "%s", token);
    }

    if (SaveConfig() == 0) {
        Send500_InternalServerError(req, "Unable to save configuration");
        return;
    }

    Send200_OK_Simple(req);
}

void PUT_Restart(HTTP_REQUEST *req, const char *urlParameter) {
    Log("PUT_Restart urlParameter=%s\r\n", urlParameter);
    char token[64];

    if (url_queryParse2(urlParameter, "confirm", token, 8)) {
        if (strcmp("restart", token) == 0) {
            Send200_OK_Simple(req);
            asm("RESET");
        } else {
            Send500_InternalServerError(req, "confirm must be = 'restart'");
            return;
        }
    } else {
        Send500_InternalServerError(req, "confirm must be = 'restart'");
        return;
    }
}

void PUT_Format(HTTP_REQUEST *req, const char *urlParameter) {
    Log("PUT_Format urlParameter=%s\r\n", urlParameter);
    char token[64];

    if (url_queryParse2(urlParameter, "confirm", token, 8)) {
        if (strcmp("format", token) == 0) {
            Send200_OK_Simple(req);
            ff_Format();
            asm("reset");
        } else {
            Send500_InternalServerError(req, "confirm must be = 'format'");
            return;
        }
    } else {
        Send500_InternalServerError(req, "confirm must be = 'format'");
        return;
    }
}

void PUT_Equipment(HTTP_REQUEST *req, const char *urlParameter) {
    char IsFinal, Overwrite;
    unsigned int offset;
    char temp[128];
    char equipName[65];
    char filename[120];
    int bWritten, res;
    ff_File file;
    EQUIPMENT_PROFILE dummy;
    char *b64_Content;
    BYTE *ContentData;
    int ContentLength;
    int b64_ContentLength;
    unsigned int chkSum;


    if (url_queryParse2(urlParameter, "offset", equipName, 6)) {
        if (sscanf(equipName, "%d", &offset) != 1) {
            sprintf(msg, "Error Offset was invalid: offset=\"%s\"", equipName);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        offset = 0;
    }

    if (url_queryParse2(urlParameter, "isfinal", equipName, 2)) {
        if (equipName[0] == 'n') IsFinal = 0;
        else IsFinal = 1;
    } else {
        IsFinal = 1;
    }

    if (url_queryParse2(urlParameter, "overwrite", equipName, 2)) {
        if (equipName[0] == 'n') Overwrite = 0;
        else Overwrite = 1;
    } else {
        Overwrite = 1;
    }

    if (url_queryParse2(urlParameter, "name", equipName, 64) == 0) {
        sprintf(msg, "the 'name' parameter must be provided and is limited to 1-64 characters.");
        Send500_InternalServerError(req, msg);
        return;
    }
    sprintf(filename, "equip.%s", equipName);

    if (url_queryParse2(urlParameter, "chksum", temp, 6)) {
        if (sscanf(temp, "%u", &chkSum) != 1) {
            sprintf(msg, "Error chksum could not parse: chksum=\"%s\"", temp);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        Send500_InternalServerError(req, "Query Parameter 'chksum' was not provided.");
        return;
    }

    if (url_queryParse(urlParameter, "content", &b64_Content, &b64_ContentLength) == 0) {
        Send500_InternalServerError(req, "Parameter 'content' is required");
        return;
    }
    if (b64_ContentLength == 0) b64_ContentLength = strlen(b64_Content);

    sprintf(msg, "Base64Content Length=%d Value='%.*s'\r\n", b64_ContentLength, b64_ContentLength, b64_Content);
    Log("%s", msg);
    ContentData = (BYTE *) b64_Content;
    ContentLength = decode_Base64(b64_Content, b64_ContentLength, ContentData);
    if (ContentLength < 0) {
        Send500_InternalServerError(req, "Unable to decode base64 Content...");
        return;
    }
    Log("Decoded Length=%i\r\n", ContentLength);

    unsigned int calcChkSum = fletcher16(ContentData, ContentLength);
    if (calcChkSum != chkSum) {
        sprintf(msg, "Checksums Do NOT match: Actual=%u Expected=%u", calcChkSum, chkSum);
        Send500_InternalServerError(req, msg);
        return;
    }

    if (ContentLength != sizeof (EQUIPMENT_PROFILE)) {
        sprintf(msg, "Equipment Profile Data Length is out of bounds. Expected=%i", sizeof (EQUIPMENT_PROFILE));
        Send500_InternalServerError(req, msg);
        return;
    }

    if (Overwrite) ff_Delete(filename);

    res = ff_OpenByFileName(&file, filename, 1);
    if (res != FR_OK) {
        sprintf(msg, "Unable to create Equipment Profile: Error=%s", Translate_DRESULT(res));
        Send500_InternalServerError(req, msg);
        return;
    }

    if (offset > 0) {
        res = ff_Seek(&file, offset, ff_SeekMode_Absolute);
        if (res != FR_OK) {
            sprintf(msg, "Error Seeking Equipment Profile File: res=%d", res);
            Send500_InternalServerError(req, msg);
            return;
        }
    }

    res = ff_Append(&file, ContentData, ContentLength, &bWritten);
    if (res != FR_OK) {
        sprintf(msg, "Unable to Write Equipment Profile: Error=%s", Translate_DRESULT(res));
        Send500_InternalServerError(req, msg);
        return;
    }

    if (IsFinal) {
        res = ff_UpdateLength(&file);
        if (res != FR_OK) {
            sprintf(msg, "Unable to Update Length of Equipment Profile: Error=%s", Translate_DRESULT(res));
            Send500_InternalServerError(req, msg);
            return;
        }

        res = LoadEquipmentProfile(file.FileName, msg, &dummy);
        if (res != FR_OK) {
            Send500_InternalServerError(req, msg);
            return;
        }

        BuildEquipmentProfileListing();
    }

    Send200_OK_Simple(req);

}

void GET_Equipment(HTTP_REQUEST *req, const char *urlParameter) {
    char equipName[65];
    if (url_queryParse2(urlParameter, "name", equipName, 64)) {
        sprintf(req->Resource, "/equip.%s", equipName);
        req->ETag = NULL;
        req->ContentLength = 0;
        Log("Passing to Get_File Resource=%s\r\n", req->Resource);
        Process_GET_File(req);
    } else {
        if (ff_exists("EquipmentProfileListing.txt") == 0) {
            BuildEquipmentProfileListing();
        }
        sprintf(req->Resource, "/EquipmentProfileListing.txt");
        req->ETag = NULL;
        req->ContentLength = 0;
        Process_GET_File(req);
        return;
    }
}

void PUT_SetEquipment(HTTP_REQUEST *req, const char *urlParameter) {
    char equipName[65];
    int ret;
    if (url_queryParse2(urlParameter, "name", equipName, 64)) {
        sprintf(req->Resource, "equip.%s", equipName);
        ret = SetEquipmentProfile(req->Resource, msg, &globalstate);
        if (ret != FR_OK) {
            Send500_InternalServerError(req, msg);
            return;
        }
        Send200_OK_Simple(req);
        return;
    } else {
        Send500_InternalServerError(req, "the 'name' parameter must be provided and is limited to 1-64 characters.");

        return;
    }
}

void PUT_trimFileSystem(HTTP_REQUEST *req, const char *urlParameter) {
    int res;
    res = ff_RepairFS();
    if (res != FR_OK) {
        sprintf(msg, "Trim Failed Res='%s'", Translate_DRESULT(res));
        Send500_InternalServerError(req, msg);
    } else {
        Send200_OK_Simple(req);
    }
}

void PUT_deleteProfileInstance(HTTP_REQUEST *req, const char *urlParameter) {
    char profileName[64];
    char strInstance[14];
    int res;

    if (url_queryParse2(urlParameter, "name", profileName, 63)) {
        if (url_queryParse2(urlParameter, "instance", strInstance, 13)) {
            sprintf(req->Resource, "trnd.%s.%s", profileName, strInstance);
            if ((globalstate.SystemMode == SYSTEMMODE_Profile) &&
                    (strcmp(ProfileTrendFile.FileName, req->Resource) == 0)) {
                Send500_InternalServerError(req, "Error: Unable to delete the currently running profile's instance");
                return;
            }

            sprintf(msg, "trnd.%s.%s", profileName, strInstance);
            res = ff_Delete(msg);
            if (res != FR_OK) {
                sprintf(msg, "Error: Unable to delete Trend Record: res='%s'", Translate_DRESULT(res));
                Send500_InternalServerError(req, msg);
                return;
            }
            sprintf(msg, "inst.%s.%s", profileName, strInstance);
            res = ff_Delete(msg);
            if (res != FR_OK) {
                sprintf(msg, "Error: Unable to delete Instance Record: res='%s'", Translate_DRESULT(res));
                Send500_InternalServerError(req, msg);
                return;
            }
            Send200_OK_Simple(req);
            return;
        } else {
            sprintf(msg, "Error: 'instance' is requried");
            Send500_InternalServerError(req, msg);
        }
    } else {
        sprintf(msg, "Error: 'name' is requried");
        Send500_InternalServerError(req, msg);
    }
}

void PUT_deleteProfile(HTTP_REQUEST *req, const char *urlParameter) {
    char profileName[65];
    int res;
    if (url_queryParse2(urlParameter, "name", profileName, 64) == 0) {
        sprintf(msg, "Error: 'name' is requried");
        Send500_InternalServerError(req, msg);
        return;
    }

    sprintf(req->Resource, ".%s.", profileName);

    if ((globalstate.SystemMode == SYSTEMMODE_Profile) &&
            (strstr(ProfileTrendFile.FileName, req->Resource) != NULL)) {
        Send500_InternalServerError(req, "Error: Unable to delete the currently running profile.");
        return;
    }

    ff_File DirInfo;
    char instanceFilter[128];
    char trendFilter[128];
    sprintf(msg, "prflinstlisting.%s", profileName);
    sprintf(instanceFilter, "inst.%s.", profileName);
    sprintf(trendFilter, "trnd.%s.", profileName);
    int FilterLength = strlen(instanceFilter);

    //Delete the profile instance listing file
    ff_Delete(msg);

    //Look for all instances...and trend records...
    res = ff_OpenDirectoryListing(&DirInfo);
    while (1) {
        res = ff_GetNextEntryFromDirectory(&DirInfo, msg);
        if (res != FR_OK) break;
        if (memcmp(msg, instanceFilter, FilterLength) == 0) {
            ff_Delete(msg); //Delete all instances
        } else if (memcmp(msg, trendFilter, FilterLength) == 0) {
            ff_Delete(msg); //Delete all trend instances
        }
    }

    //Delete the profile itself.
    sprintf(msg, "prfl.%s", profileName);
    res = ff_Delete(msg);
    if (res != FR_OK) {
        sprintf(msg, "Error Deleting Profile res='%s'", Translate_DRESULT(res));
        Send500_InternalServerError(req, msg);
        return;
    }

    BuildProfileListing(); //rebuild the profile listing
    Send200_OK_Simple(req);

    return;
}

void PUT_deleteEquipmentProfile(HTTP_REQUEST *req, const char *urlParameter) {
    char equipName[65];
    int res;

    if (url_queryParse2(urlParameter, "name", equipName, 64) == 0) {
        sprintf(msg, "Error: 'name' is requried");
        Send500_InternalServerError(req, msg);
        return;
    }

    sprintf(req->Resource, "equip.%s", equipName);
    if (strcmp(globalstate.EquipmentName, req->Resource) == 0) {
        Send500_InternalServerError(req, "Error: Unable to delete the active Equipment Profile");
        return;
    }
    res = ff_Delete(req->Resource);
    if (res != FR_OK) {
        sprintf(msg, "Error Deleting Equipment Profile res='%s'", Translate_DRESULT(res));
        Send500_InternalServerError(req, msg);
        return;
    }

    BuildEquipmentProfileListing();
    Send200_OK_Simple(req);

    return;
}

void PUT_uploadfirmware(HTTP_REQUEST *req, const char *urlParameter) {
    char Overwrite;
    char temp[32];
    unsigned long offset;
    char *b64_Content;
    BYTE *ContentData;
    int ContentLength;
    int b64_ContentLength;
    unsigned int chkSum;

    if (url_queryParse2(urlParameter, "overwrite", temp, 2)) {
        if (temp[0] == 'n') Overwrite = 0;
        else Overwrite = 1;
    } else {
        Send500_InternalServerError(req, "parameter 'overwrite' is required");
        return;
    }

    if (url_queryParse2(urlParameter, "offset", temp, 12)) {
        if (sscanf(temp, "%lu", &offset) != 1) {
            sprintf(msg, "Error Offset was invalid: offset=\"%s\"", temp);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        Send500_InternalServerError(req, "parameter 'offset' is required");
        return;
    }

    if (url_queryParse2(urlParameter, "chksum", temp, 6)) {
        if (sscanf(temp, "%u", &chkSum) != 1) {
            sprintf(msg, "Error chksum could not parse: chksum=\"%s\"", temp);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        Send500_InternalServerError(req, "Query Parameter 'chksum' was not provided.");
        return;
    }

    if (url_queryParse(urlParameter, "content", &b64_Content, &b64_ContentLength) == 0) {
        Send500_InternalServerError(req, "Parameter 'content' is required");
        return;
    }

    if (b64_ContentLength == 0) b64_ContentLength = strlen(b64_Content);

    ContentData = (BYTE *) b64_Content;
    ContentLength = decode_Base64(b64_Content, b64_ContentLength, ContentData);
    if (ContentLength < 0) {
        Send500_InternalServerError(req, "Unable to decode base64 Content...");
        return;
    }

    unsigned int calcChkSum = fletcher16(ContentData, ContentLength);
    if (calcChkSum != chkSum) {
        sprintf(msg, "Checksums Do NOT match: Actual=%u Expected=%u", calcChkSum, chkSum);
        Send500_InternalServerError(req, msg);
        return;
    }


    unsigned long limit = FIRMWARE_RESERVED_SIZE;

    if (offset + ContentLength > limit) {
        Send500_InternalServerError(req, "Firmware Length exceeds allowable size...");
        return;
    }

    unsigned long baseAddress;
    if (Overwrite) {
        for (baseAddress = FIRMWARE_PRIMARY_ADDRESS; baseAddress < (FIRMWARE_PRIMARY_ADDRESS + limit); baseAddress += 4096) {
            diskEraseSector(baseAddress);
        }
    }

    baseAddress = FIRMWARE_PRIMARY_ADDRESS;
    baseAddress += offset;
    diskWrite(baseAddress, ContentLength, ContentData);

    Log("UploadFirmware: Offset=%ul OK\r\n", offset);
    Send200_OK_Simple(req);
}

void PUT_uploadFile(HTTP_REQUEST *req, const char *urlParameter) {
    char temp[32];
    char fname[64];
    unsigned long offset;
    char *b64_Content;
    BYTE *ContentData;
    int ContentLength;
    int b64_ContentLength;
    unsigned int chkSum;
    ff_File handle;
    int ret;

    if (url_queryParse2(urlParameter, "fname", fname, 63) == 0) {
        Send500_InternalServerError(req, "parameter 'fname' is required");
        return;
    }

    if (url_queryParse2(urlParameter, "overwrite", temp, 2)) {
        if (temp[0] == 'y') ff_Delete(fname);
        ret = ff_OpenByFileName(&handle, fname, 1);
        if (ret != FR_OK) {
            sprintf(msg, "Error: Unable to open/create file: res='%s'", Translate_DRESULT(ret));
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        Send500_InternalServerError(req, "parameter 'overwrite' is required");
        return;
    }

    if (url_queryParse2(urlParameter, "offset", temp, 12)) {
        if (sscanf(temp, "%lu", &offset) != 1) {
            sprintf(msg, "Error Offset was invalid: offset=\"%s\"", temp);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        Send500_InternalServerError(req, "parameter 'offset' is required");
        return;
    }

    if (url_queryParse2(urlParameter, "chksum", temp, 6)) {
        if (sscanf(temp, "%u", &chkSum) != 1) {
            sprintf(msg, "Error chksum could not parse: chksum=\"%s\"", temp);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        Send500_InternalServerError(req, "Query Parameter 'chksum' was not provided.");
        return;
    }

    if (url_queryParse(urlParameter, "content", &b64_Content, &b64_ContentLength) == 0) {
        Send500_InternalServerError(req, "Parameter 'content' is required");
        return;
    }

    if (b64_ContentLength == 0) b64_ContentLength = strlen(b64_Content);

    ContentData = (BYTE *) b64_Content;
    ContentLength = decode_Base64(b64_Content, b64_ContentLength, ContentData);
    if (ContentLength < 0) {
        Send500_InternalServerError(req, "Unable to decode base64 Content...");
        return;
    }

    unsigned int calcChkSum = fletcher16(ContentData, ContentLength);
    if (calcChkSum != chkSum) {
        sprintf(msg, "Checksums Do NOT match: Actual=%u Expected=%u", calcChkSum, chkSum);
        Send500_InternalServerError(req, msg);
        return;
    }

    ret = ff_Seek(&handle, offset, ff_SeekMode_Absolute);
    if (ret != FR_OK) {
        sprintf(msg, "Error: Unable to seek file: res='%s'", Translate_DRESULT(ret));
        Send500_InternalServerError(req, msg);
        return;
    }

    int bWritten;
    ret = ff_Append(&handle, ContentData, ContentLength, &bWritten);
    if (ret != FR_OK) {
        sprintf(msg, "Error: Unable to append file: res='%s'", Translate_DRESULT(ret));
        Send500_InternalServerError(req, msg);
        return;
    }

    if (url_queryParse2(urlParameter, "finalize", temp, 2)) {
        if (temp[0] == 'y') {
            ret = ff_UpdateLength(&handle);
            if (ret != FR_OK) {
                sprintf(msg, "Error: Unable to finalize file: res='%s'", Translate_DRESULT(ret));
                Send500_InternalServerError(req, msg);
                return;
            }
        }
    }

    Log("UploadFile: '%s' Offset=%ul OK\r\n", fname, offset);
    Send200_OK_Simple(req);
}

void PUT_deleteFile(HTTP_REQUEST *req, const char *urlParameter) {
    char fname[64];

    if (url_queryParse2(urlParameter, "fname", fname, 63) == 0) {
        Send500_InternalServerError(req, "parameter 'fname' is required");
        return;
    }

    ff_Delete(fname);
    Log("DeleteFile: '%s' OK\r\n", fname);
    Send200_OK_Simple(req);
}

void GET_version(HTTP_REQUEST *req, const char *urlParameter) {
    Send200_OK_SmallMsg(req, Version);

    return;
}

void GET_FlashStats(HTTP_REQUEST *req, const char *urlParameter) {

    unsigned long FreeSpace, UsedSpace, TrimSpace;
    ff_GetUtilization(&FreeSpace, &TrimSpace, &UsedSpace);
    sprintf(msg, "%lu\r\n%lu\r\n%lu\r\n", FreeSpace, UsedSpace, TrimSpace);
    Send200_OK_SmallMsg(req, msg);
}

#define API_INTERFACE_COUNT    26

API_INTERFACE api_interfaces[API_INTERFACE_COUNT] = {
    {"/api/echo", &GET_echo, 0, &PUT_echo, 0, 0},
    {"/api/executeprofile", NULL, 0, &PUT_executeProfile, 0, 0},
    {"/api/terminateprofile", NULL, 0, &PUT_terminateProfile, 0, 0},
    {"/api/truncateprofile", NULL, 0, &PUT_truncateProfile, 0, 0},
    {"/api/runhistory", &GET_runHistory, 0, NULL, 0, 0},
    {"/api/profile", &GET_profile, 0, &PUT_profile, 0, 0},
    {"/api/temperaturetrend", &GET_temperatureTrend, 0, NULL, 0, 0},
    {"/api/temperature", &GET_temperature, 0, &PUT_temperature, 0, 0},
    {"/api/status", &GET_status, 0, NULL, 0, 0},
    {"/api/time", &GET_Time, 0, &GET_Time, 0, 0},
    {"/api/factorydefault", NULL, 0, &PUT_factorydefault, 0, 0},
    {"/api/wificonfig", NULL, 0, &PUT_WifiConfig, 0, 0},
    {"/api/restart", NULL, 0, &PUT_Restart, 0, 0},
    {"/api/format", NULL, 0, &PUT_Format, 0, 0},
    {"/api/updatecredentials", NULL, 0, &PUT_UpdateCredentials, 0, 0},
    {"/api/equipmentprofile", &GET_Equipment, 0, &PUT_Equipment, 0, 0},
    {"/api/setequipmentprofile", NULL, 0, &PUT_SetEquipment, 0, 0},
    {"/api/trimfilesystem", NULL, 0, &PUT_trimFileSystem, 0, 0},
    {"/api/deleteequipmentprofile", NULL, 0, &PUT_deleteEquipmentProfile, 0, 0},
    {"/api/deleteprofile", NULL, 0, &PUT_deleteProfile, 0, 0},
    {"/api/deleteinstance", NULL, 0, &PUT_deleteProfileInstance, 0, 0},
    {"/api/uploadfirmware", NULL, 0, &PUT_uploadfirmware, 0, 0},
    {"/api/version", &GET_version, 0, NULL, 0, 0},
    {"/api/flashstats", &GET_FlashStats, 0, NULL, 0, 0},
    {"/api/uploadfile", NULL, 0, &PUT_uploadFile, 0, 0},
    {"/api/deletefile", NULL, 0, &PUT_deleteFile, 0, 0}
};

API_INTERFACE * GetAPI(HTTP_REQUEST * req) {
    if (req->Resource == NULL) return NULL;

    char *cursor = req->Resource;
    char token;
    int resLen = 0;
    while (*cursor) {
        token = *(cursor++);
        if (token == '?' || token == 0) break;
        resLen++;
    }

    int idx;
    //Log("Get API Resource=%s\r\n", req->Resource);
    for (idx = 0; idx < API_INTERFACE_COUNT; idx++) {
        int iLen = strlen(api_interfaces[idx].InterfaceName);
        if (iLen == resLen) {
            if (strncmp(req->Resource, api_interfaces[idx].InterfaceName, iLen) == 0) {
                //Log("API Interface Found='%s'\r\n", api_interfaces[idx].InterfaceName);
                return &api_interfaces[idx];
            }
        }
    }
    //Log("Get API Resource == NULL!!!\r\n");

    return NULL;
}

void ProcessAPI(HTTP_REQUEST *req, API_INTERFACE * API) {
    if (req->Resource == NULL) return;

    char *urlParameter = req->Resource;
    while (*urlParameter) {
        if (*urlParameter == '?') {
            *urlParameter = 0;
            urlParameter++;
            break;
        } else {
            urlParameter++;
        }
    };

    if (API->CfgModeRequired) {
        if (Global_Config_Mode != 1) {
            Send500_InternalServerError(req, "Access to this interface is restricted to Configuration Mode Only");
            return;
        }
    }

    switch (req->Method) {
        case HTTP_METHOD_GET:
            if (API->OnGet != NULL) {
                API->OnGet(req, urlParameter);
            } else {
                Send501_NotImplemented(req);
            }
            break;
        case HTTP_METHOD_PUT:
            if (API->OnPut != NULL) {
                API->OnPut(req, urlParameter);
            } else {
                Send501_NotImplemented(req);
            }
            break;
        default:
            Send501_NotImplemented(req);
            break;
    }
}
