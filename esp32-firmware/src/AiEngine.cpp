#include "AiEngine.h"
#include "AudioMic.h" 
#include <Cry_baby_inferencing.h>

static int get_audio_data(size_t offset, size_t length, float *out_ptr) {
  for (size_t i = 0; i < length; i++) {
    out_ptr[i] = (float)audio_buffer[offset + i];
  }
  return 0;
}

float run_cry_detection() {
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &get_audio_data;

    ei_impulse_result_t result = {0};
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

    if (res == EI_IMPULSE_OK) {
      for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        // Kiểm tra nhãn "Baby" hoặc "Cry" tùy theo model của bạn
        if (strcmp(result.classification[i].label, "Baby") == 0 || 
            strcmp(result.classification[i].label, "Cry") == 0) {
          return result.classification[i].value;
        }
      }
    }
    return 0.0;
}