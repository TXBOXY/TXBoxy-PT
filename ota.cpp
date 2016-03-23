/*
 * TXBoxy-AR firmware for ARDrone2
 * 
 * Copyright (c) Bart Slinger
 * 
 * Upload command with OTA:
 * python /home/bart/.arduino15/packages/esp8266/hardware/esp8266/2.1.0/tools/espota.py -i txboxy.local -r -f /home/bart/git/TXBoxy-AR/TXBoxy-AR.cpp.generic.bin
 * 
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "txb_config.h"
#include "ota.h"

/* Private variables */
const char* ota_host = "txboxy";                /* Name for OTA update device */

void ota_init(void)
{
  ArduinoOTA.setHostname("TXBoxy");
  /* No authentication by default */
  // ArduinoOTA.setPassword((const char *)"123");
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

/* Needs to be called periodically to handle requests */
void ota_handle_client(void)
{
  ArduinoOTA.handle();
}

