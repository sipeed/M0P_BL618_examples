#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "bflb_adc.h"
#include "bflb_clock.h"
#include "bflb_flash.h"
#include "bflb_gpio.h"
#include "bflb_mtimer.h"
#include "bflb_pwm_v2.h"

#include "board.h"

#include "usbh_core.h"
#include "shell.h"

#include "lcd.h"

#define DBG_TAG "MAIN"
#include "log.h"

static TimerHandle_t display_timer_handle;
static TaskHandle_t button_task_handle;

extern void shell_init_with_task(struct bflb_device_s *shell);
extern void cdc_acm_init(void);
extern void lcd_display_loop(void);
extern void lcd_setDir(uint8_t dir);

static uint8_t lcd_dir;

static void vDisplayTimer(TimerHandle_t xTimer)
{
    static uint8_t dir = 0;
    if (lcd_dir != dir) {
        dir = lcd_dir;
        lcd_setDir(dir);
        printf("Changeed dir to %u\r\n", dir);
    }

    lcd_display_loop();
}

static void button_task(void *pvParameters)
{
    struct bflb_device_s *gpio;
    gpio = bflb_device_get_by_name("gpio");
    /* ADC_CH3 */
    bflb_gpio_init(gpio, GPIO_PIN_3, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
    // bflb_gpio_init(gpio, GPIO_PIN_3, GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_init(gpio, GPIO_PIN_2, GPIO_FUNC_PWM0 | GPIO_ALTERNATE | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);

    uint8_t last_key, distribute_key;
    uint8_t dir;
    uint8_t brightness;
    uint16_t brightness_map[] = { 0, 2, 4, 8, 16, 32, 63, 126, 251, 501, 1000 };

    struct bflb_device_s *pwm;
    pwm = bflb_device_get_by_name("pwm_v2_0");

    /* period = .XCLK / .clk_div / .period = 40MHz / 40 / 1000 = 1KHz */
    bflb_pwm_v2_init(pwm,
                     &(struct bflb_pwm_v2_config_s){
                         .clk_source = BFLB_SYSTEM_XCLK,
                         .clk_div = 40,
                         .period = 1000,
                     });
    bflb_pwm_v2_channel_set_threshold(pwm, PWM_CH2, 100, 500); /* duty = (500-100)/1000 = 40% */
    bflb_pwm_v2_channel_positive_start(pwm, PWM_CH2);
    bflb_pwm_v2_start(pwm);

    struct bflb_device_s *adc;
    adc = bflb_device_get_by_name("adc");

    /* adc clock = XCLK / 2 / 32 / 256(16B) = 2.442K */
    bflb_adc_init(
        adc,
        &(struct bflb_adc_config_s){
            .clk_div = ADC_CLK_DIV_32,
            .scan_conv_mode = true,
            .continuous_conv_mode = true,
            .differential_mode = false,
            .resolution = ADC_RESOLUTION_16B,
            .vref = ADC_VREF_3P2V,
        });
    bflb_adc_channel_config(
        adc,
        (struct bflb_adc_channel_s[]){
            {
                .pos_chan = ADC_CHANNEL_3,
                .neg_chan = ADC_CHANNEL_GND,
            },
        },
        1);
    bflb_adc_start_conversion(adc);

    typedef struct {
        uint8_t lcd_dir;
        uint8_t lcd_bright;
    } maixplayU4_config_t;
#define FLASH_BLOCK_SIZE        4096
#define CONFIG_IN_FLASH_ADDRESS (0x50000 - FLASH_BLOCK_SIZE)
    maixplayU4_config_t maixplayU4_config;
    bflb_flash_read(CONFIG_IN_FLASH_ADDRESS, (uint8_t *)&maixplayU4_config, sizeof(maixplayU4_config_t));
    if (maixplayU4_config.lcd_bright > 10 ||
        maixplayU4_config.lcd_dir > 3) {
        maixplayU4_config.lcd_bright = 6;
        maixplayU4_config.lcd_dir = 0;
        bflb_flash_erase(CONFIG_IN_FLASH_ADDRESS, FLASH_BLOCK_SIZE);
        bflb_flash_write(CONFIG_IN_FLASH_ADDRESS, (uint8_t *)&maixplayU4_config, sizeof(maixplayU4_config_t));
    }

    dir = lcd_dir = maixplayU4_config.lcd_dir;
    brightness = maixplayU4_config.lcd_bright;

    bflb_pwm_v2_channel_set_threshold(pwm, PWM_CH2, 100, 100 + brightness_map[brightness]);
    distribute_key = last_key = 0;
    while (1) {
        distribute_key = 0;
        do {
            vTaskDelay(pdMS_TO_TICKS(10));
            uint32_t raw_val = bflb_adc_read_raw(adc);
            uint8_t tmp_val = ((raw_val & 0xffff) >> 2) >> 7;
            // printf("tmp_val: %u\r\n", tmp_val);
            uint8_t key = (tmp_val < 60) + (tmp_val < 25);
            // printf("key: %u\r\n", key);
            if (key != last_key) {
                if (0 == last_key) {
                    distribute_key = key;
                }
            }
            last_key = key;
        } while (!distribute_key);

        if (2 == distribute_key) {
            dir = (dir + 1) % 4;
            lcd_dir = dir;
            printf("Changing dir to %u\r\n", dir);
            maixplayU4_config.lcd_dir = lcd_dir;
        } else if (1 == distribute_key) {
            brightness = (brightness + 1) % 11;
            bflb_pwm_v2_channel_set_threshold(pwm, PWM_CH2, 100, 100 + brightness_map[brightness]);
            printf("Changed pwm duty to %u%\r\n", brightness * 10);
            maixplayU4_config.lcd_bright = brightness;
        }

        {
            bflb_flash_erase(CONFIG_IN_FLASH_ADDRESS, FLASH_BLOCK_SIZE);
            bflb_flash_write(CONFIG_IN_FLASH_ADDRESS, (uint8_t *)&maixplayU4_config, sizeof(maixplayU4_config_t));
        }
    }

    vTaskDelete(NULL);
}

int main(void)
{
    board_init();

    struct bflb_device_s *gpio;
    gpio = bflb_device_get_by_name("gpio");
    /* backlight pin */
    bflb_gpio_init(gpio, GPIO_PIN_2, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_set(gpio, GPIO_PIN_2);

    lcd_init();
    // lcd_async_callback_register(NULL);

    lcd_clear(LCD_COLOR_RGB(0x00, 0X00, 0X00));

    cdc_acm_init();

    struct bflb_device_s *uart0;
    uart0 = bflb_device_get_by_name("uart0");
    shell_init_with_task(uart0);

    display_timer_handle = xTimerCreate("display_timer", pdMS_TO_TICKS(10), pdTRUE, NULL, vDisplayTimer);
    xTimerStart(
        display_timer_handle,
        pdMS_TO_TICKS(50));

    xTaskCreate(button_task, (char *)"producer_task", 512, NULL, configMAX_PRIORITIES - 3, &button_task_handle);

    vTaskStartScheduler();

    while (1) {
    }
}
