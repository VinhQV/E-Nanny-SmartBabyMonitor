/*
 * DHT22.h
 *
 *  Created on: Apr 9, 2026
 *      Author: vinhv
 */

#ifndef APP_LIBRARIES_DHT22_DHT22_H_
#define APP_LIBRARIES_DHT22_DHT22_H_



#include "stm32f4xx_hal.h"

// ================= CONFIG DHT22 =================
#define DHT_PORT 				GPIOA
#define DHT_PIN					GPIO_PIN_4


typedef struct{
	float Temperature;
	float Humidity;
}DHT22_Data_t;

uint8_t DHT22_Read(volatile DHT22_Data_t *DHT_Data);
#endif /* APP_LIBRARIES_DHT22_DHT22_H_ */
