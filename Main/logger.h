#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <SPIFFS.h>

// Persistent Logging System
inline void appendLog(const char *path, String msg) {
  File f = SPIFFS.open(path, "a");
  if (f) {
    if (f.size() > 5000) { // Keep last ~5KB
      f.close();
      SPIFFS.remove(path);
      f = SPIFFS.open(path, "w");
    }
    f.println(msg);
    f.close();
  }
}

inline void logLoc(String msg) {
  Serial.println("[LOC] " + msg);
  appendLog("/loc.txt", "[LOC] " + msg);
}

inline void logChat(String msg) {
  Serial.println("[CHAT] " + msg);
  appendLog("/chat.txt", msg);
}

inline String readLogFile(const char *path) {
  if (!SPIFFS.exists(path))
    return "No data.";
  File f = SPIFFS.open(path, "r");
  if (!f)
    return "Error reading logs.";
  String out = "";
  while (f.available())
    out += (char)f.read();
  f.close();
  return (out.length() > 0) ? out : "Log empty.";
}

inline void clearLogFile(const char *path) {
  if (SPIFFS.exists(path))
    SPIFFS.remove(path);
}

#endif
