/*
 * TXBoxy-PT firmware for ARDrone2
 * Copyright (C) 2016 Bart Slinger
 * 
 * This file is part of TXBoxy-PT.
 *
 * TXBoxy-PT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TXBoxy-PT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with TXBoxy-PT.  If not, see <http://www.gnu.org/licenses/>.
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

