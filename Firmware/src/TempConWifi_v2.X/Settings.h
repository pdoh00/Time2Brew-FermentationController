/* 
 * File:   Settings.h
 * Author: THORAXIUM
 *
 * Created on February 9, 2015, 10:08 AM
 */

#ifndef SETTINGS_H
#define	SETTINGS_H

#include "ESP8266.h"
#include "TemperatureController.h"

int CreateFactoryDefaultConfig();
int SaveConfig();
int ESP_ConfigSave(ESP8266_CONFIG *cfg, const char *filename);
int Load_Cfg_Mode_Config(void);
int GlobalStartup(int configMode);
int CreateDefaultEquipmentProfile();

#endif	/* SETTINGS_H */

