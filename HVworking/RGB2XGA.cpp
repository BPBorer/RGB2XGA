#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

#include "XGAvSync.pio.h"
#include "RGB_PIn.pio.h"
#include "XGAhSync.pio.h"
#include "XGAPixs.pio.h"

// Data will be copied from src to dst
const char src[] = "Hello, world! (from DMA)";
char dst[count_of(src)];

 #ifndef LED_DELAY_MS
 #define LED_DELAY_MS 250
 #endif


int main()
{   // Sys Clock set for 160 MHz.
    set_sys_clock_pll(1440000000, 3, 3);

    stdio_init_all();

// DMA channels - 0 sends/receives color data, 1 reconfigures and restarts 0
    int      rgb_chan_0 = 0;
    int      rgb_chan_1 = 1;
    PIO      rgb_pio    = pio0;
    uint     rgb_sm     = 0;
    uint32_t rgb_Rx_Cnt = (101 * 192);
    int      xga_chan_0 = 2;
    int      xga_chan_1 = 3;
    PIO      xga_pio    = pio1;
    uint     smvSync    = 0;
    uint     smhSync    = 1;
    uint     smXGAPixs  = 2;
    uint32_t xga_Tx_Cnt = (101 * 192);     // 101*4  = 404 bytes
    uint32_t *xga_lines[192*2];   
    uint32_t vga_data_array[101 * 192];
    uint     x;

   //pico_led_init();
   gpio_init(PICO_DEFAULT_LED_PIN);
   gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

   //stdio_init_all();

    for (x = 0; x < xga_Tx_Cnt; x++) {
        vga_data_array[x] = 0xcca97613;
    }

    xga_lines[0] = &vga_data_array[0];
    xga_lines[192*2-1] = &vga_data_array[0];    
    for (x = 1; x < (192-2); x+=2) {
        xga_lines[x] = &vga_data_array[(x * 101)];
        xga_lines[x+1] = &vga_data_array[x * 101];
    }

    sleep_ms(LED_DELAY_MS*9);
    
    uint offseth = pio_add_program(xga_pio, &XGAhSync_program);
    printf("XGAhSync pin %d with offset %d\n", 10, offseth);
    XGAhSync_program_init(xga_pio, smhSync, offseth, 10);
    //pio_sm_set_enabled(xga_pio, smhSync, true);
    xga_pio->txf[smhSync] = (440);

    uint offsetv = pio_add_program(xga_pio, &XGAvSync_program);
    printf("XGAvSync pin %d with offset %d\n", 11, offsetv);
    XGAvSync_program_init(xga_pio, smvSync, offsetv, 11);
    pio_sm_set_enabled(xga_pio, smvSync, true);
    xga_pio->txf[smvSync] = (601);

    uint offsetp = pio_add_program(xga_pio, &XGAPixs_program);
    printf("XGAPixs pin %d with offset %d\n", 2, offsetp);
    XGAPixs_program_init(xga_pio, smXGAPixs, offsetp, 2);
    pio_sm_set_enabled(xga_pio, smXGAPixs, true);
    xga_pio->txf[smXGAPixs] = (404);
    // xga_pio->txf[smXGAPixs] = (0xcca97613);
    // xga_pio->txf[smXGAPixs] = (0xcca97613);
    // xga_pio->txf[smXGAPixs] = (0xcca97613);

    uint offsetr = pio_add_program(rgb_pio, &RGB_PIn_program);
    printf("RGB_PIn %d with offset %d\n", 12, offsetr);
    RGB_PIn_program_init(rgb_pio, rgb_sm, offsetr, 12);
    pio_sm_set_enabled(rgb_pio, rgb_sm, true);
    xga_pio->txf[rgb_sm] = (404);

    // 32 bit transfers. Both read and write address increment after each
    // transfer (each pointing to a location in src or dst respectively).
    // DREQ is selected, so the DMA transfers are controlled.
    
    // Channel Three (reconfigures the first XGA channel)
    dma_channel_config c1 = dma_channel_get_default_config(xga_chan_1);   // default configs
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);              // 32-bit txfers
    channel_config_set_read_increment(&c1, false);                        // no read incrementing
    channel_config_set_write_increment(&c1, false);                       // no write incrementing
    channel_config_set_chain_to(&c1, xga_chan_0);                         // chain to other channel

    dma_channel_configure(
        xga_chan_1,                         // Channel to be configured
        &c1,                                // The configuration we just created
        &vga_data_array,                    // Write address (POINTER TO AN ADDRESS)
        &dma_hw->ch[xga_chan_0].read_addr,  // Read address (channel 0 read address)
        1,                                  // Number of transfers, in this case each is 4 byte
        true                                // start immediately.
    );

    // Channel One (reconfigures the first RGB channel)
    dma_channel_config c3 = dma_channel_get_default_config(rgb_chan_1);   // default configs
    channel_config_set_transfer_data_size(&c3, DMA_SIZE_32);              // 32-bit txfers
    channel_config_set_read_increment(&c3, false);                        // no read incrementing
    channel_config_set_write_increment(&c3, false);                       // no write incrementing
    channel_config_set_chain_to(&c3, rgb_chan_0);                         // chain to other channel

    dma_channel_configure(
        rgb_chan_1,                         // Channel to be configured
        &c3,                                // The configuration we just created
        &dma_hw->ch[rgb_chan_0].write_addr, // Write address (channel 0 read address)
        &vga_data_array,                    // Read address (POINTER TO AN ADDRESS)
        1,                                  // Number of transfers, in this case each is 4 byte
        true                                // start immediately.
    );
    // Channel Zero (gets color data from PIO RGB machine)
    dma_channel_config c2 = dma_channel_get_default_config(rgb_chan_0);  // default configs
    channel_config_set_transfer_data_size(&c2, DMA_SIZE_32);       // 32-bit txfers
    channel_config_set_read_increment(&c2, false);                 // no read incrementing
    channel_config_set_write_increment(&c2, true);                 // yes write incrementing
    channel_config_set_dreq(&c2, DREQ_PIO0_RX0) ;                  // DREQ_PIO0_RX0 pacing (FIFO)
    channel_config_set_chain_to(&c2, rgb_chan_1);                  // chain to other channel

    dma_channel_configure(
        rgb_chan_0,                 // Channel to be configured
        &c2,                        // The configuration we just created
        &vga_data_array,            // The initial write address (pixel color array)
        &rgb_pio->rxf[rgb_sm],      // read address (RGB PIO TX FIFO)
        rgb_Rx_Cnt,                 // Number of transfers; in this case each is 4 bytes.
        true                        // start immediately.
    );

    // Channel Two (Sends color data to PIO XGA machine)
    dma_channel_config c0 = dma_channel_get_default_config(xga_chan_0);  // default configs
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_32);       // 32-bit txfers
    channel_config_set_read_increment(&c0, true);                  // yes read incrementing
    channel_config_set_write_increment(&c0, false);                // no write incrementing
    channel_config_set_dreq(&c0, DREQ_PIO1_TX2) ;                  // DREQ_PIO1_TX2 pacing (FIFO)
    channel_config_set_chain_to(&c0, xga_chan_1);                  // chain to other channel

    dma_channel_configure(
        xga_chan_0,                 // Channel to be configured
        &c0,                        // The configuration we just created
        &xga_pio->txf[smXGAPixs],   // The write address (RGB PIO TX FIFO)
        &vga_data_array,            // The initial read address (pixel color array)
        xga_Tx_Cnt,                 // Number of transfers; in this case each is 4 bytes.
        true                        // start immediately.
    );



    while (true) {
    printf("Hello, world!\n");
    // sleep_ms(1000);
    // pico_set_led(true);
    // if (pis_sm2_tx_fifo_not_full) xga_pio->txf[smXGAPixs] = (0xcca97613);
    // if (pis_sm2_tx_fifo_not_full) xga_pio->txf[smXGAPixs] = (0xcca97613);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(LED_DELAY_MS*2);
    // pico_set_led(false);
    // if (pis_sm2_tx_fifo_not_full) xga_pio->txf[smXGAPixs] = (0xcca97613);
    // if (pis_sm2_tx_fifo_not_full) xga_pio->txf[smXGAPixs] = (0xcca97613);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    sleep_ms(LED_DELAY_MS*2);
    }
}
