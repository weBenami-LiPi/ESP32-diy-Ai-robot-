#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <SPIFFS.h>

inline void logToFile(String filename, String msg) {
  if (SPIFFS.exists(filename)) {
    File check = SPIFFS.open(filename, "r");
    if (check.size() > 10000) {
      check.close();
      SPIFFS.remove(filename);
    } else {
      check.close();
    }
  }
  File f = SPIFFS.open(filename, "a");
  if (f) {
    f.println("[" + String(millis()) + "] " + msg);
    f.close();
  }
  Serial.println("LOG (" + filename + "): " + msg);
}

inline void logChat(String msg) { logToFile("/chat.txt", msg); }
inline void logLoc(String msg) { logToFile("/loc.txt", msg); }

inline String readLogFile(String filename) {
  if (!SPIFFS.exists(filename))
    return "No logs found in " + filename;
  File f = SPIFFS.open(filename, "r");
  if (!f)
    return "Error reading logs.";
  String logs = "";
  while (f.available()) {
    logs += (char)f.read();
    if (logs.length() > 5000)
      break;
  }
  f.close();
  return logs;
}

inline void clearLogFile(String filename) { SPIFFS.remove(filename); }

#endif
