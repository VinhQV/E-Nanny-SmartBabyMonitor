#ifndef AUDIOMIC_H
#define AUDIOMIC_H

#include <Arduino.h>
#include <driver/i2s.h>

// Định nghĩa chân phần cứng
#define I2S_SCK 7
#define I2S_WS 8
#define I2S_SD 9
#define I2S_PORT I2S_NUM_0

#define RAW_SAMPLE_COUNT 16000 // Thường là 16000 cho 1 giây âm thanh

// Biến toàn cục chia sẻ
extern int16_t audio_buffer[RAW_SAMPLE_COUNT];
extern volatile bool is_buffer_ready;

void audio_mic_init();

#endif