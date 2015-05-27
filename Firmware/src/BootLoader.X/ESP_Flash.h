/* 
 * File:   ESP_Flash.h
 * Author: THORAXIUM
 *
 * Created on May 24, 2015, 11:00 PM
 */

#ifndef ESP_FLASH_H
#define	ESP_FLASH_H

#ifdef	__cplusplus
extern "C" {
#endif

    int espflash_FlashFinish(char reboot);
    int espflash_FlashFile(unsigned long imageAddress, unsigned long imageLength, unsigned long esp_flashOffset);
    int espflash_connect();
    int espflash_disconnect();
    void Delay(float time);


#ifdef	__cplusplus
}
#endif

#endif	/* ESP_FLASH_H */

