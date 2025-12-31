#ifndef API_CONFIG_H
#define API_CONFIG_H

#include <Arduino.h>

String openai_API_Key = "sk-proj-"
                        "4fXq6j2fUsND8IIJglQHZJ8ekwcVZlaOzAzptVYDH30iC71dNFDS8d"
                        "LZOaKfnR8p0CgrEFdQIET3BlbkFJHtjW2FCt1NkIOiy3lePIUAr7Vu"
                        "hGzpSs0z9tmf6_H1Yrv0BSvm5dxyASCX8GzuXonKll2FPzcA";

const String openai_endpoint = "https://api.openai.com/v1/chat/completions";
const String openai_model = "gpt-4o-mini";

String elevenlabs_API_Key = "";
String voice_ID = "21m00Tcm4TlvDq8ikWAM";

const String elevenlabs_endpoint =
    "https://api.elevenlabs.io/v1/text-to-speech/";

#endif
