#ifndef SPEAKER_H
#define SPEAKER_H
#include "../sda_platform.h"

void sda_base_beep_start();

void svp_beep();

void svp_beep_set_t(uint16_t time);
void svp_beep_set_pf(uint16_t val);
void svp_beep_set_def();
void sda_beep_setup(uint16_t freq);

void sda_base_spkr_irq_handler();

#endif
