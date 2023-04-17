#ifndef _LCD_CONF_USER_H_
#define _LCD_CONF_USER_H_

/* clang-format off */

/* Select screen Type, Optional:

  mipi dbi interface
    LCD_DBI_ILI9488

  mipi dpi (RGB) interface
    LCD_DPI_ILI9488
    LCD_DPI_GC9503V
    LCD_DPI_STANDARD

  mipi dsi vidio interface
    LCD_DSI_VIDIO_ILI9881C

  spi interface
    LCD_SPI_ILI9488
    LCD_SPI_ILI9341
    LCD_SPI_ST7796
    LCD_SPI_ST7789V
*/
#define LCD_SPI_ST7796

#if defined LCD_SPI_ST7796

    /* Selecting interface type, more configuration of peripherals comes later
        1: SPI peripheral, supported functions: spi-4wire,
    */
    #define LCD_SPI_INTERFACE_TYPE 1

    /* Selecting pixel format
        1: rgb565
    */
    #define ST7796_SPI_PIXEL_FORMAT 1

    /* enable the lcd reset function
        0: Does not care about lcd hard reset
        1: use gpio to reset the lcd
    */
    // #define LCD_RESET_EN 1

    /* LCD width and height */
    #define ST7796_SPI_W 320
    #define ST7796_SPI_H 320

    /* The offset of the area can be displayed */
    #define ST7796_SPI_OFFSET_X 0
    #define ST7796_SPI_OFFSET_Y 0

    /* Color reversal, Some screens are required
        0: disable
        1: enable
    */
   #define ST7796_SPI_COLOR_REVERSAL 1

#endif

/********** SPI hard 4-wire configuration ***********/
#if (defined(LCD_SPI_INTERFACE_TYPE) && (LCD_SPI_INTERFACE_TYPE == 1))

    /* spi idx */
    #define LCD_SPI_HARD_4_NAME "spi0"

    /* dma used by spi */
    #define LCD_SPI_HARD_4_DMA_NAME "dma0_ch3"

    /* The maximum pixel cnt to be transferred affects the number of LLI memory pools */
    #define LCD_SPI_HARD_4_PIXEL_CNT_MAX (320 * 320)

    /* spi pin, hardware controlled */
    #define LCD_SPI_HARD_4_PIN_CLK   GPIO_PIN_9
    #define LCD_SPI_HARD_4_PIN_DAT   GPIO_PIN_19
    /* cs/dc pin, software controlled */
    #define LCD_SPI_HARD_4_PIN_CS    GPIO_PIN_17
    #define LCD_SPI_HARD_4_PIN_DC    GPIO_PIN_18

#endif

/* clang-format on */

#endif
