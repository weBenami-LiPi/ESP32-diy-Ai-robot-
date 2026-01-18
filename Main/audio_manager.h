#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>

// Audio System Removed
inline void setupAudio() {}
inline void playRawAudio(int16_t *samples, size_t count) {}
inline String recordAudioBase64(int durationMs) { return ""; }
inline int detectSoundDirection() { return 0; }

#endif
