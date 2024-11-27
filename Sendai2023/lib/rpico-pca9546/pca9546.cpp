#include "pca9546.h"

Pca9546::Pca9546(uint8_t _addr)
    : addr(_addr) {
}

void Pca9546::select(uint8_t channel) {
    i2c.write(addr, 1 << channel, false);
}

void Pca9546::deselect() {
    i2c.write(addr, 0, false);
}