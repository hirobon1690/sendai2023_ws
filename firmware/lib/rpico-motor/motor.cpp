#include "motor.h"

Motor::Motor(int port0, int port1){
    pwm[0] = Pwm(port0);
    pwm[1] = Pwm(port1);
}

void Motor::init(){
    add_repeating_timer_ms(1, )
}
    

