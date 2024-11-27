#include "robot.h"
#include "timer.h"

Robot robot;
bool timer_cb(repeating_timer_t* rt) {
    robot.motor[0].timer_cb();
    robot.motor[1].timer_cb();
    return true;
}

bool timer_cb_pos(repeating_timer_t* rt) {
    robot.motor[0].timer_cb_pos();
    robot.motor[1].timer_cb_pos();
    return true;
}

void initTimer() {
    static repeating_timer_t timer;
    static repeating_timer_t timer1;
    static repeating_timer_t timer2;
    add_repeating_timer_ms(-10, timer_cb, NULL, &timer);
    add_repeating_timer_ms(-100, timer_cb_pos, NULL, &timer1);
}

int main() {
    robot.init();
    robot.main();
}
