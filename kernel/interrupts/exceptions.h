#pragma once

extern char input_buffer[256];
extern unsigned int input_pos;
extern bool input_ready;
extern volatile uint64_t ticks;

void div_by_0_handler(int_frame_32_t *frame);
void keyboard_handler(int_frame_32_t *frame);
void PIT_handler(int_frame_32_t *frame);
