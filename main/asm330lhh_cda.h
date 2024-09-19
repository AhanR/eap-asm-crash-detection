/*
//
// ASM330LHH - Crash Detection Algorithms
// Author - Ahan
*/

#pragma once

#include "sdkconfig.h"

uint16_t index_ringbuff = 0;
uint16_t prev_val[3] = {0,0,0};
uint16_t length = CONFIG_RINGBUFFER_LEN;
uint16_t threshold = CONFIG_THRESHOLD_XL;
static int16_t buff_x[CONFIG_RINGBUFFER_LEN], buff_y[CONFIG_RINGBUFFER_LEN], buff_z[CONFIG_RINGBUFFER_LEN];

int sum_abs_diff(int16_t *new_val);