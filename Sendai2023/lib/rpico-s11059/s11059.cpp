#include "s11059.h"
#include <algorithm>
using std::max;
using std::min;

// I2C address
#define I2C_ADDR 0x2A

// Register Map
#define REG_CONTROL 0x00        // 1 byte
#define REG_MANUAL_TIMING 0x01  // 2 bytes
#define REG_DATA_RED 0x03       // 2 bytes
#define REG_DATA_GREEN 0x05     // 2 bytes
#define REG_DATA_BLUE 0x07      // 2 bytes
#define REG_DATA_IR 0x09        // 2 bytes

// Control Bits
#define CONTROL_BIT_ADC_RESET 7
#define CONTROL_BIT_STANDBY_FUNCTION 6
#define CONTROL_BIT_STANDBY_FUNCTION_MONITOR 5
#define CONTROL_BIT_GAIN_SELECTION 3
#define CONTROL_BIT_INTEGRATION_MODE 2
#define CONTROL_BIT_INTEGRATION_TIME_SETTING_HIGH 1
#define CONTROL_BIT_INTEGRATION_TIME_SETTING_LOW 0

// Control Bit Masks
#define CONTROL_BIT_MASK_ADC_RESET 0x80
#define CONTROL_BIT_MASK_STANDBY_FUNCTION 0x40
#define CONTROL_BIT_MASK_STANDBY_FUNCTION_MONITOR 0x20
#define CONTROL_BIT_MASK_GAIN_SELECTION 0x08
#define CONTROL_BIT_MASK_INTEGRATION_MODE 0x04
#define CONTROL_BIT_MASK_INTEGRATION_TIME_SETTING 0x03

// Integration Times (For Fixed period mode)
#define INTEGRATION_TIME_0_US 88      // 87.5 us
#define INTEGRATION_TIME_1_US 1400    // 1.4 ms
#define INTEGRATION_TIME_2_US 22400   // 22.4 ms
#define INTEGRATION_TIME_3_US 179200  // 179.2 ms

// Tint Times (For Manual setting mode)
#define TINT_TIME_0_US 175     // 175 us
#define TINT_TIME_1_US 2800    // 2.8 ms
#define TINT_TIME_2_US 44800   // 44.8 ms
#define TINT_TIME_3_US 358400  // 358.4ms

S11059::S11059() {
    for (int i = 0; i < 4; i++) {
        _rgbir[i] = 0;
    }

    // default value from datasheet
    _gain = S11059_GAIN_LOW;
    _mode = S11059_MODE_MANUAL;
    _tint = S11059_TINT0;
    _manualTiming = 0x0C30;
}

void S11059::setGain(uint8_t gain) {
    _gain = gain;
}

void S11059::setMode(uint8_t mode) {
    _mode = mode;
}

void S11059::setTint(uint8_t tint) {
    _tint = tint;
}

void S11059::setManualTiming(uint16_t manualTiming) {
    _manualTiming = manualTiming;
}

void S11059::delay() {
    uint32_t ms = 0;
    uint32_t us = 0;
    if (_mode == S11059_MODE_MANUAL) {
        int64_t tmp = 0;
        switch (_tint) {
            case S11059_TINT0:
                tmp = (int64_t)TINT_TIME_0_US * (int64_t)_manualTiming;
                break;

            case S11059_TINT1:
                tmp = (int64_t)TINT_TIME_1_US * (int64_t)_manualTiming;
                break;

            case S11059_TINT2:
                tmp = (int64_t)TINT_TIME_2_US * (int64_t)_manualTiming;
                break;

            case S11059_TINT3:
                tmp = (int64_t)TINT_TIME_3_US * (int64_t)_manualTiming;
                break;
        }
        ms = tmp / 1000;
        us = tmp % 1000;
    } else {
        switch (_tint) {
            case S11059_TINT0:
                us = INTEGRATION_TIME_0_US;
                break;

            case S11059_TINT1:
                us = INTEGRATION_TIME_1_US;
                break;

            case S11059_TINT2:
                us = INTEGRATION_TIME_2_US;
                break;

            case S11059_TINT3:
                us = INTEGRATION_TIME_3_US;
                break;
        }
    }

    // add careful margin
    ms += 1;

    // delay for 4 colors
    sleep_ms(ms * 4);
    sleep_us(us * 4);
}

bool S11059::reset() {
    if (_mode == S11059_MODE_MANUAL) {
        uint8_t bytes[2] = {
            (uint8_t)((_manualTiming >> 8) & 0xFF),
            (uint8_t)(_manualTiming & 0xFF)};
        if (!writeRegisters(REG_MANUAL_TIMING, bytes, sizeof(bytes))) {
            return false;
        }
    }

    uint8_t controlBits = getControlBits();
    controlBits |= CONTROL_BIT_MASK_ADC_RESET;
    controlBits &= ~(CONTROL_BIT_MASK_STANDBY_FUNCTION);
    return writeRegister(REG_CONTROL, controlBits);
}

bool S11059::standby() {
    uint8_t controlBits = getControlBits();
    controlBits |= CONTROL_BIT_MASK_STANDBY_FUNCTION;
    return writeRegister(REG_CONTROL, controlBits);
}

bool S11059::start() {
    uint8_t controlBits = getControlBits();
    controlBits &= ~(CONTROL_BIT_MASK_ADC_RESET);
    controlBits &= ~(CONTROL_BIT_MASK_STANDBY_FUNCTION);
    bool result = writeRegister(REG_CONTROL, controlBits);
    return result;
}

bool S11059::update() {
    // Read all data registers (03 - 0A)
    uint8_t values[8];
    if (readRegisters(REG_DATA_RED, values, sizeof(values))) {
        // convert endian
        _rgbir[0] = (int16_t)((values[0] << 8) | values[1]);
        _rgbir[1] = (int16_t)((values[2] << 8) | values[3]);
        _rgbir[2] = (int16_t)((values[4] << 8) | values[5]);
        _rgbir[3] = (int16_t)((values[6] << 8) | values[7]);
        return true;
    } else {
        return false;
    }
}

bool S11059::isStandby() {
    uint8_t value = 0x00;
    if (readRegister(REG_CONTROL, &value)) {
        return value & CONTROL_BIT_MASK_STANDBY_FUNCTION_MONITOR;
    } else {
        // confuse i2c communication failed result with not standby state.
        return false;
    }
}

void S11059::get(uint16_t* rgbir) {
    rgbir[0] = _rgbir[0];
    rgbir[1] = _rgbir[1];
    rgbir[2] = _rgbir[2];
    rgbir[3] = _rgbir[3];
}

uint16_t S11059::getRed() {
    return _rgbir[0];
}

uint16_t S11059::getGreen() {
    return _rgbir[1];
}

uint16_t S11059::getBlue() {
    return _rgbir[2];
}

uint16_t S11059::getIR() {
    return _rgbir[3];
}

uint8_t S11059::getControlBits() {
    uint8_t bits = 0x00;

    // Gain selection (1 bit)
    bits |= !!(_gain) << CONTROL_BIT_GAIN_SELECTION;

    // Integration mode (1 bit)
    bits |= !!(_mode) << CONTROL_BIT_INTEGRATION_MODE;

    // Integration time setting (2 bits)
    bits |= _tint & 0x03;

    return bits;
}

bool S11059::write(uint8_t value) {
    uint8_t values[1] = {value};
    return write(values, 1);
}

bool S11059::write(uint8_t* values, size_t size) {
    bool res = i2c.write(I2C_ADDR, values, size);
    sleep_us(100);
    if (res == PICO_ERROR_GENERIC) {
        return false;
    }
    return res;
}

bool S11059::read(uint8_t* values, int size) {
    bool ret = i2c.read(I2C_ADDR, values, size);
    sleep_us(100);
    return ret;
}

bool S11059::readRegister(uint8_t address, uint8_t* value) {
    return readRegisters(address, value, 1);
}

bool S11059::readRegisters(uint8_t address, uint8_t* values, uint8_t size) {
    if (!write(address)) {
        return false;
    }

    return read(values, size);
}

bool S11059::writeRegister(uint8_t address, uint8_t value) {
    return writeRegisters(address, &value, 1);
}

bool S11059::writeRegisters(uint8_t address, uint8_t* values, uint8_t size) {
    uint8_t tmp[size + 1];
    tmp[0] = address;
    for (int8_t i = 0; i < size; i++) {
        tmp[i + 1] = values[i];
    }
    return write(tmp, sizeof(tmp));
}

Color::Color(int id, int th0, int th1, int th2, int th3)
    : pca9546(0x70), id(id) {
    th[0] = th0;
    th[1] = th1;
    th[2] = th2;
    th[3] = th3;
}

void Color::init() {
    pca9546.select(id);
    s11059.setMode(S11059_MODE_FIXED);
    s11059.setGain(S11059_GAIN_HIGH);
    s11059.setTint(S11059_TINT2);
    printf("start\n");
    if (!s11059.reset()) {
        printf("reset failed\n");
    }
    if (!s11059.start()) {
        printf("start failed\n");
    }
}

void Color::read(uint16_t* rgbir) {
    s11059.delay();
    pca9546.select(id);
    s11059.update();
    s11059.delay();
    s11059.get(rgbir);
}

Colors Color::getColor() {
    uint16_t color[4];
    sleep_ms(100);
    read(color);

    if (color[1] > th[0]) {
        // printf("yellow\n");
        return YELLOW;
    } else if (color[0] < th[1] && color[2] < th[2]) {
        // printf("no ball found\n");
        return NONE;
    } else if ((color[2] - color[0]) > th[3]) {
        // printf("blue\n");
        return BLUE;
    } else {
        // printf("red\n");
        return RED;
    }
    return NONE;
}

int Color::RGBtoHSV(int RGB[]){
    int R=(int)(((float)RGB[0]-5000)/7000.0*256);
    int G=(int)(((float)RGB[1]-5000)/7000.0*256);
    int B=(int)(((float)RGB[2]-5000)/7000.0*256);
	int MAX = max((max(R,G)),B);
	int MIN = min((min(R,G)),B);
	double Value = MAX/256*100;
    double Hue;
    double Saturation;

	if(MAX==MIN){
		Hue = 0;
		Saturation = 0;
	}
	else{
		if(MAX == R) Hue = 60.0*(G-B)/(MAX-MIN) + 0;
		else if(MAX == G) Hue = 60.0*(B-R)/(MAX-MIN) + 120.0;
		else if(MAX == B) Hue = 60.0*(R-G)/(MAX-MIN) + 240.0;

		if(Hue > 360.0) Hue = Hue - 360.0;
		else if(Hue < 0) Hue = Hue + 360.0;
		Saturation = (MAX-MIN)/MAX*100.0;
	}

	return int(Hue);
}