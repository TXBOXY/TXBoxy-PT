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

#ifndef AT_COMMANDS_H
#define AT_COMMANDS_H

#define AT_REF_BASE       (1U << 18) | (1U << 20) | (1U << 22) | (1U << 24) | (1U << 28)
#define AT_REF_MASK       (1U << 8)  | (1U << 9)
#define AT_REF_TAKEOFF    (1U << 9)
#define AT_REF_EMERGENCY  (1U << 8)

extern bool at_config_confirmed;

void at_connect_udp(void);
void at_ref(uint32_t commands);
void at_config(const char* arg_1, const char* arg_2);
void at_comwdg();
void at_pcmd(uint16_t* cppm);
void at_ctrl(uint32_t a, uint32_t b);
void at_ack();
void at_kill();

#endif // AT_COMMANDS_H
