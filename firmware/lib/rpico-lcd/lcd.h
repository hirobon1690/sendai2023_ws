#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "hardware/i2c.h"
#include "i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

class Lcd {
   public:
    Lcd();
    void print(const char*, int line = 1, int address = 0);
    void clr();
    void init(int num = 0, int contrast = 12);
    void write(uint8_t, uint8_t);
    void printf(int, const char*, ...);
    void setCGRAM(int id);

   private:
    const int LCD_ADRS = 0x3e;
    const int ONELINE = 1;
    const int TWOLINES = 0;
    const int DATA = 0x40;
    const int CMD = 0x00;
};