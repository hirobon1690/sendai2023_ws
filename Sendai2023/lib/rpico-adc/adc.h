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
    void setCali(float val);

   private:
    int pin;
    static bool isInitialized;
    float cali=1.0;
};