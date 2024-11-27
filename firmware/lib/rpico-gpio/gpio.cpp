#include "gpio.h"

Gpio::Gpio(int _pin, int _mode)
    : pin(_pin), mode(_mode) {
}

void Gpio::init() {
    gpio_init(pin);
    switch (mode) {
        case OUTPUT:
            gpio_set_dir(pin, GPIO_OUT);
            break;
        case INPUT:
            gpio_set_dir(pin, GPIO_IN);
            break;
        case INPUT_PU:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
            break;

        case INPUT_PD:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_down(pin);
            break;

        default:
            break;
    }
    state = 0;
}

void Gpio::write(bool val) {
    if (mode == GPIO_OUT) {
        gpio_put(pin, val);
        state = val;
    }
}

bool Gpio::read() {
    return gpio_get(pin);
}

void Gpio::toggle() {
    write(!state);
}