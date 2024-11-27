#include "i2c.h"
I2c i2c;

I2c::I2c(i2c_inst_t* _ch, int _baud)
    : ch(_ch), baud(_baud) {
}

void I2c::init() {
    i2c_init(ch,baud);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
}

int I2c::write(uint8_t addr, uint8_t data) {
    return i2c_write_blocking(i2c_default, addr, &data, 1, 0);
}

int I2c::write(uint8_t addr, uint8_t* data, size_t size) {
    return i2c_write_blocking(i2c_default, addr, data, size, 0);
}

void I2c::read(uint8_t addr, uint8_t* data, size_t size) {
    i2c_read_blocking(i2c_default, addr,data,size,0); 		
}
