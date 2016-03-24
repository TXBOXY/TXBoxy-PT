/*
 * TXBoxy-AR firmware for ARDrone2
 * 
 * Copyright (c) Bart Slinger
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

