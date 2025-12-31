#ifndef API_CONFIG_H
#define API_CONFIG_H

#include <Arduino.h>

String openai_API_Key = "YOUR API KEY";

const String openai_endpoint = "https://api.openai.com/v1/chat/completions";
const String openai_model = "gpt-4o-mini";

String elevenlabs_API_Key = "";
String voice_ID = "21m00Tcm4TlvDq8ikWAM";

const String elevenlabs_endpoint =
    "https://api.elevenlabs.io/v1/text-to-speech/";

#endif

