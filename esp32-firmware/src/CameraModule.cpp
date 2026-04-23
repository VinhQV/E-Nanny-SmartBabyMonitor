#include "CameraModule.h"
#include "CloudClient.h"
#include <HTTPClient.h> 
#include "mbedtls/base64.h" 

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM; config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Loi khoi tao Camera!");
    return;
  }
}

void captureAndSendImage() {
  Serial.println("Dang chup anh...");
  camera_fb_t * fb = esp_camera_fb_get();
  if (fb) { esp_camera_fb_return(fb); delay(200); }      
  
  fb = esp_camera_fb_get();
  if (!fb) { Serial.println("Loi chup anh!"); return; }

  size_t base64_len = ((fb->len + 2) / 3) * 4 + 1;
  unsigned char * base64_buf = (unsigned char *) malloc(base64_len);
  if (base64_buf == NULL) { esp_camera_fb_return(fb); return; }

  size_t olen = 0;
  mbedtls_base64_encode(base64_buf, base64_len, &olen, fb->buf, fb->len);
  base64_buf[olen] = '\0'; 
  esp_camera_fb_return(fb); 

  String telemetryPayload = "{\"capturedImage\":\"data:image/jpeg;base64,";
  telemetryPayload += (char*)base64_buf; telemetryPayload += "\"}";
  free(base64_buf); 

  HTTPClient http;
  String url = String("http://") + tb_broker + ":" + String(tb_http_port) + "/api/v1/" + tb_token + "/telemetry";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(telemetryPayload);
  
  if (httpResponseCode == 200) Serial.println("Da gui anh THANH CONG len ThingsBoard!");
  http.end();
}