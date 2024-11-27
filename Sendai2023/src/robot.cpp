#include "robot.h"
#include "timer.h"

// #define MUTE

Robot::Robot()
    : enc0(12),
      enc1(10),
      a(16, 500, 1, 0),
      b(19, 500, 1, 0),
      dir0(17),
      dir1(18),
      dir2(20),
      dir3(21),
      buzzer(5),
      servo{Servo(22), Servo(23)},
      adc{Adc(26), Adc(27), Adc(28), Adc(29)},
      pca9546(0x70),
      motor{Motor(dir0, dir1, a, enc0), Motor(dir2, dir3, b, enc1)},
      tofpin{Gpio(8), Gpio(9)},
      color{Color(0, 9000, 3000, 800, 500), Color(1, 10500, 1100, 1100, 1400)},
      pixels(16, 24, NEO_GRB + NEO_KHZ800),
      enc{Gpio(4, INPUT_PU), Gpio(3, INPUT_PU), Gpio(2, INPUT_PU)},
      comm(0) {
}

bool Robot::line_check(repeating_timer_t* rt) {
    static int cnt;
    int sensor[2] = {adc[0].read(), adc[3].read()};
    if (sensor[0] < THRESHOLD && sensor[1] < THRESHOLD) {
        cnt++;
    }
    if (cnt > 5) {
        blackTimes++;
        cnt = 0;
        chgFlag = true;
    }
    return true;
}
void Robot::trace_cb() {
    int sensor[4] = {adc[0].read(), int(adc[1].read()), int(adc[2].read()), adc[3].read()};
    printf("%d %d %d %d %d\n", sensor[0], sensor[1], sensor[2], sensor[3], blackTimes);
    trace.update(sensor[1] - sensor[2]);
    float calc = trace.calc();
    motor[0].setVel(dir ? -(450 + calc) : (450 + calc));
    motor[1].setVel(dir ? -(-450 + calc) : (-450 + calc));
    // printf("%f %f\n", 300+calc, -300+calc);
    line_check(NULL);
    sleep_ms(10);
}

void Robot::buz(int times, int ms) {
    for (int i = 0; i < times; i++) {
#ifndef MUTE
        buzzer.write(1);
#endif
        sleep_ms(ms);
        buzzer.write(0);
        sleep_ms(60);
    }
}

bool Robot::seek() {
    sleep_ms(1000);
    buz(2);
    rotate(60, 2000);
    servo[dir].write(5);
    sleep_ms(1000);
    std::vector<int> distance;
    for (int i = 120; i > -120; i -= 2) {
        setDeg(i / 2);
        distance.push_back(tof[dir].readRangeSingleMillimeters());
    }
    int targetDeg = 120 - (int)(std::distance(distance.begin(), std::min_element(distance.begin(), distance.end())) * 2);
    printf("targetdeg: %d\n", targetDeg);
    std::vector<int> distance2;
    for (int i = -120; i < 120; i += 2) {
        setDeg(i / 2);
        distance2.push_back(tof[dir].readRangeSingleMillimeters());
    }
    int targetDeg2 = -120 + (int)(std::distance(distance2.begin(), std::min_element(distance2.begin(), distance2.end())) * 2);
    printf("targetdeg2: %d\n", targetDeg2);
    if (abs(targetDeg2 - targetDeg) > 30) {
        targetDeg = targetDeg;
    } else {
        targetDeg = (targetDeg + targetDeg2) / 2;
    }

    if (*std::min_element(distance.begin(), distance.end()) + *std::min_element(distance2.begin(), distance2.end()) > AREA_RADIUS) {
        setDeg(0);
        sleep_ms(1000);
        stop();
        servo[dir].write(40);
        return 0;  // no ball found
    }
    setDeg((targetDeg / 2) + 14);
    sleep_ms(2000);
    servo[dir].write(0);
    resetPos();
    run(90);
    int t = 0;
    while (1) {
        int kyori = tof[dir].readRangeSingleMillimeters();
        printf("%d\n", kyori);
        if (kyori < distTH || t > 150) {
            stop();
            break;
        }
        sleep_ms(10);
        t++;
    }
    stop();
    servo[dir].write(0);
    sleep_ms(1000);
    for (int i = 0; i < 40; i++) {
        servo[dir].write(i);
        sleep_ms(10);
    }
    sleep_ms(1000);
    setDeg(0);
    sleep_ms(2000);
    if (!dir) {
        rotate(-180 - targetDeg / 2, 2000);
    } else {
        rotate(-targetDeg / 2);
    }
    stop();
    resetPos();
    return 1;
}

void Robot::initSensor() {
    i2c.init();
    tofpin[0].init();
    tofpin[1].init();
    tofpin[0].write(0);
    tofpin[1].write(0);
    sleep_ms(10);
    gpio_set_dir(8, GPIO_IN);
    tofpin[0].deinit();
    sleep_ms(10);
    tof[0].setTimeout(500);
    if (!tof[0].init()) {
        printf("Failed to detect and initialize sensor0!\n");
        while (1) {
            printf("Failed to detect and initialize sensor0!\n");
            sleep_ms(10);
        }
    }
    tof[0].setAddress(0x40);
    sleep_ms(10);
    gpio_set_dir(9, GPIO_IN);
    tofpin[1].deinit();
    sleep_ms(10);
    tof[1].setTimeout(500);
    if (!tof[1].init()) {
        printf("Failed to detect and initialize sensor1!\n");
        while (1) {
            printf("Failed to detect and initialize sensor1!\n");
            sleep_ms(10);
        }
    }
    printf("tof init done\n");
    printf("color init\n");
    color[0].init();
    color[1].init();
    printf("color init done\n");
    tof[1].setCalib(0.81);
    comm.init();
}

void Robot::trace_to_ball_area() {
    AREA_RADIUS += times * 30;
    sleep_ms(1);
    pixels.setPixelColor(times < 5 ? times + 2 : times + 5, pixels.Color(1, 15, 0));
    pixels.show();
    while (1) {
        trace_cb();
        if (chgFlag) {
            buz(1);
            chgFlag = false;
        }
        if (blackTimes >= 4) {
            blackTimes = 0;

            buz(1);
            if (times < 3) {
                stop();
                // translate(-50, 1500);
                // stop();
                // parallel();
                stop();
                sleep_ms(1000);
                // parallel();
                flip();
                for (int i = 0; i < 80; i++) {
                    trace_cb();
                }
                flip();
                // translate(-80, 1500);
            } else {
                for (int i = 0; i < 40 * times; i++) {
                    trace_cb();
                }
            }
            stop();
            sleep_ms(1000);
            times++;
            break;
        }
    }
}

void ::Robot::parallel() {
    int i = 0;
    resetPos();

    while (1) {
        setDeg(-i);
        if (adc[0].read() < THRESHOLD && adc[1].read() < THRESHOLD && adc[2].read() < THRESHOLD && adc[3].read() < THRESHOLD) {
            sleep_ms(100);
            stop();
            resetPos();
            return;
            break;
        }
        if (i > 30) {
            buz(1);
            setDeg(0);
            sleep_ms(1000);
            stop();
            resetPos();
            break;
        }
        sleep_ms(100);
        i++;
    }
    i = 0;
    while (1) {
        setDeg(i);
        if (adc[0].read() < THRESHOLD && adc[1].read() < THRESHOLD && adc[2].read() < THRESHOLD && adc[3].read() < THRESHOLD) {
            sleep_ms(100);
            stop();
            resetPos();
            return;
            break;
        }
        if (i > 30) {
            buz(1);
            setDeg(0);
            sleep_ms(1000);
            stop();
            resetPos();
            break;
        }
        sleep_ms(100);
        i++;
    }
}

void Robot::freeBall() {
    while (1) {
        trace_cb();
        if (chgFlag) {
            buz(1);
            chgFlag = false;
        }
        if (blackTimes == 2) {
            // lineToYellow(3);
            int i = 0;
            translate(-25);
            buz(1);
            rotate(-90, 2000);
            buz(2);
            stop();
            // for (int i = 0; i < 160; i++) {
            //     trace_cb();
            // }
            translate(LENGTH, 1500);
            stop();
            sleep_ms(500);
            buz(3);
            stop();
            servo[dir].write(0);
            showColor(NONE);
            sleep_ms(1000);
            blackTimes = 0;
            flip();
            translate((int)LENGTH * 1.15, 1500);
            stop();
            // while (blackTimes < 1) {
            //     // trace_cb();
            //     if (i > 180) {
            //         stop();
            //         resetPos();
            //         break;
            //     }
            //     i++;
            // }
            flip();
            stop();
            sleep_ms(500);
            servo[dir].write(35);
            rotate(90, 2000);
            blackTimes = 1;
            stop();
            blackTimes = 1;
            for (int i = 0; i < 200; i++) {
                trace_cb();
            }
            return;
        }
    }
}

void Robot::lineToBlue(int d) {
    int i = 0;
    motor[0].resetPos();
    motor[1].resetPos();
    translate(400, 3000);
    servo[dir].write(0);
    showColor(NONE);
    sleep_ms(1000);
    flip();
    translate(350, 2000);
    stop();
    servo[!dir].write(35);
    sleep_ms(1000);
    while (blackTimes < 1) {
        trace_cb();
        if (i > 500) {
            break;
        }
        i++;
    }
    stop();
    blackTimes = 2;
    return;
}

void Robot::lineToRed(int d) {
    int i = 0;
    translate(-25);
    buz(1);
    switch (d) {
        case 0:
            rotate(90, 2000);
            break;
        case 1:
            rotate(-90, 2000);
            flip();
            break;
        case 2:
            rotate(90, 2000);
            break;
    }
    buz(2);
    stop();
    translate(LENGTH, 1500);
    // for (int i = 0; i < 160; i++) {
    //     trace_cb();
    // }
    stop();
    sleep_ms(500);
    buz(3);
    servo[dir].write(0);
    showColor(NONE);
    sleep_ms(1000);
    blackTimes = 0;
    flip();
    i = 0;
    // {
    //     int cnt = 0;
    //     while (1) {
    //         trace_cb();
    //         int s[2] = {adc[0].read(), adc[3].read()};
    //         if (s[!dir] < THRESHOLD) {
    //             cnt++;
    //         }
    //         if (i > 190 || cnt > 4) {
    //             break;
    //         }
    //         i++;
    //     }
    // }
    translate(LENGTH, 1500);
    // translate(35);
    flip();
    stop();
    sleep_ms(500);
    servo[dir].write(35);
    switch (d) {
        case 0:
            rotate(90, 2000);
            break;
        case 1:
            rotate(-90, 2000);
            flip();
            break;
        case 2:
            rotate(-90, 2000);
            flip();
            break;
    }
    blackTimes = 0;
    stop();
    return;
}

void Robot::lineToYellow(int d) {
    int i = 0;
    translate(-25);
    buz(1);
    switch (d) {
        case 0:
            rotate(90, 2000);
            break;
        case 1:
            rotate(-90, 2000);
            flip();
            break;
        case 2:
            rotate(90, 2000);
            break;
        case 3:
            rotate(-90, 2000);
            break;
    }
    buz(2);
    stop();
    // for (int i = 0; i < 160; i++) {
    //     trace_cb();
    // }
    translate(LENGTH, 1500);
    stop();
    sleep_ms(500);
    buz(3);
    stop();
    servo[dir].write(0);
    showColor(NONE);
    sleep_ms(1000);
    blackTimes = 0;
    flip();
    // while (blackTimes < 1) {
    //     trace_cb();
    //     if (i > 180) {
    //         break;
    //     }
    //     i++;
    // }
    translate(LENGTH, 1500);
    flip();
    stop();
    sleep_ms(500);
    servo[dir].write(35);
    switch (d) {
        case 0:
            rotate(90, 2000);
            break;
        case 1:
            rotate(-90, 2000);
            flip();
            break;
        case 2:
            rotate(90, 2000);
            break;
        case 3:
            rotate(90, 2000);
            break;
    }
    blackTimes = 1;
    stop();
    return;
}

void Robot::trace_to_goal(Colors color) {
    blackTimes = 0;
    while (1) {
        trace_cb();
        if (chgFlag) {
            buz(1);
            chgFlag = false;
        }
        if (blackTimes == (int)color + 1) {
            blackTimes = 0;
            stop();
            buz(1, 1000);
            int i = 0;
            switch (color) {
                case BLUE:
                    lineToBlue();
                    break;
                case YELLOW:
                    lineToYellow();
                    break;
                case RED:
                    lineToRed();
                    break;
                default:
                    break;
            }
            break;
        }
    }
}

void Robot::putBall(Colors c[]) {
    setFwd();
    // if (times < 3) {
    //     for (int i = 0; i < 300; i++) {
    //         trace_cb();
    //     }
    //     blackTimes = 1;
    // } else {
    //     blackTimes = 0;
    // }
    if (c[0] == BLUE) {
        if (c[1] == BLUE) {
            // blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == (int)c[0] + 1) {
                    blackTimes = 0;
                    stop();
                    sleep_ms(1000);
                    buz(1, 1000);
                    int i = 0;
                    motor[0].resetPos();
                    motor[1].resetPos();
                    translate(400, 3000);
                    servo[dir].write(0);
                    showColor(NONE);
                    sleep_ms(1000);
                    rotate(180, 1000);
                    flip();
                    servo[dir].write(0);
                    showColor(NONE);
                    flip();
                    translate(300, 2000);
                    stop();
                    servo[0].write(35);
                    servo[1].write(35);
                    sleep_ms(1000);
                    while (blackTimes < 1) {
                        trace_cb();
                        if (i > 500) {
                            break;
                        }
                        i++;
                    }
                    stop();
                    blackTimes = 3;
                    return;
                    break;
                }
            }

        } else if (c[1] == YELLOW) {
            // blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == (int)c[0] + 1) {
                    lineToBlue();
                    break;
                }
            }
            flip();
            blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == 1) {
                    lineToYellow(1);
                    return;
                    break;
                }
            }
        } else if (c[1] == RED) {
            // blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == (int)c[0] + 1) {
                    stop();
                    lineToBlue();
                    break;
                }
            }
            flip();
            blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == 2) {
                    lineToRed(1);
                    return;
                    break;
                }
            }
        } else if (c[1] == NONE) {
            trace_to_goal(c[0]);
        }
    } else if (c[0] == YELLOW) {
        if (c[1] == BLUE) {
            // blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == (int)c[0] + 1) {
                    lineToYellow();
                    break;
                }
            }
            for (int i = 0; i < 150; i++) {
                trace_cb();
            }
            blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == 1) {
                    stop();
                    sleep_ms(1000);
                    flip();
                    lineToBlue();
                    blackTimes = 2;
                    return;
                    break;
                }
            }
        } else if (c[1] == YELLOW) {
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == (int)c[0] + 1) {
                    int i = 0;
                    translate(-35);
                    buz(1);
                    rotate(90, 2000);
                    buz(2);
                    stop();
                    for (int i = 0; i < 130; i++) {
                        trace_cb();
                    }
                    stop();
                    sleep_ms(500);
                    buz(3);
                    stop();
                    servo[dir].write(0);
                    showColor(NONE);
                    sleep_ms(1000);
                    blackTimes = 0;
                    rotate(180);
                    // flip();
                    servo[dir].write(35);
                    servo[!dir].write(0);
                    sleep_ms(1000);
                    while (blackTimes < 1) {
                        trace_cb();
                        if (i > 140) {
                            break;
                        }
                        i++;
                    }
                    flip();
                    stop();
                    sleep_ms(500);
                    servo[dir].write(35);
                    rotate(-90, 2000);
                    flip();
                    blackTimes = 1;
                    stop();
                    return;
                }
            }
        } else if (c[1] == RED) {
            // blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == (int)c[0] + 1) {
                    lineToYellow();
                    break;
                }
            }
            blackTimes = 0;
            flip();
            for (int i = 0; i < 100; i++) {
                trace_cb();
            }
            blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes >= 1) {
                    lineToRed(2);
                    stop();
                    for (int i = 0; i < 100; i++) {
                        trace_cb();
                    }
                    blackTimes = 1;
                    return;
                    break;
                }
            }
        } else if (c[1] == NONE) {
        }
    } else if (c[0] == RED) {
        if (c[1] == BLUE) {
            // blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == (int)c[0] + 1) {
                    lineToRed();
                    break;
                }
            }
            for (int i = 0; i < 160; i++) {
                trace_cb();
            }
            blackTimes = 1;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == 3) {
                    stop();
                    sleep_ms(1000);
                    flip();
                    lineToBlue();
                    blackTimes = 2;
                    return;
                    break;
                }
            }
        } else if (c[1] == YELLOW) {
            // blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == (int)c[0] + 1) {
                    lineToRed();
                    break;
                }
            }
            blackTimes = 0;
            while (1) {
                trace_cb();
                if (chgFlag) {
                    buz(1);
                    chgFlag = false;
                }
                if (blackTimes == 1) {
                    flip();
                    lineToYellow(2);
                    return;
                    break;
                }
            }
        } else if (c[1] == RED) {
            while (1) {
                // trace_cb();
                // if (chgFlag) {
                //     buz(1);
                //     chgFlag = false;
                // }
                // if (blackTimes >= 0){
                    stop();
                    rotate(-90, 2000);
                    buz(2);
                    stop();
                    translate(180, 1500);
                    stop();
                    servo[dir].write(0);
                    comm.write(1);
                    sleep_ms(500);
                    comm.write(0);
                    sleep_ms(10000);
                    rotate(180);
                    flip();
                    servo[dir].write(0);
                    comm.write(1);
                    sleep_ms(500);
                    comm.write(0);
                    sleep_ms(1000);
                    flip();
                    translate(180, 1500);
                    rotate(-90, 2000);
                    blackTimes=0;
                // }
                // if (blackTimes == (int)c[0] + 1) {
                //     int i = 0;
                //     translate(-35);
                //     buz(1);
                //     rotate(90, 2000);
                //     buz(2);
                //     stop();
                //     // for (int i = 0; i < 130; i++) {
                //     //     trace_cb();
                //     // }
                //     translate(LENGTH, 1500);
                //     stop();
                //     sleep_ms(500);
                //     buz(3);
                //     stop();
                //     servo[dir].write(0);
                //     showColor(NONE);
                //     sleep_ms(1000);
                //     blackTimes = 0;
                //     rotate(180);
                //     // flip();
                //     servo[dir].write(35);
                //     servo[!dir].write(0);
                //     flip();
                //     showColor(NONE);
                //     flip();
                //     sleep_ms(1000);
                //     // while (blackTimes < 1) {
                //     //     trace_cb();
                //     //     if (i > 140) {
                //     //         break;
                //     //     }
                //     //     i++;
                //     // }
                //     translate(LENGTH * 0.98, 1500);
                //     flip();
                //     stop();
                //     sleep_ms(500);
                //     servo[dir].write(35);
                //     rotate(-90, 2000);
                //     flip();
                //     blackTimes = 1;
                //     stop();
                //     return;
                // }
            }
        } else if (c[1] == NONE) {
        }
    } else if (c[0] == NONE) {
        if (c[1] == BLUE) {
        } else if (c[1] == YELLOW) {
        } else if (c[1] == RED) {
        } else if (c[1] == NONE) {
        }
    }
}

void Robot::init() {
    stdio_init_all();
    pixels.clear();
    for (int i = 0; i < 16; i++) {
        pixels.setPixelColor(i, pixels.ColorHSV(i * 4096, 255, 10));
    }
    pixels.show();
    buzzer.init();
    servo[0].init();
    servo[1].init();

    i2c.init();
    pca9546.select(0);
    buz(3);
    servo[0].init();
    servo[1].init();
    servo[0].setOffset(50);
    servo[1].setOffset(55);
    servo[0].write(35);
    servo[1].write(35);

    adc[0].init();
    adc[1].init();
    adc[2].init();
    adc[3].init();
    adc[0].setCali(1.2);
    adc[1].setCali(0.94);
    adc[2].setCali(1.1);
    adc[1].setCali(0.93);
    enc[0].init();
    enc[1].init();
    enc[2].init();
    initSensor();

    initTimer();
    motor[0].init();
    motor[1].init();

    motor[0].disablePosPid();
    motor[1].disablePosPid();

    trace.setGain(0.36, 0.021, 0.12);
    trace.setDt(0.01);
    trace.setGoal(0);

    buz(1, 500);
}

void Robot::showColor(Colors c) {
    sleep_ms(1);
    int ledid = 0;
    if (dir == 1) {
        ledid = 8;
    }
    switch (c) {
        case RED:
            pixels.setPixelColor(ledid, pixels.Color(100, 0, 0));
            break;
        case BLUE:
            pixels.setPixelColor(ledid, pixels.Color(0, 0, 100));
            break;
        case YELLOW:
            pixels.setPixelColor(ledid, pixels.Color(100, 100, 0));
            break;
        case WHITE:
            pixels.setPixelColor(ledid, pixels.Color(100, 100, 100));
            break;
        case NONE:
            pixels.setPixelColor(ledid, pixels.Color(0, 0, 0));
            break;
        default:
            break;
    }
    pixels.show();
}

void Robot::main() {
    mode = 2;
    while (1) {
        static int prevstat;
        char status = enc[0].read();
        if (prevstat != status && status == 1) {
            printf("c\n");
            switch (enc[1].read()) {
                case 0:
                    buz(1, 10);
                    mode++;
                    if (mode > 16) {
                        mode = 0;
                    }
                    sleep_ms(1);
                    break;
                case 1:
                    buz(1, 10);
                    mode--;
                    if (mode < 0) {
                        mode = 16;
                    }
                    sleep_ms(1);
                    break;
                default:
                    break;
            }
        }
        printf("%d\n", mode);
        prevstat = status;
        sleep_ms(1);
        for (int i = 0; i < 16; i++) {
            pixels.setPixelColor(i, mode <= i ? pixels.Color(0, 0, 0) : pixels.Color(0, 10, 10));
        }
        pixels.show();
        if (!enc[2].read()) {
            pixels.clear();
            buz(1, 50);
            pixels.show();
            break;
        }
    }
    switch (mode) {
        case 1:
            mode1();
            break;
        case 2:
            mode2();
            break;
        case 3:
            mode3();
            break;
        case 4:
            mode4();
            break;
        case 5:
            mode5();
            break;
        case 6:
            mode6();
            break;
        case 7:
            mode7();
            break;
        case 8:
            mode8();
            break;
        case 9:
            mode9();
            break;
        case 10:
            mode10();
            break;
        case 11:
            mode11();
            break;
        case 12:
            mode12();
            break;
        case 13:
            mode13();
            break;
        case 14:
            mode14();
            break;
        case 15:
            mode15();
            break;
        case 16:
            mode16();
            break;
        default:
            break;
    }
    while (1)
        ;
}

void Robot::setDir(bool dir) {
    this->dir = dir;
    dir ? pdir = 1 : pdir = -1;
}

void Robot::setFwd() {
    setDir(0);
    sleep_ms(1);
    pixels.setPixelColor(1, pixels.Color(10, 10, 10));
    pixels.setPixelColor(7, pixels.Color(0, 0, 0));
    pixels.setPixelColor(9, pixels.Color(0, 0, 0));
    pixels.setPixelColor(15, pixels.Color(10, 10, 10));
    pixels.show();
}

void Robot::setBwd() {
    setDir(1);
    sleep_ms(1);
    pixels.setPixelColor(1, pixels.Color(0, 0, 0));
    pixels.setPixelColor(7, pixels.Color(10, 10, 10));
    pixels.setPixelColor(9, pixels.Color(10, 10, 10));
    pixels.setPixelColor(15, pixels.Color(0, 0, 0));
    pixels.show();
}

void Robot::flip() {
    if (dir) {
        setFwd();
    } else {
        setBwd();
    }
}

void Robot::rotate(float deg, int ms) {
    motor[0].resetPos();
    motor[1].resetPos();
    setDeg(deg);
    sleep_ms(ms);
}

void Robot::setDeg(float deg) {
    motor[0].setPos(deg * 2);
    motor[1].setPos(deg * 2);
}

void Robot::stop() {
    motor[0].disablePosPid();
    motor[1].disablePosPid();
    motor[0].setVel(0);
    motor[1].setVel(0);
}

void Robot::resetPos() {
    motor[0].resetPos();
    motor[1].resetPos();
}

void Robot::run(int speed) {
    motor[0].disablePosPid();
    motor[1].disablePosPid();
    motor[0].setVel(-pdir * speed);
    motor[1].setVel(pdir * speed);
}

void Robot::translate(float deg, int ms) {
    motor[0].resetPos();
    motor[1].resetPos();
    motor[0].setPos(-pdir * deg * 2);
    motor[1].setPos(pdir * deg * 2);
    sleep_ms(ms);
}

void Robot::mode1() {
    start();
    freeBall();
    while (1) {
        setFwd();
        trace_to_ball_area();
        for (int d = 0; d < 2; d++) {
            while (!seek()) {
                for (int i = 0; i < 80; i++) {
                    trace_cb();
                }
                stop();
            }
            sleep_ms(1000);
            Colors c = color[dir].getColor();
            showColor(c);
            buz(c);
            inArm[dir] = c;
            flip();
        }
        sleep_ms(1000);
        buz(inArm[dir]);
        resetPos();
        putBall(inArm);
    }
}

void Robot::mode2() {
    while (1) {
        uint16_t a[4];
        uint16_t b[4];

        color[0].read(a);
        color[1].read(b);
        int adeg = color[0].RGBtoHSV((int*)a);
        int bdeg = color[1].RGBtoHSV((int*)b);
        // printf("%d, %d\n",tof[0].readRangeSingleMillimeters(),tof[1].readRangeSingleMillimeters());
        printf("%5d, %5d, %5d, %5d| %5d, %5d, %5d, %5d\n", a[0], a[1], a[2], adeg, b[0], b[1], b[2], bdeg);
        setFwd();
        showColor(color[0].getColor());
        setBwd();
        showColor(color[1].getColor());
    }
}

void Robot::mode3() {
    while (1) {
        servo[0].write(0);
        servo[1].write(0);
        buz(1);
        sleep_ms(2000);
        for (int i = 0; i < 40; i++) {
            servo[0].write(i);
            servo[1].write(i);
            sleep_ms(10);
        }
        buz(1);
        sleep_ms(2000);
    }
}

void Robot::mode4() {
    while (1) {
        setFwd();
        seek();
        Colors c = color[dir].getColor();
        showColor(c);
        buz(c);
        sleep_ms(1000);
        pixels.clear();
        pixels.show();
        servo[dir].write(0);
        showColor(NONE);
        sleep_ms(1000);
        buz(1, 500);
        servo[dir].write(40);
        sleep_ms(1000);
    }
}

void Robot::mode5() {
    while (1) {
        setBwd();
        seek();
        Colors c = color[dir].getColor();
        showColor(c);
        buz(c);
        sleep_ms(1000);
        pixels.clear();
        pixels.show();
        servo[dir].write(0);
        showColor(NONE);
        sleep_ms(1000);
        buz(1, 500);
        servo[dir].write(40);
        sleep_ms(1000);
    }
}

void Robot::mode6() {
    while (1) {
        setFwd();
        showColor(RED);
        buz(RED);
        trace_to_goal(RED);
    }
}

void Robot::mode7() {
    while (1) {
        setFwd();
        showColor(BLUE);
        sleep_ms(1000);
        trace_to_goal(BLUE);
    }
}

void Robot::mode8() {
    while (1) {
        setFwd();
        showColor(YELLOW);
        sleep_ms(1000);
        trace_to_goal(YELLOW);
    }
}

void Robot::mode9() {
    servo[0].write(0);
    servo[1].write(0);
    while (1) {
        printf("%d, %d\n", tof[0].readRangeSingleMillimeters(), tof[1].readRangeSingleMillimeters());
        sleep_ms(100);
        if (tof[0].readRangeSingleMillimeters() < distTH) {
            setFwd();
            showColor(NONE);
        } else if (tof[0].readRangeSingleMillimeters() > distTH) {
            setFwd();
            showColor(WHITE);
        }
        if (tof[1].readRangeSingleMillimeters() < distTH) {
            setBwd();
            showColor(NONE);
        } else if (tof[1].readRangeSingleMillimeters() > distTH) {
            setBwd();
            showColor(WHITE);
        }
    }
}

void Robot::mode10() {
    while (1) {
        int a[4] = {adc[0].read(), adc[1].read(), adc[2].read(), adc[3].read()};
        printf("%d, %d, %d, %d\n", a[0], a[1], a[2], a[3]);
        sleep_ms(100);
    }
}

void Robot::mode11() {
    start();
    freeBall();
    blackTimes = 2;
    while (1) {
        setFwd();
        trace_to_ball_area();
        for (int d = 0; d < 2; d++) {
            while (!seek()) {
                for (int i = 0; i < 80; i++) {
                    trace_cb();
                }
                stop();
            }
            sleep_ms(1000);
            Colors c = color[dir].getColor();
            showColor(c);
            buz(c);
            inArm[dir] = c;
            flip();
            // for (int i = 0; i < 20; i++) {
            //     trace_cb();
            // }
        }
        sleep_ms(1000);
        buz(inArm[dir]);
        resetPos();
        putBall(inArm);
    }
}

void Robot::mode12() {
    motor[0].setVel(90);
    motor[1].setVel(90);
    while (1) {
        for (int j = 0; j < 16; j++) {
            for (int i = 0; i < 16; i++) {
                pixels.setPixelColor(i, pixels.ColorHSV((i + j > 16 ? i + j - 16 : i + j) * 4096, 255, 10));
            }
            pixels.show();
            sleep_ms(100);
        }
    }

    while (1)
        ;
}

void Robot::mode13() {
    parallel();
}

void Robot::mode14() {
    setFwd();
    servo[0].write(35);
    servo[1].write(35);
    // Colors c[2] = {color[0].getColor(), color[1].getColor()};
    Colors c[2] = {BLUE, RED};
    setFwd();
    showColor(c[0]);
    setBwd();
    showColor(c[1]);
    putBall(c);
}

void Robot::mode15() {
    // run(90);
    while (1) {
        start();
        /* code */
    }
}

void Robot::mode16() {
    setFwd();
    servo[0].write(35);
    servo[1].write(35);
    // Colors c[2] = {color[0].getColor(), color[1].getColor()};
    Colors c[2] = {YELLOW, RED};
    setFwd();
    showColor(c[0]);
    setBwd();
    showColor(c[1]);
    putBall(c);
    buz(1);
    stop();
}

void Robot::start() {
    resetPos();
    setFwd();
    translate(270, 2000);
    resetPos();
    stop();
    buz(2);
}