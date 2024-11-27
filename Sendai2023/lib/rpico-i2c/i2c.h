#pragma once
#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

class I2c {
   public:
    I2c(i2c_inst_t* ch = i2c_default, int baud = 100 * 1000);
    void init();
    int write(uint8_t addr, uint8_t data, bool nostop = true);
    int write(uint8_t addr, uint8_t* data, size_t size, bool nostop = true);
    int read(uint8_t addr, uint8_t* data, size_t size);

   private:
    i2c_inst_t* ch;
    int baud;
};

extern I2c i2c;