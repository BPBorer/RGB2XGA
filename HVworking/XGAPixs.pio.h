// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ------- //
// XGAPixs //
// ------- //

#define XGAPixs_wrap_target 2
#define XGAPixs_wrap 6

static const uint16_t XGAPixs_program_instructions[] = {
    0x80a0, //  0: pull   block                      
    0xa047, //  1: mov    y, osr                     
            //     .wrap_target
    0x20c2, //  2: wait   1 irq, 2                   
    0xa022, //  3: mov    x, y                       
    0x20c1, //  4: wait   1 irq, 1                   
    0x6608, //  5: out    pins, 8                [6] 
    0x0045, //  6: jmp    x--, 5                     
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program XGAPixs_program = {
    .instructions = XGAPixs_program_instructions,
    .length = 7,
    .origin = -1,
};

static inline pio_sm_config XGAPixs_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + XGAPixs_wrap_target, offset + XGAPixs_wrap);
    return c;
}

static inline void XGAPixs_program_init(PIO xga_pio, uint smXGAPixs, uint offsetp, uint pin) {
    // Set this pin's GPIO function (connect PIO to the pad)
    pio_gpio_init(xga_pio, pin);
    pio_gpio_init(xga_pio, (pin + 1));
    pio_gpio_init(xga_pio, (pin + 2));
    pio_gpio_init(xga_pio, (pin + 3));
    pio_gpio_init(xga_pio, (pin + 4));
    pio_gpio_init(xga_pio, (pin + 5));
    pio_gpio_init(xga_pio, (pin + 6));
    pio_gpio_init(xga_pio, (pin + 7));
    pio_sm_set_consecutive_pindirs(xga_pio, smXGAPixs, pin, 8, true);
    pio_sm_config cp = XGAPixs_program_get_default_config(offsetp);
    sm_config_set_out_pins(&cp, pin, 8);
    sm_config_set_out_shift(&cp, true, true, 32);
    sm_config_set_fifo_join(&cp, PIO_FIFO_JOIN_TX);
    pio_sm_init(xga_pio, smXGAPixs, offsetp, &cp);
    pio_sm_set_enabled(xga_pio, smXGAPixs, true);
}

#endif
