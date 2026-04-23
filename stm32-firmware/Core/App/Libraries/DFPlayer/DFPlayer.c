/*
 * DFPlayer.c
 *
 *  Created on: Apr 16, 2026
 *      Author: vinhv
 */

#include "DFPlayer.h"

extern UART_HandleTypeDef huart2;

void DFPlayer_SendCommand(uint8_t command, uint8_t param1, uint8_t param2) {
    uint8_t buffer[10];
    uint16_t checksum;

    buffer[0] = 0x7E; // Start byte
    buffer[1] = 0xFF; // Version
    buffer[2] = 0x06; // Length
    buffer[3] = command; // Command
    buffer[4] = 0x00; // Feedback
    buffer[5] = param1; // Parameter 1
    buffer[6] = param2; // Parameter 2

    // Tinh Checksum
    checksum = 0xFFFF - (buffer[1] + buffer[2] + buffer[3] + buffer[4] + buffer[5] + buffer[6]) + 1;

    buffer[7] = (uint8_t)(checksum >> 8); // Checksum high byte
    buffer[8] = (uint8_t)checksum;        // Checksum low byte
    buffer[9] = 0xEF; // End byte

    // Gui data qua UART2
    HAL_UART_Transmit(&huart2, buffer, 10, HAL_MAX_DELAY);
}



void DFPlayer_Init(uint8_t volume) {
    if(volume > 30) volume = 30; 		//Max volume

    DFPlayer_SendCommand(0x0C, 0x00, 0x00);
    HAL_Delay(2000);

    DFPlayer_SendCommand(0x06, 0x00, volume);
    HAL_Delay(500);
}


void DFPlayer_PlayTrack(uint16_t track_number) {
    uint8_t high_byte = (uint8_t)(track_number >> 8);
    uint8_t low_byte = (uint8_t)(track_number & 0xFF);
    DFPlayer_SendCommand(0x03, high_byte, low_byte);
}


void DFPlayer_Stop(void) {
    DFPlayer_SendCommand(0x16, 0x00, 0x00);
}
