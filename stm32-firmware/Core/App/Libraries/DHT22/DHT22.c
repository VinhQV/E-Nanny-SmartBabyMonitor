/*
 * DHT22.c
 *
 *  Created on: Apr 9, 2026
 *      Author: vinhv
 */

#include "DHT22.h"

//Lay timer tu system de tao delay
extern TIM_HandleTypeDef htim11;


static void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim11, 0);
    while ((__HAL_TIM_GET_COUNTER(&htim11)) < us);
}

// ==== CAU HINH 1 WIRE ====

//Ham cau hinh che do OUTPUT cho MCU
static void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

//Config mode INPUT
static void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;			//Keo tro len
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

static uint8_t DHT22_Start(void){
	uint8_t response = 0;
	uint8_t timeout = 0;

	Set_Pin_Output(DHT_PORT, DHT_PIN);
	HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
	delay_us(1200);

	HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
	delay_us(20);

	Set_Pin_Input(DHT_PORT, DHT_PIN);
	delay_us(40);

	if(!(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))){
		delay_us(80);
		if((HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))){
			response = 1;

		}else{
			response = -1;
		}
	}

	timeout = 0;
	while((HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)))
	{
		delay_us(1);
		if(++timeout > 100) return 0;
	}
	return response;
}


static uint8_t DHT22_Read_Byte(void)
{
	uint8_t i = 0, j;
	uint16_t timeout;

	for(j = 0; j < 8; j++)
	{
		timeout = 0;
		while(!(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))){
			delay_us(1);
			if(++timeout > 100) return 0;
		}
		delay_us(40);
		if(!(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))){
			i &= ~(1 << (7 - j)); 			//Clear bit
		}else{
			i |=(1 << (7 - j));				//Set bit
			timeout = 0;
			while((HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))){
				delay_us(1);
				if(++timeout > 100) return 0;
			}
		}
	}
	return i;
}


//==== READ DATA DHT22 -->  TEMPERATURE & HUMIDITY

uint8_t DHT22_Read(volatile DHT22_Data_t *DHT_Data)
{
	uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
	uint16_t checkSum;

	if(DHT22_Start() == 1)
	{
		Rh_byte1 = DHT22_Read_Byte();
		Rh_byte2 = DHT22_Read_Byte();
		Temp_byte1 = DHT22_Read_Byte();
		Temp_byte2 = DHT22_Read_Byte();
		checkSum =  DHT22_Read_Byte();
	}
	//Kiem tra bang bien checkSum xem co du 40bit theo DTS yeu cau khong
	if( checkSum == ((Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2) & 0xFF))
	{
		uint16_t raw_temp = (Temp_byte1 << 8) | Temp_byte2;
		uint16_t raw_humid = (Rh_byte1 << 8) | Rh_byte2;

		DHT_Data->Temperature = (float)raw_temp / 10.0f;
		DHT_Data->Humidity =  (float)raw_humid / 10.0f;
		return 1; //Thanh cong
	}

	return 0;			//fail
}



















