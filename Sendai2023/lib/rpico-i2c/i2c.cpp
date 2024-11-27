#include "i2c.h"
I2c i2c(i2c1, 100000);

I2c::I2c(i2c_inst_t* _ch, int _baud)
    : ch(_ch), baud(_baud) {
}

void I2c::init() {
    i2c_init(ch, baud);
    gpio_set_function(6, GPIO_FUNC_I2C);
    gpio_set_function(7, GPIO_FUNC_I2C);
    gpio_pull_up(6);
    gpio_pull_up(7);
}

int I2c::write(uint8_t addr, uint8_t data, bool nostop) {
    return i2c_write_blocking(i2c1, addr, &data, 1, nostop);
}

int I2c::write(uint8_t addr, uint8_t* data, size_t size, bool nostop) {
    return i2c_write_blocking(i2c1, addr, data, size, nostop);
}

int I2c::read(uint8_t addr, uint8_t* data, size_t size) {
    return i2c_read_blocking(i2c1, addr, data, size, 0);
}
