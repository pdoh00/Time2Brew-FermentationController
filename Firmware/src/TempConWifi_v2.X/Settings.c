#include <string.h>

#include "Base64.h"
#include <stdio.h>
#include "SystemConfiguration.h"
#include "fletcherChecksum.h"
#include "Settings.h"
#include "Http_API.h"
#include "ESP8266.h"
#include "TemperatureController.h"
#include "FlashFS.h"
#include "mDNS.h"
#include "uPnP.h"
#include "MD5.h"

void TrimWhiteSpace(char *inp);
int ESP_ConfigLoad(ESP8266_CONFIG *cfg, const char *filename);
int ESP_ConfigSave(ESP8266_CONFIG *cfg, const char *filename);

int CreateDefaultEquipmentProfile() {
    int res;
    ff_File handle;
    EQUIPMENT_PROFILE *eq = &globalstate.equipmentConfig;
    sprintf(eq->Name, "default");
    eq->Probe0Assignment = PROBE_ASSIGNMENT_PROCESS;
    eq->Probe1Assignment = PROBE_ASSIGNMENT_TARGET;
    eq->CoolMinTimeOff = 60;
    eq->CoolMinTimeOn = 60;
    eq->HeatMinTimeOff = 60;
    eq->HeatMinTimeOn = 60;
    eq->Process_Kp = 0.2;
    eq->Process_Ki = 0;
    eq->Process_Kd = 0;
    eq->RegulationMode = REGULATIONMODE_SimplePID;
    eq->Target_Kp = 0.2;
    eq->Target_Ki = 0;
    eq->Target_Kd = 0;
    eq->TargetOutput_Max = 80;
    eq->TargetOutput_Min = 0;
    eq->heatDifferential = 5; //5C or 9F
    eq->heatTransition = 5; //5C or 9F
    eq->coolDifferential = 5; //10C or 18F
    eq->coolTransition = 5; //10C or 18F
    eq->Target_D_AdaptiveBand = 25.0;
    eq->Target_D_FilterGain = 20.8776099;
    eq->Target_D_FilterCoeff = 0.9042035937;
    eq->Process_D_AdaptiveBand = 25.0;
    eq->Process_D_FilterGain = 20.8776099;
    eq->Process_D_FilterCoeff = 0.9042035937;
    eq->CheckSum = fletcher16((BYTE *) eq, sizeof (EQUIPMENT_PROFILE) - 2);

    res = ff_Delete("equip.0");
    res = ff_OpenByFileName(&handle, "equip.0", 1);
    if (res != FR_OK) {
        Log("Unable to Open Default Equipment file! RES=%s\r\n", Translate_DRESULT(res));
        return res;
    }
    int bytesWritten;
    res = ff_Append(&handle, (BYTE*) eq, sizeof (EQUIPMENT_PROFILE), &bytesWritten);
    if (res != FR_OK) {
        Log("Unable to Write EQUIPMENT_CONFIG file! RES=%s\r\n", Translate_DRESULT(res));
        return res;
    }
    ff_UpdateLength(&handle);

    return FR_OK;
}

static int CreateFactoryDefaultWifiConfig() {
    sprintf(ESP_Config.Name, "TEMPCON");
    sprintf(ESP_Config.UUID, "f14fe247-a3bb-4663-9875-edccbe0b3f35");
    ESP_Config.Mode = SOFTAP_MODE;
    ESP_Config.STA_Password[0] = 0;
    sprintf(ESP_Config.SSID, "TEMPCON");
    ESP_Config.Channel = 1;
    ESP_Config.EncryptionMode = AUTH_OPEN;
    ESP_Config.HA1[0] = 0;

    return ESP_ConfigSave(&ESP_Config, WiFiConfigFilename);
}

int SaveConfig() {
    Log("Starting Save Config\r\n");
    if (ESP_ConfigSave(&ESP_Config, WiFiConfigFilename) == 0) return 0;
    Log("Config Saved OK!\r\n");
    return 1;
}

int ESP_ConfigLoad(ESP8266_CONFIG *cfg, const char *filename) {
    ff_File Handle;
    ff_File *handle = &Handle;
    int res;
    int bytesRead;

    res = ff_OpenByFileName(handle, filename, 0);
    if (res != FR_OK) {
        Log("Unable to Open WifiConfig file! RES=%s\r\n", Translate_DRESULT(res));
        return 0;
    }

    res = ff_Read(handle, (BYTE*) cfg, sizeof (ESP8266_CONFIG), &bytesRead);
    if (res != FR_OK) {
        Log("Unable to Read WifiConfig file! RES=%s\r\n", Translate_DRESULT(res));
        return 0;
    }

    if (strlen(cfg->Name) == 0 || strlen(cfg->Name) > 62) {
        Log("Name is not OK\r\n");
        return 0;
    }
    if (strlen(cfg->STA_Password) > 30) {
        Log("Password is not OK\r\n");
        return 0;
    }
    if (strlen(cfg->SSID) == 0 || strlen(cfg->SSID) > 30) {
        Log("SSID is not OK\r\n");
        return 0;
    }
    if (cfg->Channel < 0 || cfg->Channel > 11) {
        Log("Channel is not OK\r\n");
        return 0;
    }
    if (cfg->EncryptionMode != AUTH_WPA2_PSK && cfg->EncryptionMode != AUTH_OPEN &&
            cfg->EncryptionMode != AUTH_WPA_PSK && cfg->EncryptionMode != AUTH_WPA_WPA2_PSK) {
        Log("EncryptionMode is not OK\r\n");
        return 0;
    }
    if (cfg->Mode != STATION_MODE && cfg->Mode != SOFTAP_MODE) {
        Log("Mode is not OK\r\n");
        return 0;
    }
    return 1;
}

int ESP_ConfigSave(ESP8266_CONFIG *cfg, const char *filename) {
    ff_File Handle;
    ff_File *handle = &Handle;
    int res;
    int bytesWritten;

    res = ff_Delete(filename);
    res = ff_OpenByFileName(handle, filename, 1);
    if (res != FR_OK) {
        Log("Unable to Open WifiConfig file! RES=%s\r\n", Translate_DRESULT(res));
        return res;
    }

    res = ff_Append(handle, (BYTE*) cfg, sizeof (ESP8266_CONFIG), &bytesWritten);
    if (res != FR_OK) {
        Log("Unable to Write WifiConfig file! RES=%s\r\n", Translate_DRESULT(res));
        return res;
    }

    return FR_OK;
}

int GlobalStartup(int configMode) {
    ff_File defaultEquip;
    int res;

    ff_SPI_initialize();

    res = ff_Initialize();
    if (res == FR_NOT_FORMATTED) {
        Log("File System is Not Formatted...Format Starting\r\n");
        ff_Format();
        if (ff_Initialize() != FR_OK) {
            Log("GlobalStartup: Fatal Error: Unable to Format File System = \"%s\"", Translate_DRESULT(res));
            while (1);
        }
    }

    res = ff_CheckFS();
    if (res != FR_OK) {
        Log("File System Is Corrupted. Attempting to repair...\r\n");
        res = ff_RepairFS();
        if (res != FR_OK) {
            Log("Unable to Repair the File System. Format and try again...\r\n");
            ff_Format();
            if (ff_Initialize() != FR_OK) {
                Log("GlobalStartup: Fatal Error: Unable to Format File System = \"%s\"", Translate_DRESULT(res));
                while (1);
            }
        }
    }
    Log("File system is clean\r\n");

    Log("\r\nRead Unique Key...");
    unsigned char uid[8], hashUid[16];
    MD5_CTX hashContext;

    diskReadUniqueID(uid);

    MD5_Init(&hashContext);
    MD5_Update(&hashContext, uid, 8);
    MD5_Final((char *) hashUid, &hashContext);

    uid[0] = (hashUid[0] ^ hashUid[1]) ^ hashUid[12];
    uid[1] = (hashUid[2] ^ hashUid[3]) ^ hashUid[13];
    uid[2] = (hashUid[4] ^ hashUid[5]) ^ hashUid[14];
    uid[3] = (hashUid[6] ^ hashUid[7]) ^ hashUid[15];
    uid[4] = (hashUid[8] ^ hashUid[9]) ^ hashUid[1];
    uid[5] = (hashUid[10] ^ hashUid[11]) ^ hashUid[0];

    char ConfigPassword[9];
    sprintf_Base64(ConfigPassword, uid, 6);
    ConfigPassword[8] = 0;

    uid[0] = 0xe0;
    uid[0] = 0xc4;
    uid[0] = 0x71;
    uid[0] = 0xa1;
    uid[0] = 0x88;
    uid[0] = 0xe1;
    uid[0] = 0x44;
    uid[0] = 0xe5;
    MD5_Init(&hashContext);
    MD5_Update(&hashContext, uid, 8);
    MD5_Update(&hashContext, hashUid, 16);
    MD5_Final((char *) hashUid, &hashContext);

    char UUID[40];
    sprintf(UUID, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            (unsigned int) hashUid[0],
            (unsigned int) hashUid[1],
            (unsigned int) hashUid[2],
            (unsigned int) hashUid[3],
            (unsigned int) hashUid[4],
            (unsigned int) hashUid[5],
            (unsigned int) hashUid[6],
            (unsigned int) hashUid[7],
            (unsigned int) hashUid[8],
            (unsigned int) hashUid[9],
            (unsigned int) hashUid[10],
            (unsigned int) hashUid[11],
            (unsigned int) hashUid[12],
            (unsigned int) hashUid[13],
            (unsigned int) hashUid[14],
            (unsigned int) hashUid[15]);

    Log("Config Password='%s' UUID='%s'\r\n", ConfigPassword, UUID);


    Log("Loading ESP Config...");
    if (ESP_ConfigLoad(&ESP_Config, WiFiConfigFilename) == 0) {
        if (CreateFactoryDefaultWifiConfig() != FR_OK) {
            Log("Unable to create Default Wifi Config...\r\n");
            while (1);
        }
    }
    Log(" OK\r\n");

    sprintf(ESP_Config.UUID, "%s", UUID);
    sprintf(ESP_Config.SOFTAP_Password, "%s", ConfigPassword);

    if (strlen(ESP_Config.STA_Password) == 0) sprintf(ESP_Config.STA_Password, "%s", ConfigPassword);

    Log("Checing HA1...");
    if (strlen(ESP_Config.HA1) != 32) {
        Log("HA1 Value is corrupt! Creating default username and password...username='user' Password='%s'\r\n", ConfigPassword);
        UpdateCredentials("user", ConfigPassword);
    }
    Log("OK\r\n");

    if (configMode) {
        Log("CONFIG MODE DETECTED\r\n");
        Global_Config_Mode = 1;
        if (strlen(ESP_Config.Name) == 0) {
            sprintf(ESP_Config.Name, "defaultConfig");
        }
        ESP_Config.EncryptionMode = AUTH_WPA2_PSK;
        ESP_Config.Mode = SOFTAP_MODE;
        sprintf(ESP_Config.STA_Password, "%s", ESP_Config.SOFTAP_Password);
        sprintf(ESP_Config.SSID, "CFG_%s", ESP_Config.Name);
        ESP_Config.Channel = 1;
        Log("***SSID='%s' Password='%s'\r\n", ESP_Config.SSID, ESP_Config.STA_Password);
    } else {
        Global_Config_Mode = 0;
    }

    Log("\r\nInitializing WiFi Module\r\n");
    if (ESP_Init() != 1) {
        Log("Unable to initialize the ESP Module...");
        return -1;
    }
    Log("   WiFI Module Ready. IP Address=%d.%d.%d.%d\r\n", IP_Address.b[3], IP_Address.b[2], IP_Address.b[1], IP_Address.b[0]);

    Log("Initializing mDNS Service\r\n");
    mDNS_Init(ESP_Config.Name, IP_Address.l);
    Log("    mDNS Service Online\r\n");

    Log("\r\nInitializing uPnP Service\r\n");
    uPnP_Init(ESP_Config.Name, ESP_Config.UUID, IP_Address.l, 1);
    Log("   uPnP Service Online\r\n");

    res = ff_OpenByFileName(&defaultEquip, "equip.0", 0);
    if (res == FR_NOT_FOUND) {
        if (CreateDefaultEquipmentProfile() != FR_OK) {
            Log("GlobalStartup: Fatal Error: Unable to Create 'equip.default' = \"%s\"", Translate_DRESULT(res));
            while (1);
        }
    } else if (res != FR_OK) {
        Log("GlobalStartup: Fatal Error: Unable to Open 'equip.default' = \"%s\"", Translate_DRESULT(res));
        while (1);
    }

    Log("\r\nInitializing Temperature Controller\r\n");
    TemperatureController_Initialize();
    Log("   Temperature Controller Online\r\n");
    return FR_OK;
}
