RGB (256x192) input to SVGA (800x600) output


This project was built with "Raspberry Pi Pico SDK 1.5.1" and "Pico Visual Studio Code".

Details of RGB2XGA Project:

RGB inputs 

Pin 16, GP12 - Red Mid
Pin 17, GP13 - Red Hi
Pin 19, GP14 - Red Lo
Pin 20, GP15 - Green Mid
Pin 21, GP16 - Green Hi
Pin 22, GP17 - Green Lo
Pin 24, GP18 - Blue Mid
Pin 25, GP19 - Blue Hi

Pin 26, GP20 - H Sync
Pin 27, GP21 - V Sync 

SVGA (XGA) outputs

Pin 14, GP10 - H Sync
Pin 15, GP11 - V Sync

Pin 4, GP2 - Red Mid
Pin 5, GP3 - Red Hi
Pin 6, GP4 - Red Lo
Pin 7, GP5 - Green Mid
Pin 9, GP6 - Green Hi
Pin 10, GP7 - Green Lo
Pin 11, GP8 - Blue Mid
Pin 12, GP9 - Blue Hi

        GP25 - LED

Available

Pin 1, GP0 - UART0 Tx, Future option
Pin 2, GP1 - UART0 Rx, Future option

Pin 29, GP22 - unused
Pin 31, GP26 - unused
Pin 32, GP27 - unused 
Pin 34, GP28 - unused 