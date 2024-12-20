#pragma once
#include "i2c.h"
#include "pca9546.h"

const int S11059_GAIN_HIGH = 1;
const int S11059_GAIN_LOW = 0;

const int S11059_MODE_MANUAL = 1;
const int S11059_MODE_FIXED = 0;

const int S11059_TINT0 = 0;
const int S11059_TINT1 = 1;
const int S11059_TINT2 = 2;
const int S11059_TINT3 = 3;

const int S11059_MANUAL_TIMING_MAX = 65535;
const int S11059_MANUAL_TIMING_MIN = 0;

enum Colors {
    NONE,
    BLUE,
    YELLOW,
    RED,
    WHITE
};

class S11059 {
   public:
    S11059();
    void setGain(uint8_t gain);
    void setMode(uint8_t mode);
    void setTint(uint8_t tint);
    void setManualTiming(uint16_t manualTiming);
    void delay();
    bool reset();
    bool standby();
    bool start();
    bool update();
    bool isStandby();
    void get(uint16_t*);
    uint16_t getRed();
    uint16_t getGreen();
    uint16_t getBlue();
    uint16_t getIR();

   private:
    uint16_t _rgbir[4];
    uint8_t _gain;
    uint8_t _mode;
    uint8_t _tint;
    uint16_t _manualTiming;

    uint8_t getControlBits();
    bool write(uint8_t value);
    bool write(uint8_t* values, size_t size);
    bool read(uint8_t* values, int size);
    bool readRegister(uint8_t address, uint8_t* value);
    bool readRegisters(uint8_t address, uint8_t* values, uint8_t size);
    bool writeRegister(uint8_t address, uint8_t value);
    bool writeRegisters(uint8_t address, uint8_t* values, uint8_t size);
};

class Color {
   public:
    Color(int id, int th0, int th1, int th2, int th3);
    void init();
    void read(uint16_t*);
    Colors getColor();
    int RGBtoHSV(int RGB[]);

   private:
    S11059 s11059;
    Pca9546 pca9546;
    int id;
    int th[4];
};
