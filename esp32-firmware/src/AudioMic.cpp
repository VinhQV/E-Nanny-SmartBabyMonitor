#include "AudioMic.h"

int16_t audio_buffer[RAW_SAMPLE_COUNT];
volatile bool is_buffer_ready = false;

void i2s_record_task(void *pvParameters) {
  const int CHUNK_SIZE = 64;
  int32_t i2s_raw_buffer[CHUNK_SIZE];
  size_t bytesRead;
  int buffer_index = 0;

  while (true) {
    i2s_read(I2S_PORT, &i2s_raw_buffer, sizeof(i2s_raw_buffer), &bytesRead, portMAX_DELAY);
    int samples_read = bytesRead / sizeof(int32_t);

    if (!is_buffer_ready) {
      for (int i = 0; i < samples_read; i++) {
        audio_buffer[buffer_index] = (int16_t)(i2s_raw_buffer[i] >> 14);
        buffer_index++;

        if (buffer_index >= RAW_SAMPLE_COUNT) {
          buffer_index = 0;
          is_buffer_ready = true;
          break;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void audio_mic_init() {
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0};

  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_SD};

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);

  xTaskCreatePinnedToCore(i2s_record_task, "I2S_Record", 4096, NULL, 1, NULL, 0);
}