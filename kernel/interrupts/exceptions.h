#pragma once

extern char input_buffer[256];
extern unsigned int input_pos;
extern bool input_ready;

void div_by_0_handler(int_frame_32_t *frame);
void keyboard_handler(int_frame_32_t *frame);
