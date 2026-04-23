#ifndef STM32_CONTROLLER_H
#define STM32_CONTROLLER_H

#include <Arduino.h>

#define RX_PIN_TO_STM32 44 
#define TX_PIN_TO_STM32 43 

void initStm32UART();
void sendCommandToSTM32(uint8_t command, uint8_t data);
void checkSerialCommands();
void processIncomingUART();

#endif