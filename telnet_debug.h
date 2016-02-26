/*
 * TXBoxy-AR firmware for ARDrone2
 * 
 * Copyright (c) Bart Slinger
 * 
 */

#ifndef DEBUG_H
#define DEBUG_H

#define SMART(_V_)  if (_V_ != (int)NULL) _V_
//#define SMART(_V_) _V_

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "txb_config.h"

extern WiFiClient debug_client;

void debug_init(void);
void debug_check_clients(void);

#endif // DEBUG_H
