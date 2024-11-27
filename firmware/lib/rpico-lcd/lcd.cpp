#include "lcd.h"

const int Custom_Char5x8[2][64] = {
    {// 0x00, 通常は使わない
     0b00100,
     0b00001,
     0b10010,
     0b00100,
     0b00100,
     0b10010,
     0b00001,
     0b00100,

     // 0x01
     0b00100,
     0b10000,
     0b01001,
     0b00100,
     0b00100,
     0b01001,
     0b10000,
     0b00100,

     // 0x02
     0b00000,
     0b00110,
     0b01001,
     0b10000,
     0b10000,
     0b10000,
     0b01111,
     0b00000,

     // 0x03
     0b00000,
     0b00000,
     0b11100,
     0b00010,
     0b00010,
     0b00010,
     0b11100,
     0b00000,

     // 0x04
     0b00001,
     0b00011,
     0b00111,
     0b01111,
     0b00001,
     0b00001,
     0b00001,
     0b00000,

     // 0x05
     0b10000,
     0b11000,
     0b11100,
     0b11110,
     0b00000,
     0b00000,
     0b01000,
     0b10000,

     // 0x06
     0b00000,
     0b00111,
     0b01000,
     0b10011,
     0b00100,
     0b00000,
     0b00011,
     0b00001,

     // 0x07
     0b00000,
     0b11100,
     0b00010,
     0b11001,
     0b00100,
     0b00000,
     0b11000,
     0b10000},

    {// 0x00, 通常は使わない
     0b00100,
     0b00001,
     0b10010,
     0b00100,
     0b00100,
     0b10010,
     0b00001,
     0b00100,

     // 0x01
     0b00100,
     0b10000,
     0b01001,
     0b00100,
     0b00100,
     0b01001,
     0b10000,
     0b00100,

     // 0x02
     0b00000,
     0b00110,
     0b01001,
     0b10000,
     0b10000,
     0b10000,
     0b01111,
     0b00000,

     // 0x03
     0b00000,
     0b00000,
     0b11100,
     0b00010,
     0b00010,
     0b00010,
     0b11100,
     0b00000,

     // 0x04
     0b00001,
     0b00011,
     0b00111,
     0b01111,
     0b00001,
     0b00001,
     0b00001,
     0b00000,

     // 0x05
     0b10000,
     0b11000,
     0b11100,
     0b11110,
     0b00000,
     0b00000,
     0b01000,
     0b10000,

     // 0x06
     0b11100,
     0b10100,
     0b11100,
     0b00111,
     0b01000,
     0b01000,
     0b01000,
     0b00111,

     // 0x07
     0b00100,
     0b00001,
     0b10010,
     0b00100,
     0b00100,
     0b10010,
     0b00001,
     0b00100}};

Lcd::Lcd() {}

void Lcd::write(uint8_t command, uint8_t t_data) {
    uint8_t cmd[2];
    cmd[0] = command;
    cmd[1] = t_data;
    i2c.write(LCD_ADRS, cmd, 2);
    sleep_ms(1);
}

void Lcd::print(const char* c, int line, int address) {
    this->write(CMD, 0x02);
    if (line == 2) {
        this->write(CMD, 0x40 + 0x80);
    } else if (line == 0) {
        this->write(CMD, 0x80 + address);
    }
    unsigned char i, str;
    for (i = 0; i < strlen(c); i++) {
        str = c[i];
        this->write(DATA, str);
    }
}

void Lcd::clr() {
    this->write(CMD, 0x01);
    sleep_ms(1);
    this->write(CMD, 0x02);
    sleep_ms(1);
}

void Lcd::init(int num, int contrast) {
    sleep_ms(1);
    if (num == 1) {
        this->write(CMD, 0b00111100);
    } else {
        this->write(CMD, 0b00111000);  // Function Set
    }
    if (num == 1) {
        this->write(CMD, 0b00111101);
    } else {
        this->write(CMD, 0b00111001);  // Function Set
    }
    sleep_ms(1);
    if (num == 1) {
        this->write(CMD, 0b00111101);
    } else {
        this->write(CMD, 0b00111001);  // Function Set
    }
    sleep_ms(1);
    this->write(CMD, 0x14);  // Internal OSC Freq
    sleep_ms(1);
    this->write(CMD, 0b01110000 | (contrast & 0b001111));  // Contrast Set
    sleep_ms(1);
    this->write(CMD, 0b01010100 | ((contrast >> 4)&0b11));  // P/I/C Control
    sleep_ms(1);
    this->write(CMD, 0x6C);  // Follower Control
    sleep_ms(1);
    if (num == 1) {
        this->write(CMD, 0b00111100);
    } else {
        this->write(CMD, 0b00111000);  // Function Set
    }
    sleep_ms(1);
    this->write(CMD, 0x01);  // Clear Display
    sleep_ms(1);
    this->write(CMD, 0x0C);  // On/Off Control
    sleep_ms(1);
    this->write(CMD, 0x40);  // Set CGRAM
    sleep_ms(1);
    int i;
    for (i = 0; i < 64; i++) {
        this->write(DATA, Custom_Char5x8[0][i]);  // Set CGRAM
        // sleep_ms(1);
    }
    sleep_ms(1);
}

void Lcd::setCGRAM(int id) {
    this->write(CMD, 0x40);  // Set CGRAM
    sleep_ms(1);
    int i;
    for (i = 0; i < 64; i++) {
        this->write(DATA, Custom_Char5x8[id][i]);  // Set CGRAM
        sleep_ms(1);
    }
}

void Lcd::printf(int line, const char* format, ...) {
    char buf[32];
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    this->print(buf, line);
}