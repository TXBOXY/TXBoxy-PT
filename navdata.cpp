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

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "txb_config.h"
#include "navdata.h"
#include "telnet_debug.h"

/* Public variables */
uint32_t drone_state = 0xFFFFFFFF;      /* Initialize at invalid state    */

/* Private variables */
WiFiUDP navdata_udp;                    /* UDP connection for navdata     */

/* Private functions */
//--

void navdata_init()
{
  navdata_udp.begin(NAVDATA_UDP_PORT);
}

/*
 * Send first packet to initiate data stream.
 * Sending hex values "01 00 04 00" to 192.168.1.1:5554 (UDP)
 * First two bytes: 16-bit little-endian 1, represents Unicast (see ardrone_navdata_client.c)
 * Secnd two bytes: 16-bit little-endian 4, length of the message
 */
void navdata_send_dummy_packet()
{
  char message[4];
  message[0] = 0x01;
  message[1] = 0x00;
  message[2] = 0x04;
  message[3] = 0x00;
  navdata_udp.beginPacket(drone_ip, NAVDATA_UDP_PORT);
  navdata_udp.write(message, sizeof(message));
  navdata_udp.endPacket();
}

/*
 * Example response 88 77 66 55 B5 C8 0F
 * See Soft/Common/config.h for drone state variables
 */
bool navdata_get_status()
{
  int packetSize = navdata_udp.parsePacket();
  int len = 0;
  if (packetSize >= 8) {
    //Serial.printf("\nReceived %d\n", packetSize);
    char sbuf[packetSize];
    len = navdata_udp.read(sbuf, packetSize);
    /* Drone state is received in little endian format */
    if (sbuf[0] == 0x88 && sbuf[1] == 0x77 && sbuf[2] == 0x66 && sbuf[3] == 0x55) {
      drone_state = (sbuf[4] << 0) | (sbuf[5] << 8) | (sbuf[6] << 16) | (sbuf[7] << 24);
      //Serial.print(drone_state, HEX);
      return true;
    }
  }
  return false;
}

bool navdata_is_valid()
{
  if (drone_state & ARDRONE_NAVDATA_BOOTSTRAP) return false;
  return true;
}

bool navdata_state(uint32_t bit)
{
  return (drone_state & bit);
}

