#pragma once
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include "adc.h"
#include "encoder.h"
#include "gpio.h"
#include "motor.h"
#include "pico/stdlib.h"
#include "pid.h"
#include "pwm.h"

#include "Adafruit_NeoPixel.hpp"
#include "i2c.h"
#include "pca9546.h"
#include "s11059.h"
#include "servo.h"
#include "vl53l0x.h"

class Robot {
   public:
    Motor motor[2];
    Robot();
    bool line_check(repeating_timer_t* rt);
    void trace_cb();
    void buz(int times, int ms = 25);
    bool seek();
    void initSensor();
    void trace_to_ball_area();
    void trace_to_goal(Colors color);
    void init();
    void main();
    void setDir(bool dir);
    void setFwd();
    void setBwd();
    void rotate(float deg, int ms = 1000);
    void stop();
    void run(int speed);
    void translate(float deg, int ms = 1000);
    void flip();
    void setDeg(float deg);
    void resetPos();
    void showColor(Colors c);
    void waiting();
    void putBall(Colors c[]);
    void lineToBlue(int d = 0);
    void lineToRed(int d = 0);
    void lineToYellow(int d = 0);
    void freeBall();
    void parallel();
    void start();
    void mode1();
    void mode2();
    void mode3();
    void mode4();
    void mode5();
    void mode6();
    void mode7();
    void mode8();
    void mode9();
    void mode10();
    void mode11();
    void mode12();
    void mode13();
    void mode14();
    void mode15();
    void mode16();

   private:
    bool dir;
    int pdir;
    Encoder enc0;
    Encoder enc1;
    Pwm a;
    Pwm b;
    Gpio dir0;
    Gpio dir1;
    Gpio dir2;
    Gpio dir3;
    Gpio buzzer;
    Servo servo[2];
    Adc adc[4];
    Pca9546 pca9546;
    S11059 colorSensor;
    Pid trace;
    Gpio tofpin[2];
    VL53L0X tof[2];
    Color color[2];
    Colors inArm[2];
    Adafruit_NeoPixel pixels;
    Gpio enc[3];
    Gpio comm;
    int blackTimes;
    const int THRESHOLD = 600;
    bool chgFlag = false;
    int mode;
    int distTH = 35;
    int AREA_RADIUS = 400;
    int LENGTH = 180;
    int times = 0;
};
