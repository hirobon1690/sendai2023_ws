#pragma once
#include <pico/time.h>

bool timer_cb(repeating_timer_t*);
bool timer_cb_pos(repeating_timer_t*);
void initTimer();