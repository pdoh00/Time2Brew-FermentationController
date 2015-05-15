#include <string.h>
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

void TrimWhiteSpace(char *inp);
int ESP_ConfigLoad(ESP8266_CONFIG *cfg, const char *filename);
int ESP_ConfigSave(ESP8266_CONFIG *cfg, const char *filename);

int CreateDefaultEquipmentProfile() {
    int res;
    ff_File handle;    
    EQUIPMENT_PROFILE *eq = &globalstate.equipmentConfig;
    eq->Probe0Assignment = PROBE_ASSIGNMENT_PROCESS;
    eq->Probe1Assignment = PROBE_ASSIGNMENT_TARGET;
    eq->CoolMinTimeOff = 60;
    eq->CoolMinTimeOn = 60;
    eq->HeatMinTimeOff = 60;
    eq->HeatMinTimeOn = 60;
    eq->Process_Kp = 1;
    eq->Process_Ki = 0;
    eq->Process_Kd = 0;
    eq->RegulationMode = REGULATIONMODE_SimplePID;
    eq->Target_Kp = 1;
    eq->Target_Ki = 0;
    eq->Target_Kd = 0;
    eq->TargetOutput_Max = 1000;
    eq->TargetOutput_Min = 0;
    eq->ThresholdDelta = 5;
    eq->CheckSum = fletcher16((BYTE *) eq, sizeof (EQUIPMENT_PROFILE) - 2);

    Log("Equip Size=%i Checksum=%ui\r\n", sizeof (EQUIPMENT_PROFILE), eq->CheckSum);

    res = ff_Delete("equip.default");
    res = ff_OpenByFileName(&handle, "equip.default", 1);
    if (res != FR_OK) {
        Log("Unable to Open WifiConfig file! RES=%s\r\n", Translate_DRESULT(res));
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

static unsigned char GetRandomByte() {
    unsigned char rnd;
    while (1) {
        DISABLE_INTERRUPTS;
        if (TRNG_fifo->Read != TRNG_fifo->Write) {
            ENABLE_INTERRUPTS;
            break;
        }
        ENABLE_INTERRUPTS;
        DELAY_5uS;
    };
    FIFO_Read(TRNG_fifo, rnd);
    return rnd;
}

static int CreateUnqiueKey() {
    unsigned char rnd;
    unsigned char secBuff[256];
    diskEraseSecure(1);

    //    Log("A new key is needed: Short Press the CFG button to continue\r\n");
    //    if (!CFG_MODE_PORT) while (!CFG_MODE_PORT);
    //    while (CFG_MODE_PORT);

    Log("Setting Magic\r\n");
    memset(secBuff, 0xFF, 256);
    secBuff[0] = 0x12;
    secBuff[1] = 0x34;

    Log("Build WiFi Password...");
    char *cursor = (char *) &secBuff[4];
    int x;
    for (x = 0; x < 8; x++) {
        rnd = GetRandomByte();
        cursor += sprintf(cursor, "%02x", rnd ^ (TMR4 & 0xFF));
    }
    *(cursor++) = 0;
    Log("OK\r\n");

    Log("Build GUID Part 1...");
    for (x = 0; x < 4; x++) {
        rnd = GetRandomByte();
        cursor += sprintf(cursor, "%02x", rnd ^ (TMR4 & 0xFF));
    }
    *(cursor++) = '-';
    Log("OK\r\n");

    Log("Build GUID Part 2...");
    for (x = 0; x < 2; x++) {
        rnd = GetRandomByte();
        cursor += sprintf(cursor, "%02x", rnd ^ (TMR4 & 0xFF));
    }
    *(cursor++) = '-';
    Log("OK\r\n");

    Log("Build GUID Part 3...");
    for (x = 0; x < 2; x++) {
        rnd = GetRandomByte();
        cursor += sprintf(cursor, "%02x", rnd ^ (TMR4 & 0xFF));
    }
    *(cursor++) = '-';
    Log("OK\r\n");

    Log("Build GUID Part 4...");
    for (x = 0; x < 2; x++) {
        rnd = GetRandomByte();
        cursor += sprintf(cursor, "%02x", rnd ^ (TMR4 & 0xFF));
    }
    *(cursor++) = '-';
    Log("OK\r\n");

    Log("Build GUID Part 5...");
    for (x = 0; x < 6; x++) {
        rnd = GetRandomByte();
        cursor += sprintf(cursor, "%02x", rnd ^ (TMR4 & 0xFF));
    }
    *(cursor++) = 0;
    Log("OK\r\n");

    Log("Write Secure Sector...");
    diskWriteSecure(1, secBuff);
    Log("OK\r\n");
    return 1;
}

int Load_Cfg_Mode_Config(void) {
    int res;
    ff_File file;
    Log("Check For equip.default file\r\n");
    res = ff_OpenByFileName(&file, "equip.default", 0);
    if (res != FR_OK) {
        Log("Equipment File Not Found: Creating the Default Value\r\n");
        res = CreateDefaultEquipmentProfile();
        if (res != FR_OK) return res;
    }

    Log("\r\nCheck Unique Key...");
    unsigned char secBuff[256];
    unsigned long *magic = (unsigned long *) &secBuff[0];

    diskReadSecure(1, secBuff);
    if (secBuff[0] == 0xFF || *magic != 0x12398764ul || strlen((char *) &secBuff[4]) != 16) {
        CreateUnqiueKey();
        diskReadSecure(1, secBuff);
    }
    Log(" OK\r\n");

    sprintf(ESP_Config.Name, "TEMPCONCONFIG");
    sprintf(ESP_Config.UUID, "f14fe247-a3bb-4663-9875-edccbe0b3f35");
    ESP_Config.EncryptionMode = AUTH_WPA2_PSK;
    ESP_Config.Mode = SOFTAP_MODE;
    sprintf(ESP_Config.Password, "%s", &secBuff[4]);
    sprintf(ESP_Config.SSID, "TEMPCONCONFIG");
    ESP_Config.Channel = 1;

    InitializeRecoveryRecord();

    Log("CONFIG_MODE! SSID='%s' Password='%s'\r\n", ESP_Config.SSID, ESP_Config.Password);
    return 1;
}

int CreateFactoryDefaultConfig() {
    Log("Factory Default Started\r\n");

    sprintf(ESP_Config.Name, "TEMPCON");
    sprintf(ESP_Config.UUID, "f14fe247-a3bb-4663-9875-edccbe0b3f35");
    ESP_Config.Mode = SOFTAP_MODE;
    ESP_Config.Password[0] = 0;
    sprintf(ESP_Config.SSID, "TEMPCON");
    ESP_Config.Channel = 1;
    ESP_Config.EncryptionMode = AUTH_OPEN;
    ESP_Config.HA1[0] = 0;

    CreateDefaultEquipmentProfile();

    UpdateCredentials("user", "pass");

    if (ESP_ConfigSave(&ESP_Config, WiFiConfigFilename) == 0) return 0;

    Log("    WiFi Config Defaulted\r\n");

    Log("Default Config Saved OK!\r\n");

    return 1;
}

static int CreateFactoryDefaultWifiConfig() {
    sprintf(ESP_Config.Name, "TEMPCON");
    sprintf(ESP_Config.UUID, "f14fe247-a3bb-4663-9875-edccbe0b3f35");
    ESP_Config.Mode = SOFTAP_MODE;
    ESP_Config.Password[0] = 0;
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
    if (strlen(cfg->Password) > 30) {
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
    unsigned char secBuff[256];
    char *ConfigPassword = (char *) &secBuff[4];
    char *UUID = (char *) &secBuff[21];

    diskReadSecure(1, secBuff);
    if (secBuff[0] != 0x12 || secBuff[1] != 0x34) {
        Log("!!!Secure MAGIC is BAD! - Create a new one\r\n");
        CreateUnqiueKey();
        Log("Key Created...Reading\r\n");
        diskReadSecure(1, secBuff);
    } else if (strlen(ConfigPassword) != 16) {
        Log("!!!Secure ConfigPassword is BAD! - Create a new one\r\n");
        CreateUnqiueKey();
        Log("Key Created...Reading\r\n");
        diskReadSecure(1, secBuff);
    } else if (strlen(UUID) != 36) {
        Log("!!!Secure UUID s BAD! len=%i - Create a new one\r\n", (int) strlen(UUID));
        CreateUnqiueKey();
        Log("Key Created...Reading\r\n");
        diskReadSecure(1, secBuff);
    }
    Log(" OK\r\n");

    Log("Loading ESP Config...");
    if (ESP_ConfigLoad(&ESP_Config, WiFiConfigFilename) == 0) {
        if (CreateFactoryDefaultWifiConfig() != FR_OK) {
            Log("Unable to create Default Wifi Config...\r\n");
            while (1);
        }
    }
    Log(" OK\r\n");


    sprintf(ESP_Config.UUID, "%s", UUID);

    Log("Checing HA1...");
    if (strlen(ESP_Config.HA1) != 32) {
        Log("HA1 Value is corrupt! Creating default username and password...username='user' Password='%s'\r\n", (const char *) &secBuff[4]);
        UpdateCredentials("user", ConfigPassword);
    }
    Log("OK\r\n");

    if (configMode) {
        Log("CONFIG MODE DETECTED\r\n");
        Global_Config_Mode = 1;
        sprintf(ESP_Config.Name, "TEMPCONCONFIG");
        ESP_Config.EncryptionMode = AUTH_WPA2_PSK;
        ESP_Config.Mode = SOFTAP_MODE;
        sprintf(ESP_Config.Password, "%s", &secBuff[4]);
        sprintf(ESP_Config.SSID, "TEMPCONCONFIG");
        ESP_Config.Channel = 1;
        Log("***SSID='%s' Password='%s'\r\n", ESP_Config.SSID, ESP_Config.Password);
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
    uPnP_Init(ESP_Config.Name, ESP_Config.UUID, IP_Address.l);
    Log("   uPnP Service Online\r\n");


    res = ff_OpenByFileName(&defaultEquip, "equip.default", 0);
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
