#include "CloudClient.h"
#include <ArduinoJson.h>
#include "Stm32Controller.h"
#include "CameraModule.h"

const char* ssid = "Jizy";
const char* password = "222222222";
const char* tb_broker = "nguyendinhdat.io.vn";
const int tb_http_port = 80;          
const char* tb_token = "JXVcFhixjFi78g0B1mLU";

WiFiClient espClient;
PubSubClient client(espClient);

// Ham xu ly khi nhan duoc tin nhan tu ThingsBoard
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) messageTemp += (char)payload[i];
  
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, messageTemp);
  if (error) return;

  const char* method = doc["method"];
  bool command_recognized = false;
  
  if (String(method) == "take_photo") {
    Serial.println("=> CLOUD: Lenh chup anh");
    command_recognized = true;
  }
  else if (String(method) == "set_crib") {
    bool state = doc["params"]; 
    if (state) {
      Serial.println("=> CLOUD: CONG TAC - Bat Noi");
      sendCommandToSTM32(0x04, 1);
    } else {
      Serial.println("=> CLOUD: CONG TAC - Tat Noi");
      sendCommandToSTM32(0x04, 0);
    }
    command_recognized = true;
  }
  else if (String(method) == "set_speaker") {
    bool state = doc["params"]; 
    if (state) {
      Serial.println("=> CLOUD: CONG TAC - Bat Loa");
      sendCommandToSTM32(0x01, 1);
    } else {
      Serial.println("=> CLOUD: CONG TAC - Tat Loa");
      sendCommandToSTM32(0x01, 0);
    }
    command_recognized = true;
  }

  // Tra loi Thingsboard ngay lap tuc de tranh timeout
  if (command_recognized) {
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), "{\"status\":\"ok\"}");
    
    // Thuc thi lenh chup anh sau khi tra loi
    if (String(method) == "take_photo") {
       captureAndSendImage();
    }
  }
}

void reconnectMqtt() {
  while (!client.connected()) {
    Serial.print("Dang ket noi MQTT...");
    if (client.connect("XIAO_S3_CAM", tb_token, NULL)) {
      Serial.println(" Thanh cong!");
      client.subscribe("v1/devices/me/rpc/request/+"); 
      updateBabyStatus("He thong khoi dong"); 
    } else {
      delay(3000);
    }
  }
}

void setupCloud() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\nDa ket noi WiFi!");

  client.setBufferSize(1024); 
  client.setServer(tb_broker, 1883);
  client.setCallback(onMqttMessage);
}

void cloudLoop() {
  if (!client.connected()) reconnectMqtt();
  client.loop();
}

void updateBabyStatus(String status) {
  static String last_status = "";
  if (status != last_status && client.connected()) {
    String payload = "{\"baby_status\":\"" + status + "\"}";
    client.publish("v1/devices/me/telemetry", payload.c_str());
    last_status = status;
    Serial.println("=> CLOUD: Cap nhat trang thai: " + status);
  }
}

void sendTelemetry(uint8_t temp, uint8_t hum) {
  if (client.connected()) {
    String telemetryInfo = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + "}";
    client.publish("v1/devices/me/telemetry", telemetryInfo.c_str());
  }
}