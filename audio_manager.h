#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H
#include "config.h"
#include <Arduino.h>
#include <driver/i2s.h>

static const char B64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

String encodeBase64(uint8_t *data, size_t length) {
  String encodedString = "";
  encodedString.reserve(((length + 2) / 3) * 4);
  int i = 0, j = 0;
  uint8_t char_array_3[3], char_array_4[4];
  while (length--) {
    char_array_3[i++] = *(data++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;
      for (i = 0; i < 4; i++)
        encodedString += B64_CHARS[char_array_4[i]];
      i = 0;
    }
  }
  if (i) {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;
    for (j = 0; j < i + 1; j++)
      encodedString += B64_CHARS[char_array_4[j]];
    while (i++ < 3)
      encodedString += '=';
  }
  return encodedString;
}

void setupAudio() {
  // Setup Mic (I2S0)
  i2s_config_t mic_cfg = {.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
                          .sample_rate = SAMPLE_RATE_ASR,
                          .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
                          .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
                          .communication_format = I2S_COMM_FORMAT_I2S,
                          .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
                          .dma_buf_count = 8,
                          .dma_buf_len = 64};
  i2s_pin_config_t mic_pins = {.bck_io_num = I2S_MIC_SERIAL_CLOCK,
                               .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
                               .data_out_num = I2S_PIN_NO_CHANGE,
                               .data_in_num = I2S_MIC_SERIAL_DATA};
  i2s_driver_install(I2S_PORT_MIC, &mic_cfg, 0, NULL);
  i2s_set_pin(I2S_PORT_MIC, &mic_pins);

  // Setup Speaker (I2S1)
  i2s_config_t spk_cfg = {.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
                          .sample_rate = SAMPLE_RATE_TTS,
                          .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
                          .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
                          .communication_format = I2S_COMM_FORMAT_I2S,
                          .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
                          .dma_buf_count = 8,
                          .dma_buf_len = 64};
  i2s_pin_config_t spk_pins = {.bck_io_num = I2S_SPEAKER_BCLK,
                               .ws_io_num = I2S_SPEAKER_LRC,
                               .data_out_num = I2S_SPEAKER_DIN,
                               .data_in_num = I2S_PIN_NO_CHANGE};
  i2s_driver_install(I2S_PORT_SPK, &spk_cfg, 0, NULL);
  i2s_set_pin(I2S_PORT_SPK, &spk_pins);
}

void playRawAudio(int16_t *samples, size_t count) {
  size_t bytes_written = 0;
  i2s_write(I2S_PORT_SPK, samples, count * 2, &bytes_written, portMAX_DELAY);
}

String recordAudioBase64(int durationMs) {
  int numSamples = (SAMPLE_RATE_ASR * durationMs) / 1000;
  size_t bytesToRead = numSamples * 2 * 2; // 16-bit, 2 channels (Stereo)
  uint8_t *buffer = (uint8_t *)malloc(bytesToRead);
  if (!buffer)
    return "";
  size_t bytesRead = 0;
  // Read from 2x INMP441 (Stereo)
  i2s_read(I2S_PORT_MIC, buffer, bytesToRead, &bytesRead, portMAX_DELAY);
  String base64Audio = encodeBase64(buffer, bytesRead);
  free(buffer);
  return base64Audio;
}

int detectSoundDirection() {
  const int numSamples = 512;
  int16_t buffer[numSamples * 2];
  size_t bytesRead = 0;
  i2s_read(I2S_PORT_MIC, buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);
  long leftSum = 0, rightSum = 0;
  int sampleCount = bytesRead / 4; // 2 channels, 2 bytes per sample
  if (sampleCount < 10)
    return 0;
  for (int i = 0; i < sampleCount * 2; i += 2) {
    leftSum += abs(buffer[i]);      // Channel 1
    rightSum += abs(buffer[i + 1]); // Channel 2
  }
  float leftAvg = (float)leftSum / sampleCount;
  float rightAvg = (float)rightSum / sampleCount;
  if (leftAvg < 150 && rightAvg < 150)
    return 0;
  if (rightAvg > leftAvg * 1.5)
    return 1; // Right
  if (leftAvg > rightAvg * 1.5)
    return -1; // Left
  return 2;    // Center
}

#endif
