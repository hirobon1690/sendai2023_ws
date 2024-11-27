#pragma once
#include <stdio.h>
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

class Adc {
   public:
    Adc(int pin);
    void init();
    uint16_t read();
    float readVoltage();

   private:
    int pin;
    static bool isInitialized;
};