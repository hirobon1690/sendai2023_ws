#pragma once
#include "pico/stdlib.h"
#include "pwm.h"

class Motor {
   public:
    Motor(int port1, int port2);
    void init();
    void setDuty(float val);
    void calc();

   private:
    repeating_timer ti;
    Pwm pwm[2];
};
