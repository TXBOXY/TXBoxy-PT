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
#include "txb_config.h"
#include "cppm.h"

/* Struct containing cppm data */
struct cppm_t cppm;

/* Private function declarations */
void ISR_cppm();
void ISR_empty();

void cppm_init()
{
  pinMode(CPPM_GPIO, INPUT);
  cppm_continue();
}

/*
 * Set cppm.values using values obtained in the interrupts.
 */
void cppm_update()
{
  for(uint8_t ch=0; ch<CPPM_CHANNELS; ch++) {
    noInterrupts();
      cppm.values[ch] = cppm.servo_data[ch];
    interrupts();
  }    
}

void cppm_pause()
{
  detachInterrupt(CPPM_GPIO);
}

void cppm_continue()
{
  attachInterrupt(CPPM_GPIO, ISR_cppm, CHANGE);
}

void cppm_capture_special_position()
{
  if (cppm.values[AILERON] > CPPM_MAX_COMMAND && cppm.values[ELEVATOR] > CPPM_MAX_COMMAND) {
    cppm.captured_position = TOP_RIGHT;
  }
  else if (cppm.values[AILERON] < CPPM_MIN_COMMAND && cppm.values[ELEVATOR] > CPPM_MAX_COMMAND) {
    cppm.captured_position = TOP_LEFT;
  }
  else if (cppm.values[AILERON] < CPPM_MIN_COMMAND && cppm.values[ELEVATOR] < CPPM_MIN_COMMAND) {
    cppm.captured_position = BOTTOM_LEFT;
  }
}

void ISR_cppm()
{
  static unsigned int pulse;
  static unsigned long counterPPM;
  static unsigned long previous_micros = 0;
  static byte chan;
  unsigned long now = micros();
  counterPPM = now - previous_micros;
  previous_micros = now;
  cppm.ok=false;
  if(counterPPM < 510) {  //must be a pulse if less than 510us
    pulse = counterPPM;
  }
  else if(counterPPM > 1910) {  //sync pulses over 1910us
    chan = 0;
  }
  else {  //servo values between 510us and 2420us will end up here
    if(chan < CPPM_CHANNELS) {
      cppm.servo_data[chan]= constrain((counterPPM + pulse), CPPM_MIN, CPPM_MAX);
      if(chan==3)
        cppm.ok = true; // 4 first CPPM_CHANNELS Ok
    }
    chan++;
  }
}

void ISR_empty()
{
  
}

