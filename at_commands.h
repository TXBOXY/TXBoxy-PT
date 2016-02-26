/*
 * TXBoxy-AR firmware for ARDrone2
 * 
 * Copyright (c) Bart Slinger
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
