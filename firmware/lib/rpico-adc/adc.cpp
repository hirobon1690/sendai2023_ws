#include "adc.h"

Adc::Adc(int _pin) {
    pin = _pin;
}

void Adc::init() {
    adc_init();
    adc_gpio_init(pin);
}

uint16_t Adc::read() {
    adc_select_input(pin - 26);
    return adc_read();
}

float Adc::readVoltage() {
    adc_select_input(pin - 26);
    return adc_read() * 3.3f / (1 << 12);
}