#include "bflb_gpio.h"
#include "bflb_mtimer.h"
#include "board.h"

#define DBG_TAG "MAIN"
#include "log.h"

static uint8_t gpio_led_pins[] = {
    GPIO_PIN_0,
    GPIO_PIN_1,
    // GPIO_PIN_2,
    GPIO_PIN_3,

    GPIO_PIN_6,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
    GPIO_PIN_16,
    GPIO_PIN_17,
    GPIO_PIN_18,
    GPIO_PIN_19,
    GPIO_PIN_20,
    // GPIO_PIN_21,
    // GPIO_PIN_22,
    GPIO_PIN_23,
    GPIO_PIN_24,
    GPIO_PIN_25,
    GPIO_PIN_26,
    GPIO_PIN_27,
    GPIO_PIN_28,
    GPIO_PIN_29,
    GPIO_PIN_30,
    GPIO_PIN_31,
    GPIO_PIN_32,
    GPIO_PIN_33,
    GPIO_PIN_34,
};
#include "bl616_glb_gpio.h"
int main(void)
{
    board_init();
    struct bflb_device_s *gpio = bflb_device_get_by_name("gpio");

    bflb_gpio_init(gpio, GPIO_PIN_2, GPIO_INPUT | GPIO_SMT_EN);
    for (size_t idx = 0; idx < sizeof(gpio_led_pins) / sizeof(gpio_led_pins[0]);
         idx++)
        GLB_GPIO_Set_HZ(gpio_led_pins[idx]);

    int count = 0;
    while (1) {
        static bool last_state_btn = 0;
        bool curr_state_btn;
        curr_state_btn = GLB_GPIO_Read(GPIO_PIN_2);
        bool state_btn_pushed = last_state_btn && !curr_state_btn;
        last_state_btn = curr_state_btn;

        if (state_btn_pushed) {
            count += 1;
            if (count & 1) {
                LOG_I("HZ gpio set\r\n");
                for (size_t idx = 0; idx < sizeof(gpio_led_pins) / sizeof(gpio_led_pins[0]);
                     idx++) {
                    GLB_GPIO_Set_HZ(gpio_led_pins[idx]);
                }
            } else {
                LOG_I("low gpio set\r\n");
                for (size_t idx = 0; idx < sizeof(gpio_led_pins) / sizeof(gpio_led_pins[0]);
                     idx++) {
                    bflb_gpio_init(gpio, gpio_led_pins[idx],
                                   GPIO_INPUT | /*GPIO_PULLDOWN |*/ GPIO_SMT_EN | GPIO_DRV_0);
                }
            }
        }
    }
}
