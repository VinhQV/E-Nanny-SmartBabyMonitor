#include <Arduino.h>
#include "CloudClient.h"
#include "CameraModule.h"
#include "Stm32Controller.h"
#include "AudioMic.h"
#include "AiEngine.h"

int cry_detected_count = 0;
int silence_count = 0; 
const int REQUIRE_CRY_COUNT = 2;

void setup() {
  Serial.begin(115200);
  Serial.println("Khoi dong he thong E-Nanny...");

  initStm32UART();
  setupCamera();
  audio_mic_init();
  setupCloud();

  Serial.println("E-Nanny System Ready!");
}

void loop() {
  cloudLoop(); // Xu ly giu ket noi mang va MQTT
  
//  checkSerialCommands();
  processIncomingUART(); // Doc du lieu UART tu STM32

  // Chay module AI nhan dien tieng khoc
  if (is_buffer_ready) {
    float current_score = run_cry_detection();
    
    if (current_score >= 0.5) {
      cry_detected_count++;
      silence_count = 0; // Reset bo dem im lang
      if (cry_detected_count >= REQUIRE_CRY_COUNT) {
         Serial.printf("=> PHAT HIEN TIENG KHOC! Score: %.2f\n", current_score);
         sendCommandToSTM32(0x04, 1); // Bat noi
         sendCommandToSTM32(0x01, 1); // Bat nhac
         updateBabyStatus("Em be dang khoc"); // API hien thi tren App
         cry_detected_count = 0; 
      }
    } else {
      cry_detected_count = 0; 
      silence_count++;
      if (silence_count >= 10) { // Neu im lang mot luc lau 
         updateBabyStatus("Em be ngu ngoan");
      }
    }
    is_buffer_ready = false;
  }
  
  delay(10); 
}