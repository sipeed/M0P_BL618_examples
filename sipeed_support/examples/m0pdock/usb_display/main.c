#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "usbh_core.h"
#include "bflb_mtimer.h"
#include "bflb_gpio.h"
#include "board.h"
#include "shell.h"

#include "lcd.h"

#define DBG_TAG "MAIN"
#include "log.h"

extern void cdc_acm_init(void);
extern void lcd_display_loop(void);
extern void shell_init_with_task(struct bflb_device_s *shell);

static uint8_t lcd_dir;

static void vDisplayTimer(TimerHandle_t xTimer)
{
    // lcd_set_dir(lcd_dir, 0);
    lcd_display_loop();
}

static void button_task(void *pvParameters)
{
    struct bflb_device_s *gpio;
    gpio = bflb_device_get_by_name("gpio");
    bflb_gpio_init(gpio, GPIO_PIN_3, GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_0);
    bool push_button;
    uint32_t push_times;
    uint8_t dir;

    lcd_dir = dir = 0;
    while (1) {
        push_times = 0;
        do {
            vTaskDelay(pdMS_TO_TICKS(10));
            push_button = (0 == bflb_gpio_read(gpio, GPIO_PIN_3));
            push_times += push_button;
        } while (push_button);
        if (push_times) {
            dir = (dir + 1) % 4;
            lcd_dir = dir;
            // lcd_set_dir(dir, 0);
            // printf("Change dir to %u\r\n", dir);
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

    xTimerStart(
        xTimerCreate("display_timer", pdMS_TO_TICKS(10), pdTRUE, NULL, vDisplayTimer),
        pdMS_TO_TICKS(50));

    xTaskHandle button_task_handle;
    xTaskCreate(button_task, (char *)"producer_task", 512, NULL, configMAX_PRIORITIES - 3, &button_task_handle);

    vTaskStartScheduler();

    while (1) {
    }
}
