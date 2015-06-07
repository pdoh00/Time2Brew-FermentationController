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

void BuildProfileInstanceListing(const char *ProfileID) {
    ff_File DirInfo, CacheFile;
    int bWritten;
    int res;
    char Filter[128];
    char Filename[128];
    sprintf(Filename, "prflinstlisting.%s", ProfileID);
    sprintf(Filter, "inst.%s.", ProfileID);
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
    char filename[128];
    char ProfileName[64];
    ff_File DirInfo, CacheFile, thisProfile;
    int bWritten;
    int res;

    Log("BuildProfileListing\r\n");

    res = ff_Delete("ProfileListing.txt");
    res = ff_OpenDirectoryListing(&DirInfo);
    res = ff_OpenByFileName(&CacheFile, "ProfileListing.txt", 1);

    while (1) {
        res = ff_GetNextEntryFromDirectory(&DirInfo, filename);
        if (res != FR_OK) {
            ff_UpdateLength(&CacheFile);
            Log("Done\r\n");
            return;
        }
        if (memcmp("prfl.", filename, 5) == 0) {
            res = ff_OpenByFileName(&thisProfile, filename, 0);
            if (res != FR_OK) continue;
            res = ff_Read(&thisProfile, (BYTE *) ProfileName, 64, &bWritten);
            if (res != FR_OK) continue;
            sprintf(msg, "%s,%s\r\n", filename + 5, ProfileName);
            ff_Append(&CacheFile, (BYTE *) msg, strlen(msg), &bWritten);
            Log("   %s", msg);
        }
    }
}

void BuildEquipmentProfileListing() {
    ff_File DirInfo, CacheFile, equipProfile;
    char equipName[64];
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
            ff_OpenByFileName(&equipProfile, msg, 0);
            ff_Read(&equipProfile, (BYTE*) equipName, 64, &bWritten);
            sprintf(msg, "%s, %s\r\n", msg + 6, equipName);
            ff_Append(&CacheFile, (BYTE *) msg, strlen(msg), &bWritten);
            Log("  %s", msg);
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
    char strProfileID[7];

    if (url_queryParse2(urlParameter, "id", strProfileID, 7)) {
        sprintf(req->Resource, "/prfl.%s", strProfileID);
        req->ETag = NULL;
        req->ContentLength = 0;
        Process_GET_File(req, 0);
    } else {
        if (ff_exists("ProfileListing.txt") == 0) {
            BuildProfileListing();
        }

        sprintf(req->Resource, "/ProfileListing.txt");
        req->ETag = NULL;
        req->ContentLength = 0;
        Process_GET_File(req, 0);
    }
}

void PUT_profile(HTTP_REQUEST *req, const char *urlParameter) {
    char IsFinal;
    char strProfileID[7];
    unsigned int profileID;
    ff_File handle;
    int bWritten;
    int res;
    int offset;
    char *b64_Content;
    BYTE *ContentData;
    int ContentLength;
    int b64_ContentLength;
    unsigned int chkSum;


    if (url_queryParse2(urlParameter, "isfinal", strProfileID, 2)) {
        if (strProfileID[0] == 'n') IsFinal = 0;
        else IsFinal = 1;
    } else {
        IsFinal = 1;
    }

    if (url_queryParse2(urlParameter, "offset", strProfileID, 6)) {
        if (sscanf(strProfileID, "%d", &offset) != 1) {
            sprintf(msg, "Error Offset was invalid: offset=\"%s\"", strProfileID);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        offset = 0;
    }

    if (url_queryParse2(urlParameter, "chksum", strProfileID, 6)) {
        if (sscanf(strProfileID, "%u", &chkSum) != 1) {
            sprintf(msg, "Error chksum could not parse: chksum=\"%s\"", strProfileID);
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

    if (url_queryParse2(urlParameter, "id", strProfileID, 6) == 0) {
        if (offset != 0) {
            Send500_InternalServerError(req, "Query Parameter 'id' was not provided but offset!=0 so this is not a profile creation call.");
            return;
        }

        do {
            profileID = (unsigned int) (TMR4 & 0xFFFF);
            sprintf(strProfileID, "%u", profileID);
        } while (ff_exists(strProfileID));

    }
    sprintf(msg, "prfl.%s", strProfileID);


    if (offset == 0) res = ff_Delete(msg);

    res = ff_OpenByFileName(&handle, msg, 1);
    if (res != FR_OK) {
        sprintf(msg, "Error Creating Profile File: res=%d", res);
        Send500_InternalServerError(req, msg);
        return;
    }

    res = ff_Seek(&handle, offset);
    if (res != FR_OK) {
        sprintf(msg, "Error Seeking Profile File: res=%d", res);
        Send500_InternalServerError(req, msg);
        return;
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

        if (globalstate.SystemMode == SYSTEMMODE_Profile) {
            sscanf(strProfileID, "%u", &profileID);
            if (globalstate.ProfileID == profileID) {
                GetProfileName(strProfileID, globalstate.ActiveProfileName);
            }
        }

        BuildProfileListing();
    }
    Send200_OK_SmallMsg(req, strProfileID);

    return;
}

void PUT_executeProfile(HTTP_REQUEST *req, const char *urlParameter) {
    if (globalstate.SystemMode != SYSTEMMODE_IDLE) {
        sprintf(msg, "Controller is not IDLE");
        Send500_InternalServerError(req, msg);
        return;
    }
    char strProfileID[7];
    unsigned int ProfileID;
    if (url_queryParse2(urlParameter, "id", strProfileID, 7)) {
        sscanf(strProfileID, "%u", &ProfileID);
        if (ExecuteProfile(ProfileID, msg) != 1) {
            Send500_InternalServerError(req, msg);
        } else {
            BuildProfileInstanceListing(strProfileID);
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

void GET_instanceSteps(HTTP_REQUEST *req, const char *urlParameter) {
    char strProfileID[7];
    char strInstance[14];

    if (url_queryParse2(urlParameter, "id", strProfileID, 7)) {
        if (url_queryParse2(urlParameter, "instance", strInstance, 13)) {
            sprintf(req->Resource, "/inst.%s.%s", strProfileID, strInstance);
            req->ETag = NULL;
            req->ContentLength = 0;
            Log("Passing to Get_File Resource=%s\r\n", req->Resource);
            Process_GET_File(req, 0);
        } else {
            sprintf(req->Resource, "prflinstlisting.%s", strProfileID);
            if (ff_exists(req->Resource) == 0) {
                BuildProfileInstanceListing(strProfileID);
            }
            //No Instance provided so send a list of instances
            sprintf(req->Resource, "/prflinstlisting.%s", strProfileID);
            req->ETag = NULL;
            req->ContentLength = 0;
            Process_GET_File(req, 0);
            return;
        }
    } else {
        //No name so send a list of profiles...
        sprintf(req->Resource, "/ProfileListing.txt");
        req->ETag = NULL;
        req->ContentLength = 0;
        Process_GET_File(req, 0);

        return;
    }
}

void GET_instanceTrend(HTTP_REQUEST *req, const char *urlParameter) {
    char strProfileID[7];
    char strInstance[14];
    unsigned int profileID;

    if (url_queryParse2(urlParameter, "id", strProfileID, 7)) {
        sscanf(strProfileID, "%u", &profileID);
        if (url_queryParse2(urlParameter, "instance", strInstance, 13)) {
            unsigned long sendLength;

            if ((globalstate.SystemMode == SYSTEMMODE_Profile) &&
                    (globalstate.ProfileID == profileID)) {
                RLE_Flush(&globalstate.trend_RLE_State);
                sendLength = globalstate.trend_RLE_State.sizeofSample + 1;
                sendLength *= globalstate.trend_RLE_State.packetCount;
            } else {
                sendLength = 0; //Tell Process_GET_File_ex to send the file Length as it is recorded on disk...
            }

            sprintf(req->Resource, "/trnd.%s.%s", strProfileID, strInstance);
            req->ETag = NULL;
            req->ContentLength = 0;
            Log("Passing to Get_File Resource=%s\r\n", req->Resource);
            Process_GET_File_ex(req, 0, sendLength, 0);
        } else {
            //No Instance provided so send a list of instances
            sprintf(req->Resource, "prflinstlisting.%s", strProfileID);
            if (ff_exists(req->Resource) == 0) {
                BuildProfileInstanceListing(strProfileID);
            }

            sprintf(req->Resource, "/prflinstlisting.%s", strProfileID);
            req->ETag = NULL;
            req->ContentLength = 0;
            Process_GET_File(req, 0);
            return;
        }
    } else {
        //No name so send a list of profiles...

        sprintf(req->Resource, "/ProfileListing.txt");
        req->ETag = NULL;
        req->ContentLength = 0;
        Process_GET_File(req, 0);
    }
}

void GET_temperature(HTTP_REQUEST *req, const char *urlParameter) {
    Log("%d: GET_temperature urlParameter=%s\r\n", req->TCP_ChannelID, urlParameter);
    char ProbeID[3];
    if (url_queryParse2(urlParameter, "probe", ProbeID, 2)) {
        if (ProbeID[0] == '0') {
            sprintf(msg, "%d\r\n", (int) (globalstate.ProcessTemperature * 100));
            Send200_OK_SmallMsg(req, msg);
        } else if (ProbeID[0] == '1') {
            sprintf(msg, "%d\r\n", (int) (globalstate.TargetTemperature * 100));
            Send200_OK_SmallMsg(req, msg);
        } else {
            sprintf(msg, "%d\r\n%d\r\n", (int) (globalstate.ProcessTemperature * 100), (int) (globalstate.TargetTemperature * 100));
            Send200_OK_SmallMsg(req, msg);
        }
    } else {

        sprintf(msg, "%d\r\n%d\r\n", (int) (globalstate.ProcessTemperature * 100), (int) (globalstate.TargetTemperature * 100));
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
    unsigned char *cursour = Pack(packedData, "lbbbfbfbbaiIlIlallllBffffll", temp.SystemTime, temp.SystemMode,
            temp.equipmentConfig.RegulationMode, temp.equipmentConfig.Probe0Assignment, temp.TargetTemperature,
            temp.equipmentConfig.Probe1Assignment, temp.ProcessTemperature, temp.HeatRelay,
            temp.CoolRelay, temp.ActiveProfileName, len, temp.StepIdx,
            temp.StepTemperature, temp.StepTimeRemaining,
            temp.ManualSetPoint, temp.ProfileStartTime, temp.equipmentConfig.Name, len,
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
        if (Setpoint<-250 || Setpoint > 1250) {
            Send500_InternalServerError(req, "Error: 'setpoint' is outside of bounds");
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
    MD5_Update(&ctx, buff, strlen(buff));
    MD5_Final(ESP_Config.HA1, &ctx);
    Log("HA1=%s\r\n", ESP_Config.HA1);
    if (SaveConfig()) {
        return 1;
    } else {

        return 0;
    }
}

void GET_PUT_Time(HTTP_REQUEST *req, const char *urlParameter) {
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

void PUT_EquipmentProfile(HTTP_REQUEST *req, const char *urlParameter) {
    char IsFinal;
    unsigned int offset;
    char temp[128];
    char strEquipID[65];
    unsigned int EquipID;

    char filename[120];
    int bWritten, res;
    ff_File file;
    EQUIPMENT_PROFILE dummy;
    char *b64_Content;
    BYTE *ContentData;
    int ContentLength;
    int b64_ContentLength;
    unsigned int chkSum;


    if (url_queryParse2(urlParameter, "offset", strEquipID, 6)) {
        if (sscanf(strEquipID, "%d", &offset) != 1) {
            sprintf(msg, "Error Offset was invalid: offset=\"%s\"", strEquipID);
            Send500_InternalServerError(req, msg);
            return;
        }
    } else {
        offset = 0;
    }

    if (url_queryParse2(urlParameter, "isfinal", strEquipID, 2)) {
        if (strEquipID[0] == 'n') IsFinal = 0;
        else IsFinal = 1;
    } else {
        IsFinal = 1;
    }


    if (url_queryParse2(urlParameter, "id", strEquipID, 64) == 0) {
        if (offset != 0) {
            Send500_InternalServerError(req, "Query Parameter 'id' was not provided but offset!=0 so this is not a profile creation call.");
            return;
        }

        do {
            EquipID = (unsigned int) (TMR4 & 0xFFFF);
            sprintf(filename, "equip.%u", EquipID);
        } while (ff_exists(filename));
    } else {
        sprintf(filename, "equip.%s", strEquipID);
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

    if (offset == 0) ff_Delete(filename);

    res = ff_OpenByFileName(&file, filename, 1);
    if (res != FR_OK) {
        sprintf(msg, "Unable to create Equipment Profile: Error=%s", Translate_DRESULT(res));
        Send500_InternalServerError(req, msg);
        return;
    }

    if (offset > 0) {
        res = ff_Seek(&file, offset);
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

void GET_EquipmentProfile(HTTP_REQUEST *req, const char *urlParameter) {
    char strEquipID[7];
    if (url_queryParse2(urlParameter, "id", strEquipID, 7)) {
        sprintf(req->Resource, "/equip.%s", strEquipID);
        req->ETag = NULL;
        req->ContentLength = 0;
        Log("Passing to Get_File Resource=%s\r\n", req->Resource);
        Process_GET_File(req, 0);
    } else {
        if (ff_exists("EquipmentProfileListing.txt") == 0) {
            BuildEquipmentProfileListing();
        }
        sprintf(req->Resource, "/EquipmentProfileListing.txt");
        req->ETag = NULL;
        req->ContentLength = 0;
        Process_GET_File(req, 0);

        return;
    }
}

void PUT_SetEquipment(HTTP_REQUEST *req, const char *urlParameter) {
    char strEquipID[7];
    unsigned int equipmentID;
    int ret;
    if (url_queryParse2(urlParameter, "id", strEquipID, 7)) {
        sprintf(req->Resource, "equip.%s", strEquipID);
        sscanf(strEquipID, "%u", &equipmentID);
        ret = SetEquipmentProfile(equipmentID, msg, &globalstate);
        if (ret != FR_OK) {
            Send500_InternalServerError(req, msg);
            return;
        }
        Send200_OK_Simple(req);
        return;
    } else {
        Send500_InternalServerError(req, "the 'id' parameter must be provided.");

        return;
    }
}

void PUT_trimFileSystem(HTTP_REQUEST *req, const char *urlParameter) {
    int res;
    res = ff_RepairFS();
    if (res != FR_OK) {

        sprintf(msg, "Trim Failed Res='%s'", Translate_DRESULT(res));
        Send500_InternalServerError(req, msg);
    }

    Send200_OK_Simple(req);
}

void GET_rawSector(HTTP_REQUEST *req, const char *urlParameter) {
    Log("GET_rawSector...");
    int sector;
    char strSector[32];
    unsigned char rawBuff[256];

    if (url_queryParse2(urlParameter, "sector", strSector, 16)) {
        if (sscanf(strSector, "%d", &sector) != 1) {
            Send500_InternalServerError(req, "Error: 'sector' is invalid");
            return;
        }
    } else {
        Send500_InternalServerError(req, "Error: 'sector' is not provided");
        return;
    }

    Log("%xi...", sector);

    ESP_TCP_StartStream(req->TCP_ChannelID);

    circularPrintf(txFIFO,
            "HTTP/1.1 200 OK\r\n"
            "Connection: Keep-Alive\r\n"
            "Content-Type:  application/octet-stream\r\n"
            "Content-Length: 4096\r\n"
            "\r\n");
    _U1TXIF = 1;
    int x;
    unsigned long address;
    address = sector;
    address <<= 12;

    for (x = 0; x < 16; x++) {
        diskRead(address, 256, rawBuff);
        ESP_StreamArray(rawBuff, 256);
        address += 256;
    }

    if (ESP_TCP_TriggerWiFi_Send(req->TCP_ChannelID) < 0) {
        Log("Unable to Send Wifi Data...\r\n");
        return;
    }

    if (!ESP_TCP_Wait_WiFi_SendCompleted(req->TCP_ChannelID)) {
        ESP_TCP_CloseConnection(req->TCP_ChannelID);
        Log("GET_rawSectorFailed!\r\n");
    } else {

        Log("OK\r\n");
    }

}

void PUT_deleteProfileInstance(HTTP_REQUEST *req, const char *urlParameter) {
    char strProfileID[7];
    char strInstance[14];
    int res;

    if (url_queryParse2(urlParameter, "id", strProfileID, 7)) {
        if (url_queryParse2(urlParameter, "instance", strInstance, 13)) {
            sprintf(req->Resource, "inst.%s.%s", strProfileID, strInstance);
            if ((globalstate.SystemMode == SYSTEMMODE_Profile) &&
                    (strcmp(globalstate.Profile.FileName, req->Resource) == 0)) {
                Send500_InternalServerError(req, "Error: Unable to delete the currently running instance");
                return;
            }

            sprintf(msg, "trnd.%s.%s", strProfileID, strInstance);
            res = ff_Delete(msg);
            if (res != FR_OK) {
                sprintf(msg, "Error: Unable to delete Trend Record: res='%s'", Translate_DRESULT(res));
                Send500_InternalServerError(req, msg);
                return;
            }
            sprintf(msg, "inst.%s.%s", strProfileID, strInstance);
            res = ff_Delete(msg);
            if (res != FR_OK) {
                sprintf(msg, "Error: Unable to delete Instance Record: res='%s'", Translate_DRESULT(res));
                Send500_InternalServerError(req, msg);
                return;
            }

            BuildProfileInstanceListing(strProfileID);

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
    char strProfileID[7];
    int res;
    if (url_queryParse2(urlParameter, "id", strProfileID, 7) == 0) {
        sprintf(msg, "Error: 'id' is requried");
        Send500_InternalServerError(req, msg);
        return;
    }

    sprintf(req->Resource, "inst.%s.", strProfileID);

    if ((globalstate.SystemMode == SYSTEMMODE_Profile) &&
            (strstr(globalstate.Profile.FileName, req->Resource) != NULL)) {
        Send500_InternalServerError(req, "Error: Unable to delete the currently running profile.");
        return;
    }

    ff_File DirInfo;
    char instanceFilter[128];
    char trendFilter[128];
    sprintf(msg, "prflinstlisting.%s", strProfileID);
    sprintf(instanceFilter, "inst.%s.", strProfileID);
    sprintf(trendFilter, "trnd.%s.", strProfileID);
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
    sprintf(msg, "prfl.%s", strProfileID);
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
    char strEquipID[7];
    unsigned int equipID;
    int res;

    if (url_queryParse2(urlParameter, "id", strEquipID, 7) == 0) {
        sprintf(msg, "Error: 'ID' is requried");
        Send500_InternalServerError(req, msg);
        return;
    }

    sscanf(strEquipID, "%u", &equipID);

    if (globalstate.equipmentProfileID == equipID) {
        Send500_InternalServerError(req, "Error: Unable to delete the active Equipment Profile");
        return;
    }

    sprintf(req->Resource, "equip.%s", strEquipID);
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
    char temp[32];
    unsigned long offset;
    char *b64_Content;
    BYTE *ContentData;
    int ContentLength;
    int b64_ContentLength;
    unsigned int chkSum;

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
    if (offset == 0) {
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

    if (url_queryParse2(urlParameter, "offset", temp, 12)) {
        if (sscanf(temp, "%lu", &offset) != 1) {
            sprintf(msg, "Error Offset was invalid: offset=\"%s\"", temp);
            Send500_InternalServerError(req, msg);
            return;
        }
        if (offset == 0) {
            ff_Delete(fname);
            ret = ff_OpenByFileName(&handle, fname, 1);
            if (ret != FR_OK) {
                sprintf(msg, "Error: Unable to open/create file: res='%s'", Translate_DRESULT(ret));
                Send500_InternalServerError(req, msg);
                return;
            }
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

    ret = ff_Seek(&handle, offset);
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

    Log("UploadFile: '%s' Offset=%ul finalize='%s'...OK\r\n", fname, offset, temp);
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

void GET_SecurityTest(HTTP_REQUEST *req, const char *urlParameter) {

    Send200_OK_SmallMsg(req, "Secure!!!");
}

void GET_EraseBlob(HTTP_REQUEST *req, const char *urlParameter) {

    Send200_OK_Simple(req);
    unsigned long baseAddress;
    Log("Erasing Blob...");
    for (baseAddress = BLOB_START_ADDRESS; baseAddress < (BLOB_START_ADDRESS + BLOB_LENGTH); baseAddress += 4096) {
        diskEraseSector(baseAddress);
    }
    Log("OK\r\n");
}

void GET_UploadBlob(HTTP_REQUEST *req, const char *urlParameter) {
    char temp[32];
    unsigned long offset;
    char *b64_Content;
    BYTE *ContentData;
    int ContentLength;
    int b64_ContentLength;
    unsigned int chkSum;

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

    unsigned long limit = BLOB_LENGTH;

    if (offset + ContentLength > limit) {
        Send500_InternalServerError(req, "Blob Length exceeds allowable size...");
        return;
    }

    unsigned long baseAddress;
    baseAddress = BLOB_START_ADDRESS;
    baseAddress += offset;
    diskWrite(baseAddress, ContentLength, ContentData);

    Send200_OK_Simple(req);

    Log("UploadBLOB: Offset=%ul Length=%i OK\r\n", offset, ContentLength);
}

#define API_INTERFACE_COUNT    30

API_INTERFACE api_interfaces[API_INTERFACE_COUNT] = {
    {"/api/echo", &GET_echo, 0, &PUT_echo, 0, 0},
    {"/api/executeprofile", NULL, 0, &PUT_executeProfile, 0, 0},
    {"/api/terminateprofile", NULL, 0, &PUT_terminateProfile, 0, 0},
    {"/api/truncateprofile", NULL, 0, &PUT_truncateProfile, 0, 0},
    {"/api/runhistory", &GET_instanceSteps, 0, NULL, 0, 0},
    {"/api/profile", &GET_profile, 0, &PUT_profile, 0, 0},
    {"/api/temperaturetrend", &GET_instanceTrend, 0, NULL, 0, 0},
    {"/api/temperature", &GET_temperature, 0, &PUT_temperature, 0, 0},
    {"/api/status", &GET_status, 0, NULL, 0, 0},
    {"/api/time", &GET_PUT_Time, 0, &GET_PUT_Time, 0, 0},
    {"/api/factorydefault", NULL, 0, &PUT_factorydefault, 0, 0},
    {"/api/wificonfig", NULL, 0, &PUT_WifiConfig, 0, 0},
    {"/api/restart", NULL, 0, &PUT_Restart, 0, 0},
    {"/api/format", NULL, 0, &PUT_Format, 0, 0},
    {"/api/updatecredentials", NULL, 0, &PUT_UpdateCredentials, 0, 0},
    {"/api/equipmentprofile", &GET_EquipmentProfile, 0, &PUT_EquipmentProfile, 0, 0},
    {"/api/setequipmentprofile", NULL, 0, &PUT_SetEquipment, 0, 0},
    {"/api/trimfilesystem", NULL, 0, &PUT_trimFileSystem, 0, 0},
    {"/api/deleteequipmentprofile", NULL, 0, &PUT_deleteEquipmentProfile, 0, 0},
    {"/api/deleteprofile", NULL, 0, &PUT_deleteProfile, 0, 0},
    {"/api/deleteinstance", NULL, 0, &PUT_deleteProfileInstance, 0, 0},
    {"/api/uploadfirmware", NULL, 0, &PUT_uploadfirmware, 0, 0},
    {"/api/version", &GET_version, 0, NULL, 0, 0},
    {"/api/flashstats", &GET_FlashStats, 0, NULL, 0, 0},
    {"/api/uploadfile", NULL, 0, &PUT_uploadFile, 0, 0},
    {"/api/deletefile", NULL, 0, &PUT_deleteFile, 0, 0},
    {"/api/rawsector", &GET_rawSector, 0, &GET_rawSector, 0, 0},
    {"/api/sectest", &GET_SecurityTest, 1, &GET_SecurityTest, 1, 0},
    {"/api/eraseblob", &GET_EraseBlob, 1, &GET_EraseBlob, 0, 0},
    {"/api/uploadblob", &GET_UploadBlob, 1, &GET_UploadBlob, 0, 0}
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
