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
