/*
 * TXBoxy-AR firmware for ARDrone2
 * 
 * Copyright (c) Bart Slinger
 * 
 */

#include "telnet_debug.h"
#include "cppm.h"

WiFiClient debug_client;
WiFiServer telnet(23);

void debug_init()
{
  /* Begin mDNS */
  if (!MDNS.begin("txboxy")) {
    Serial.println("Error configuring mDNS");
    return;  
  }

  /* Start telnet server */
  telnet.begin();
  telnet.setNoDelay(true);
}

void debug_check_clients()
{
  if (telnet.hasClient()){
    //find free/disconnected spot
    Serial.println("1");
    if (!debug_client || !debug_client.connected()){
      Serial.println("2");
      if(debug_client) debug_client.stop();
      Serial.println("3");
      debug_client = telnet.available();
      Serial.println("4");
      debug_client.println("Telnet client initialized.");
      return;
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = telnet.available();
    serverClient.stop();
  }
}

