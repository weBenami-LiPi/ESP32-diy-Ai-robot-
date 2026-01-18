#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// Simple logger to replace missing file
inline void logLoc(String msg) { Serial.println("[LOC] " + msg); }

inline void logChat(String msg) { Serial.println("[CHAT] " + msg); }

inline String readLogFile(const char *path) {
  return "Logs not supported currently.";
}
inline void clearLogFile(const char *path) {}

#endif
