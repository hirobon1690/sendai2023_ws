#pragma once
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "stdio.h"

class Servo {
   public:
    Servo(int pin);
    void init();
    void write(float val);
    float read();
    void write_ms(float ms);
    void setOffset(float val);

   private:
    int pin;
    uint slice_num;
    uint channel;
    float offset = 0;
};