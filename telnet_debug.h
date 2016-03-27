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
