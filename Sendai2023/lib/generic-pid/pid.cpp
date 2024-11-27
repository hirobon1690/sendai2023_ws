#include "pid.h"

Pid::Pid() {
    this->kp = 0;
    this->ki = 0;
    this->kd = 0;
    this->integral = 0;
    this->prev_error = 0;
    this->goal = 0;
    this->current = 0;
    this->error = 0;
}

void Pid::setGain(float kp, float ki, float kd) {
    this->kp = kp;
    this->ki = ki;
    this->kd = kd;
}

void Pid::setDt(float dt) {
    this->dt = dt;
}

void Pid::setGoal(float goal) {
    this->goal = goal;
}

void Pid::update(float current) {
    this->current = current;
}

float Pid::calc() {
    error = goal - current;
    integral += error*dt;
    float derivative = (error - prev_error)/dt;
    prev_error = error;
    return kp * error + ki * integral + kd * derivative;
}
