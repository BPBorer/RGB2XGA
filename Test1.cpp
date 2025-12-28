
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/pll.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

// #include "blink.pio.h"
#include "XGAPixs.pio.h"
#include "XGAhSync.pio.h"
#include "XGAvSync.pio.h"
#include "RGBFP.pio.h"
#include "RGBline.pio.h"
#include "RGBPixs.pio.h"

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 2
#endif

#define pixels   400
#define RGBlines 192
#define XGAlines 600
#define rgb_Rx_Cnt (pixels / 4) * 192 // 19200   //
#define xga_Tx_Cnt (pixels / 4) * 300 // 30000   //
#define Ins_PULL    0x80A0
#define Ins_MOV2ISR 0xA0C7
#define Ins_MOV2Y   0xA047

io_rw_32 *xga_lines[XGAlines]; // [RGBlines*2];
int xga_chan_0;
int xga_chan_1;
int n, x;
uint32_t *XGAaddr_pointer;
uint32_t vga_data_array[xga_Tx_Cnt + 100];

/* void DMA_IRQ_handler()
{
    extern io_rw_32 *xga_lines[];
    extern int xga_chan_0;
    extern long unsigned int *lineptr;
    extern int n, x;
//    extern uint32_t *XGAaddr_pointer;
    extern uint32_t vga_data_array[];

    XGAaddr_pointer = &vga_data_array[n * pixels / 4];

    irq_clear(PIO0_IRQ_0);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    x++;
    if (x >= 2)
    {
        n++;
        x = 0;
    }
    if (n >= (XGAlines / 2) - 1)
        n = 0;
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}
*/
int main()
{   extern io_rw_32 *xga_lines[];
    extern int xga_chan_0;
    extern int n, x;
    extern uint32_t *XGAaddr_pointer;
    extern uint32_t vga_data_array[];


    PIO RGB_pio = pio1;
    PIO xga_pio = pio0;
    uint smXGAPixs = 1;
    uint smvSync   = 2;
    uint smhSync   = 3;

    uint smRGBFP   = 0;
    uint smRGBline = 1;
    uint smRGBPixs = 2;

    uint y;
    uint instrs[2] = {0xA047, 0x80A0};
    uint32_t *RGBaddr_pointer = &vga_data_array[0];

    uint32_t Data[8];
    uint32_t Col, DataPtr;
    char     MessPtr;
    int      i,j; 
    static char Message[29*6] = {
        0x84, 0x84, 0x84, 0xFC, 
        0x54, 0x38, 0x00, 0x78,  // D
        0x00, 0x24, 0x54, 0x54,  // e
        0x54, 0x54, 0x54, 0x24,  
        0x00, 0x00, 0x00, 0x48,  // s
        0x00, 0x00, 0x00, 0x5C,  // i
        0x25, 0x25, 0x25, 0x18, 
        0x10, 0x3C, 0x00, 0x1E,  // g
        0x00, 0x1C, 0x20, 0x20,  // n
        0X54, 0X54, 0X54, 0X38, 
        0x14, 0x08, 0X00, 0X24,  // e
        0x00, 0xFC, 0x14, 0x14,  // d
        0x00, 0x00, 0x00, 0x00, 
        0x14, 0xFC, 0x00, 0x00,  // space
        0x00, 0x08, 0x14, 0x14,  // b
        0x05, 0x05,0x05, 0x19,  
        0x00, 0x00, 0x00, 0x1E,  // y
        0x00, 0x00, 0x00, 0x00,  // space
        0xA4, 0xA4, 0xA4, 0xFC, 
        0x10, 0x3C, 0x00, 0x58,  // B
        0x00, 0x10, 0x20, 0x20,  // r
        0x04, 0x04, 0x04, 0x38, 
        0x24, 0x18, 0x00, 0x38,  // u
        0x00, 0x24, 0x24, 0x24,  // c
        0X54, 0X54, 0X54, 0X38, 
        0x00, 0x00, 0X00, 0X24,  // e
        0x00, 0x00, 0x00, 0x00,  // space
        0xA0, 0xA0, 0xA0, 0xFC, 
        0x00, 0x00, 0x00, 0x40,  // P
        0x00, 0x00, 0x0C, 0x0C,  // .
        0x00, 0x00, 0x00, 0x00, 
        0xA4, 0xFC, 0x00, 0x00,  // space
        0x00, 0x58, 0xA4, 0xA4,  // B
        0x24, 0x24, 0x24, 0x18, 
        0x10, 0x3C, 0x00, 0x18, // o
        0x00, 0x10, 0x20, 0x20,  // r
        0X54, 0X54, 0X54, 0X38, 
        0x10, 0x3C, 0X00, 0X24,  // e
        0x00, 0x10, 0x20, 0x20,  // r
        0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00,  // space
        0x00, 0x00, 0x00, 0x00   // space
    };

// Sys Clock set for 160 MHz.
    set_sys_clock_pll(1440000000, 3, 3);

    stdio_init_all();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

   for (x = 0; x < rgb_Rx_Cnt; x++)
    {
        vga_data_array[x] = 0x00010100 * x;
    }
    for (x = rgb_Rx_Cnt; x < xga_Tx_Cnt ; x++)
    {
        vga_data_array[x] = 0;
    }
    vga_data_array[0] = 0xFFFF0000;
    vga_data_array[4] = 0;
    vga_data_array[104] = 0;
    vga_data_array[204] = 0;
    vga_data_array[304] = 0;
    vga_data_array[404] = 0;
    vga_data_array[504] = 0;
    vga_data_array[604] = 0;
    vga_data_array[704] = 0;
    vga_data_array[10] = 0;
    vga_data_array[50] = 0;
    vga_data_array[99] = 0;
    vga_data_array[199] = 0;
    vga_data_array[299] = 0;
    vga_data_array[399] = 0;
    vga_data_array[499] = 0;
    vga_data_array[29900] = 0;
    vga_data_array[30000] = 0;

    vga_data_array[rgb_Rx_Cnt + 204] = 0xFFFFFFFF;

    XGAaddr_pointer = &vga_data_array[0];

    DataPtr = 24106;
    for (MessPtr = 0; MessPtr < 28*6; MessPtr = MessPtr+4){
        for (i=0; i<8; i++) {
            Data[i] = 0;
        }
         for (j=0; j < 4; j++) {
            Col = Message[MessPtr+j];
            for (i=0; i<8; i++) {
                Data[i] = Data[i] << 8;
                if (Col & (1<<i)) Data[i] = Data[i] + 0xFF;
            }
        }
        for (i = 0; i < 8; i++) {
            vga_data_array[DataPtr+(i * 100)] = Data[7-i];
        }
        DataPtr = DataPtr + 1;
   } 

    n = 0;
    x = 0;

    sleep_ms(LED_DELAY_MS * 1000);
 
    gpio_set_drive_strength(2, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(3, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(4, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(5, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(6, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(7, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(8, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(9, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(10, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(11, GPIO_DRIVE_STRENGTH_12MA);

    uint offseth = pio_add_program(xga_pio, &XGAhSync_program);
    XGAhSync_program_init(xga_pio, smhSync, offseth, 10);
    xga_pio->txf[smhSync] = (440);         //; Active+FP (400+40) =  440
    xga_pio->sm[smhSync].instr = Ins_PULL; // pull  block
    pio_sm_set_enabled(xga_pio, smhSync, true);

    uint offsetv = pio_add_program(xga_pio, &XGAvSync_program);
    XGAvSync_program_init(xga_pio, smvSync, offsetv, 11);
    xga_pio->txf[smvSync] = (XGAlines - 1);
    xga_pio->sm[smvSync].instr = Ins_PULL; // pull  block
    pio_sm_set_enabled(xga_pio, smvSync, true);

    uint offsetp = pio_add_program(xga_pio, &XGAPixs_program);
    XGAPixs_program_init(xga_pio, smXGAPixs, offsetp, 2);
    xga_pio->txf[smXGAPixs] = (pixels - 1);
    xga_pio->sm[smXGAPixs].instr = Ins_PULL;    // pull  block
    xga_pio->sm[smXGAPixs].instr = Ins_MOV2ISR; // mov   isr, osr
    pio_sm_set_enabled(xga_pio, smXGAPixs, true);

    uint offsetR = pio_add_program(RGB_pio, &RGBFP_program);
    RGBFP_program_init(RGB_pio, smRGBFP, offsetR, 26);
    RGB_pio->txf[smRGBFP] = 39;
    RGB_pio->sm[smRGBFP].instr = Ins_PULL; // pull  block

    uint offsetG = pio_add_program(RGB_pio, &RGBline_program);
    RGBline_program_init(RGB_pio, smRGBline, offsetG, 22);
    RGB_pio->txf[smRGBline] = (RGBlines - 1);
    RGB_pio->sm[smRGBline].instr = Ins_PULL; // pull  block
    RGB_pio->sm[smRGBline].instr = Ins_MOV2ISR;
    RGB_pio->txf[smRGBline] = 1127;          // ((80*22)-1);
    RGB_pio->sm[smRGBline].instr = Ins_PULL; // pull  block

    uint offsetB = pio_add_program(RGB_pio, &RGBPixs_program);
    RGBPixs_program_init(RGB_pio, smRGBPixs, offsetB, 12);
    RGB_pio->txf[smRGBPixs] = (pixels / 4) - 1;
    RGB_pio->sm[smRGBPixs].instr = Ins_PULL; // pull  block

    pio_sm_set_enabled(RGB_pio, smRGBPixs, true);
    pio_sm_set_enabled(RGB_pio, smRGBline, true);
    pio_sm_set_enabled(RGB_pio, smRGBFP, true);

    // 32 bit transfers. Both read and write address increment after each
    // transfer (each pointing to a location in src or dst respectively).
    // DREQ is selected, so the DMA transfers are controlled.

    // DMA channels - 0 sends color data,
    //              - 1 reconfigures and restarts 0
    xga_chan_0 = dma_claim_unused_channel(true);
    xga_chan_1 = dma_claim_unused_channel(true);

    // Channel Two (Sends color data to PIO XGA machine)
    dma_channel_config c2 = dma_channel_get_default_config(xga_chan_0); // default configs
    channel_config_set_transfer_data_size(&c2, DMA_SIZE_32);            // 32-bit txfers
    channel_config_set_read_increment(&c2, true);                       // yes read incrementing
    channel_config_set_write_increment(&c2, false);                     // no write incrementing
    channel_config_set_dreq(&c2, DREQ_PIO0_TX1);                        // DREQ_PIO0_TX1 pacing (FIFO)
    channel_config_set_chain_to(&c2, xga_chan_1);                       // chain to other channel

    dma_channel_configure(
        xga_chan_0,               // Channel to be configured
        &c2,                      // The configuration we just created
        &xga_pio->txf[smXGAPixs], // The write address (RGB PIO TX FIFO)
        &vga_data_array,          // The initial read address (pixel color array)
        (pixels / 4),             // Number of transfers; in this case each is 4 bytes.
        false                     // do not start immediately.
    );

    // Channel Three (reconfigures the first XGA channel)
    dma_channel_config c3 = dma_channel_get_default_config(xga_chan_1); // default configs
    channel_config_set_transfer_data_size(&c3, DMA_SIZE_32);            // 32-bit txfers
    channel_config_set_read_increment(&c3, false);                      // read incrementing
    channel_config_set_write_increment(&c3, false);                     // no write incrementing
    channel_config_set_chain_to(&c3, xga_chan_0);                       // chain to other channel

    dma_channel_configure(
        xga_chan_1,                        // Channel to be configured
        &c3,                               // The configuration we just created
        &dma_hw->ch[xga_chan_0].read_addr, // Write address (channel 0 read address)
        &XGAaddr_pointer,                  // read address (POINTER TO AN ADDRESS)
        1,                                 // Number of transfers,one per frame
        false                              // do not start immediately.
    );

    // dma_start_channel_mask(1u << xga_chan_0 | 1u << xga_chan_1 );
    dma_start_channel_mask(1u << xga_chan_0);

    int rgb_chan_0 = dma_claim_unused_channel(true);
    int rgb_chan_1 = dma_claim_unused_channel(true);

    // Channel Zero (Receives color data from PIO RGB machine)
    dma_channel_config c0 = dma_channel_get_default_config(rgb_chan_0); // default configs
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_32);            // 32-bit txfers
    channel_config_set_read_increment(&c0, false);                      // no read incrementing
    channel_config_set_write_increment(&c0, true);                      // yes write incrementing
    channel_config_set_dreq(&c0, DREQ_PIO1_RX2);                        // DREQ_PIO1_RX2 pacing (FIFO)
    channel_config_set_chain_to(&c0, rgb_chan_1);                       // chain to other channel

    dma_channel_configure(
        rgb_chan_0,               // Channel to be configured
        &c0,                      // The configuration we just created
        &vga_data_array,          // The write address (RGB PIO TX FIFO)
        &RGB_pio->rxf[smRGBPixs], // The initial read address (pixel color array)
        rgb_Rx_Cnt,               // Number of transfers; in this case each is 4 bytes.
        false                     // do not start immediately.
    );

    // Channel One (reconfigures the first RGB channel)
    dma_channel_config c1 = dma_channel_get_default_config(rgb_chan_1); // default configs
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);            // 32-bit txfers
    channel_config_set_read_increment(&c1, false);                      // read incrementing
    channel_config_set_write_increment(&c1, false);                     // no write incrementing
    channel_config_set_chain_to(&c1, rgb_chan_0);                       // chain to other channel

    dma_channel_configure(
        rgb_chan_1,                         // Channel to be configured
        &c1,                                // The configuration we just created
        &dma_hw->ch[rgb_chan_0].write_addr, // Write address (channel 1 write address)
        &RGBaddr_pointer,                   // write address (POINTER TO AN ADDRESS)
        1,                                  // Number of transfers,one per frame
        false                               // do not start immediately.
    );
    // pio0_hw->irq_inte[0];
    // dma_channel_set_irq0_enabled(xga_chan_1,true);
    // irq_set_exclusive_handler(PIO0_IRQ_0, DMA_IRQ_handler);
    // irq_set_enabled(PIO0_IRQ_0, true);

    // dma_start_channel_mask(1u << rgb_chan_0 | 1u << rgb_chan_1 );
    dma_start_channel_mask(1u << rgb_chan_0);

    // DMA_IRQ_handler();

    while (true)
    {
        /* 
            //    printf("CH0_COUNT = %d SM0= %d SM1= %d SM2= %d\n ", dma_channel_hw_addr(rgb_chan_0)->transfer_count, RGB_pio->sm[smRGBFP].addr, RGB_pio->sm[smRGBline].addr, RGB_pio->sm[smRGBPixs].addr);
                printf("CH0_COUNT = %d SM0= %d SM1= %d SM2= %d\n ", dma_channel_hw_addr(xga_chan_0)->transfer_count, xga_pio->sm[smvSync].addr, xga_pio->sm[smhSync].addr, xga_pio->sm[smXGAPixs].addr);
                printf("XGA_Pixs= %d\n ", xga_pio->sm[smXGAPixs].addr);
                printf("1SM_ADDR0 = %d\n", RGB_pio->sm[smRGBFP].addr);
        */
        if (gpio_get(10))
        {   dma_hw->ch[xga_chan_0].al3_read_addr_trig =  (long unsigned int) &vga_data_array[n * pixels / 4]; 

            gpio_put(PICO_DEFAULT_LED_PIN, 1);

            x++;
            if (x >= 2)
            {
                n++;
                x = 0;
            }
            if (n >= (XGAlines / 2) + 2)  n = 0;

            while (gpio_get(10)) ;
        }
        if  (!gpio_get(11))
        {   dma_hw->ch[xga_chan_0].al3_read_addr_trig =  (long unsigned int) &vga_data_array[0]; 
            x = 1;
            n = 290;
            while (!gpio_get(11)) ;
        }
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
    }
}
