/*
 * TXBoxy-AR firmware for ARDrone2
 * 
 * Copyright (c) Bart Slinger
 * 
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "txb_config.h"
#include "at_commands.h"
#include "cppm.h"

#define AT_UDP_MAX_LENGTH 512

/* Public variables */
bool at_config_confirmed = false;

/* Private variables */
WiFiUDP at_udp;                   /* UDP connection for AT Commands                   */
uint8_t packetBuffer[512];        /* Buffer to hold incoming and outgoing packets     */
uint32_t at_counter = 1;          /* Ever incrementing counter sent with each packet  */

/* Private functions */
void at_send_udp_packet(const char *str, ...);
int32_t cppm_to_AT(int32_t ppm);

void at_connect_udp()
{
  at_udp.begin(AT_UDP_PORT);
}

/*
 * Use flags AT_REF_TAKEOFF and AT_REF_EMERGENCY as argument.
 */
void at_ref(uint32_t commands)
{
  /* Can only set Takeoff/Land bit and Emergency bit */
  commands &= AT_REF_MASK;
  /* Append some bits which are always set to 1 */
  commands |= AT_REF_BASE;

  /* Send command */
  at_send_udp_packet("AT*REF=%d,%d\r", at_counter++, commands);
}

void at_config(const char* arg_1, const char* arg_2)
{
  at_send_udp_packet("AT*CONFIG=%d,\"%s\",\"%s\"\r", at_counter++, arg_1, arg_2);
}

void at_comwdg()
{
  at_send_udp_packet("AT*COMWDG=%d\r", at_counter++);
}

void at_pcmd(uint16_t* cppm)
{
  at_send_udp_packet("AT*PCMD=%d,%d,%d,%d,%d,%d\r", at_counter++, 1, cppm_to_AT(cppm[1]), cppm_to_AT( -(cppm[2]-CPPM_MID)+CPPM_MID ), cppm_to_AT(cppm[0]), cppm_to_AT(cppm[3]));
}

void at_ctrl(uint32_t a, uint32_t b)
{
  at_send_udp_packet("AT*CTRL=%d,%d,%d\r", at_counter++, a, b);
}

void at_ack()
{
  at_ctrl(5, 0);
}

void at_kill()
{
  unsigned long one = at_counter++;
  unsigned long two = at_counter++;
  unsigned long three = at_counter++;
  at_send_udp_packet("AT*REF=%d,290717696\rAT*REF=%d,290717952\rAT*REF=%d,290717696\r", one, two, three);
}

void at_send_udp_packet(const char *format, ...)
{
  /* Construct databuffer to send */
  char mybuffer[AT_UDP_MAX_LENGTH];
  va_list myargs;
  va_start (myargs, format);
  vsnprintf(mybuffer, AT_UDP_MAX_LENGTH, format, myargs);
  va_end(myargs);
  //Serial.println(mybuffer);
  at_udp.beginPacket(drone_ip, AT_UDP_PORT);
  at_udp.print(mybuffer);
  at_udp.endPacket();
}

int32_t cppm_to_AT(int32_t ppm)
{
  float value = (ppm - CPPM_MID) / 500.0;
  if (value < -1.0) value = -1.0;
  if (value > 1.0) value = 1.0;
  return *(int32_t*)(&value);
}


