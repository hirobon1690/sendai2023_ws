#pragma once
#include "i2c.h"

class Pca9546 {
   public:
    Pca9546(uint8_t _addr);
    void select(uint8_t channel);
    void deselect();

   private:
    uint8_t addr;
};