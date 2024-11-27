#include "encoder.h"
int Encoder::id = 0;
int Encoder::rotation[8] = {0};
uint Encoder::offset = 0;
bool Encoder::isInitialized = false;
// constructor
// rotary_encoder_A is the pinA for the A of the rotary encoder.
// The B of the rotary encoder has to be connected to the next GPIO.
Encoder::Encoder(int pinA)
    : pinA(pinA) {
    sm = id;
    id++;
}

void Encoder::init() {
    uint8_t pinB = pinA + 1;
    // pio 0 is used
    
    PIO pio;
    switch (sm) {
        case 0:
            pio=pio0;
            break;
        case 1:
            pio=pio1;
            break;
        default:
            break;
    }
    // state machine 0
    // configure the used pinAs as input with pull up
    pio_gpio_init(pio, pinA);
    gpio_set_pulls(pinA, true, false);
    pio_gpio_init(pio, pinB);
    gpio_set_pulls(pinB, true, false);
    // load the pio program into the pio memory
    if (1) {
        offset = pio_add_program(pio, &pio_rotary_encoder_program);
        isInitialized = true;
    }
    // make a sm config
    pio_sm_config c = pio_rotary_encoder_program_get_default_config(offset);
    // set the 'in' pinAs
    sm_config_set_in_pins(&c, pinA);
    // set shift to left: bits shifted by 'in' enter at the least
    // significant bit (LSB), no autopush
    sm_config_set_in_shift(&c, false, false, 0);
    // set the IRQ handler
    switch (sm) {
        case 0:
            irq_set_exclusive_handler(PIO0_IRQ_0, pio_irq_handler_0);
            irq_set_enabled(PIO0_IRQ_0, true);
            pio0_hw->inte0 = PIO_IRQ0_INTE_SM0_BITS | PIO_IRQ0_INTE_SM1_BITS;
            break;
        case 1:
            irq_set_exclusive_handler(PIO1_IRQ_0, pio_irq_handler_1);
            irq_set_enabled(PIO1_IRQ_0, true);
            pio1_hw->inte0 = PIO_IRQ0_INTE_SM0_BITS | PIO_IRQ0_INTE_SM1_BITS;
        default:
            break;
    }
    // enable the IRQ

    // init the sm.
    // Note: the program starts after the jump table -> initial_pc = 16
    pio_sm_init(pio, 0, 16, &c);
    // enable the sm
    pio_sm_set_enabled(pio, 0, true);
}

// set the current rotation to a specific value
void Encoder::set(int _rotation) {
    rotation[sm] = _rotation;
}

// get the current rotation
int Encoder::get(void) {
    return rotation[sm];
}

void Encoder::pio_irq_handler_0() {
    // test if irq 0 was raised
    if (pio0_hw->irq & 1) {
        rotation[0] = rotation[0] - 1;
    }
    // test if irq 1 was raised
    if (pio0_hw->irq & 2) {
        rotation[0] = rotation[0] + 1;
    }
    // clear both interrupts
    pio0_hw->irq = 3;
}

void Encoder::pio_irq_handler_1() {
    // test if irq 0 was raised
    if (pio1_hw->irq & 1) {
        rotation[1] = rotation[1] - 1;
    }
    // test if irq 1 was raised
    if (pio1_hw->irq & 2) {
        rotation[1] = rotation[1] + 1;
    }
    // clear both interrupts
    pio1_hw->irq = 3;
}