/*
 * TXBoxy-AR firmware for ARDrone2
 * 
 * Copyright (c) Bart Slinger
 * 
 */
 
#ifndef TXB_CONFIG_H
#define TXB_CONFIG_H

#define LED_PIN BUILTIN_LED
#define LED_ON  LOW
#define LED_OFF HIGH

#define TAKEOFF_SWITCH  AUX1
#define KILL_SWITCH     AUX2

#define CPPM_GPIO 14

#define AT_UDP_PORT       5556
#define NAVDATA_UDP_PORT  5554

extern const char *drone_ip;      /* Drone IP defined in main .ino */

#endif // TXB_CONFIG_H


