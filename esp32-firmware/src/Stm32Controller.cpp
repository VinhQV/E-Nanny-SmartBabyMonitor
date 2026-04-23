#include "Stm32Controller.h"
#include "CloudClient.h"

enum RX_STATE { WAIT_HEADER_1, WAIT_HEADER_2, WAIT_LENGTH, WAIT_PAYLOAD_AND_CRC };
RX_STATE rx_state = WAIT_HEADER_1;

uint8_t rx_buffer[16];
uint8_t rx_index = 0;
uint8_t frame_len_rx = 0;

void initStm32UART() {
  Serial1.begin(115200, SERIAL_8N1, RX_PIN_TO_STM32, TX_PIN_TO_STM32);
}

void sendCommandToSTM32(uint8_t command, uint8_t data) {
  uint8_t frame_len = 2; 
  uint8_t crc = frame_len ^ command ^ data; 
  uint8_t tx_buffer[6] = {0xAA, 0x55, frame_len, command, data, crc};
  Serial1.write(tx_buffer, 6);
}

void checkSerialCommands() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input == "batnoi") {
      Serial.println("=> APP: Bat noi");
      sendCommandToSTM32(0x04, 1);
    } 
    else if (input == "tat noi") {
      Serial.println("=> APP: Tat noi");
      sendCommandToSTM32(0x04, 0); 
    }
    else if (input == "bat loa") {
      Serial.println("=> APP: Bat loa");
      sendCommandToSTM32(0x01, 1);
    }
    else if (input == "tat loa") {
      Serial.println("=> APP: Tat loa");
      sendCommandToSTM32(0x01, 0);
    }
  }
}

void processIncomingUART() {
  while (Serial1.available() > 0) {
    uint8_t rx_byte = Serial1.read();
    
    switch (rx_state) {
      case WAIT_HEADER_1: if (rx_byte == 0xAA) rx_state = WAIT_HEADER_2; break;
      case WAIT_HEADER_2: if (rx_byte == 0x55) rx_state = WAIT_LENGTH; else rx_state = WAIT_HEADER_1; break;
      case WAIT_LENGTH:
        frame_len_rx = rx_byte; rx_index = 0;
        if (frame_len_rx > 0 && frame_len_rx < 12) rx_state = WAIT_PAYLOAD_AND_CRC;
        else rx_state = WAIT_HEADER_1;
        break;
        
      case WAIT_PAYLOAD_AND_CRC:
        rx_buffer[rx_index++] = rx_byte;
        if (rx_index == (frame_len_rx + 1)) { 
          uint8_t calc_crc = frame_len_rx;
          for (int i = 0; i < frame_len_rx; i++) calc_crc ^= rx_buffer[i];
          
          if (calc_crc == rx_buffer[frame_len_rx]) {
            uint8_t command = rx_buffer[0];
            if (command == 0x03) { // Lenh Nhiet do do am
              uint8_t temp = rx_buffer[1];
              uint8_t hum = rx_buffer[2];
              Serial.printf("=> SENSOR: Nhiet do = %d C, Do am = %d %%\n", temp, hum);
              sendTelemetry(temp, hum); // Goi ham tu CloudClient
            }
          } else {
             Serial.println("=> LOI: Sai ma CRC tu STM32!");
          }
          rx_state = WAIT_HEADER_1;
        }
        break;
    }
  }
}