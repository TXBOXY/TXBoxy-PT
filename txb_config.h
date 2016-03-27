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
 
#ifndef TXB_CONFIG_H
#define TXB_CONFIG_H

#define LED_PIN 5 //BUILTIN_LED
#define LED_ON  HIGH
#define LED_OFF LOW

#define TAKEOFF_SWITCH  AUX1
#define KILL_SWITCH     AUX2

#define CPPM_GPIO 14

#define AT_UDP_PORT       5556
#define NAVDATA_UDP_PORT  5554

extern const char *drone_ip;      /* Drone IP defined in main .ino */

#endif // TXB_CONFIG_H


