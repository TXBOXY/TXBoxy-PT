/*
 * TXBoxy-AR firmware for ARDrone2
 * 
 * Copyright (c) Bart Slinger
 * 
 */

#ifndef CPPM_H
#define CPPM_H

#define CPPM_CHANNELS 12 // number of channels in cCPPM stream, 12 ideally
enum chan_order{
  THROTTLE,
  AILERON,
  ELEVATOR,
  RUDDER,
  AUX1,
  AUX2,
  AUX3,
  AUX4,
  AUX5,
  AUX6,
  AUX7,
  AUX8,
};

#define CPPM_MIN 1000
#define CPPM_SAFE_THROTTLE 1050 
#define CPPM_MID 1500
#define CPPM_MAX 2000
#define CPPM_MIN_COMMAND 1300
#define CPPM_MAX_COMMAND 1700

enum startup_codes {
  CENTER = 0,
  TOP,
  TOP_RIGHT,
  RIGHT,
  BOTTOM_RIGHT,
  BOTTOM,
  BOTTOM_LEFT,
  LEFT,
  TOP_LEFT,
};

/* Public variables */
struct cppm_t {
  volatile bool ok;
  enum startup_codes captured_position = CENTER;
  volatile uint16_t servo_data[12];
  uint16_t values[12] = {CPPM_MIN, CPPM_MIN, CPPM_MIN, CPPM_MIN, CPPM_MIN, CPPM_MIN,
                         CPPM_MIN, CPPM_MIN, CPPM_MIN, CPPM_MIN, CPPM_MIN, CPPM_MIN};
};

extern struct cppm_t cppm;

/* Public function declarations */
void cppm_init(void);
void cppm_pause(void);
void cppm_continue(void);
void cppm_update(void);
void cppm_capture_special_position();

#endif // CPPM_H
