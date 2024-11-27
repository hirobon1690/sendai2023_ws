#pragma once

class Pid {
   public:
    Pid();
    void update(float current);
    void setGain(float kp, float ki, float kd);
    void setGoal(float goal);
    void setDt(float dt);
    float calc();

   private:
    float kp, ki, kd;
    float dt;
    float integral;
    float error, prev_error;
    float goal;
    float current;
};
