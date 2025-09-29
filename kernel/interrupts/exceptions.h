#pragma once

#define INPUT_BUFFER_SIZE 256

extern char input_buffer[INPUT_BUFFER_SIZE];
extern unsigned int input_pos;
extern bool input_ready;
extern volatile uint64_t ticks;

void div_by_0_handler(int_frame_32_t *frame);
void keyboard_handler(int_frame_32_t *frame);
void PIT_handler(int_frame_32_t *frame);
void page_fault_handler(int_frame_32_t* frame, uint32_t error_code);
void gpf_handler(int_frame_32_t* frame, uint32_t error_code);
