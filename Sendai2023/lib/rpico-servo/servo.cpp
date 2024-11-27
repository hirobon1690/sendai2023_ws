#include "servo.h"

Servo::Servo(int pin)
    : pin(pin) {}

void Servo::init() {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(pin);
    channel = pwm_gpio_to_channel(pin);
    pwm_set_wrap(slice_num, 24999);
    pwm_set_clkdiv(slice_num, 100.0);
    pwm_set_chan_level(slice_num, channel, 0);
    pwm_set_enabled(slice_num, true);
}

void Servo::write_ms(float ms) {
    int level = int(ms / 0.0008);
    // printf("%d\n", level);
    pwm_set_chan_level(slice_num, channel, level);
    pwm_set_enabled(slice_num, 1);
}

void Servo::write(float val) {
    if (val < 0) {
        val = 0;
    } else if (val > 180) {
        val = 180;
    }
    val+=offset;
    write_ms(0.5 + (2.4 - 0.5) / 180 * val);
}

void Servo::setOffset(float val){
    offset = val;
}