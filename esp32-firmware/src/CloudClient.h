#ifndef CLOUD_CLIENT_H
#define CLOUD_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Expose cac bien de module khac dung chung
extern PubSubClient client;
extern const char* tb_broker;
extern const int tb_http_port;
extern const char* tb_token;

void setupCloud();
void cloudLoop();
void reconnectMqtt();
void updateBabyStatus(String status);
void sendTelemetry(uint8_t temp, uint8_t hum);

#endif