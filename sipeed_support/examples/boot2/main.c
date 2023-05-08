#include <assert.h>
#include <stdint.h>

#include "csi_rv32_gcc.h"

#include "bflb_gpio.h"
#include "bflb_mtimer.h"
#include "board.h"

#include "lcd.h"

#include "selector.h"
#include "coroutine.h"

#define BTN              GPIO_PIN_2
#define LED1             GPIO_PIN_16

#define FLASH_BLOCK_SIZE 4096
#define MAX_INIT_TIME    300
#define AUTO_START_TIME  2000

static void led_blink(uint8_t value, uint64_t delay_ms)
{
    static bool is_active = false;
    static uint64_t last_changed_time = 0;
    static struct bflb_device_s *gpio = NULL;

#define LED1_ON  bflb_gpio_reset(gpio, LED1)
#define LED1_OFF bflb_gpio_set(gpio, LED1)

    scrBegin;
    gpio = bflb_device_get_by_name("gpio");
    for (;;) {
        uint64_t now = bflb_mtimer_get_time_ms();
        if (now - last_changed_time > delay_ms) {
            last_changed_time = now;
            is_active ^= true;
        }

        if (is_active) {
            /* some on */
            if ((value >> 0) & 0x1) {
                LED1_ON;
            } else {
                LED1_OFF;
            }
        } else {
            /* all off */
            LED1_OFF;
        }
        scrReturnV;
    }
    scrFinishV;
}

static struct {
    const uint32_t firmware_address;
    const char firmware_name[12];
} supported_firmwares[] = {
    { 0x10000, "MaixPlay-U4" },
    { 0x50000, "Pika Python" },
};

static volatile bool flag_long_pressed = false;

#define ARRAY_SIZE(a) ((size_t)(sizeof(a)) / sizeof(a[0]))

static void gpio_isr(int irq, void *arg)
{
    static struct bflb_device_s *gpio = NULL;
    if (unlikely(gpio == NULL))
        gpio = bflb_device_get_by_name("gpio");

    selector_t *pSelector = (selector_t *)arg;

    if (bflb_gpio_get_intstatus(gpio, BTN)) {
        static uint64_t int_counts = 0;
        static uint64_t last_time = 0;
        uint64_t curr_time = bflb_mtimer_get_time_ms();
        int_counts += 1;
        {
            if (int_counts & 0x1) {
                /* press */
            } else {
                /* release */
                if (curr_time - last_time > 700) {
                    /* has long press */
                    flag_long_pressed = true;
                } else {
                    /* click */
                    selector_next(pSelector);
                    printf("[gpio_isr]select:\r\n");
                    uint8_t curr_select = selector_idx(pSelector);
                    printf("\t(%u)%s@0x%x\r\n", curr_select, supported_firmwares[curr_select].firmware_name, 0 * supported_firmwares[curr_select].firmware_address);
                }
            }
        }
        last_time = curr_time;
        bflb_gpio_int_clear(gpio, BTN);
    }
}

enum {
    STATE_CONFIGURING = 0,
    STATE_STARTING_AUTO,
    STATE_STARTING_MANUAL,
    STATE_OTA,
    STATE_MAX,
};

#include "fw_header.h"

#include "bflb_flash.h"
#include "bflb_l1c.h"
bool need_reboot_after_upgrade = false;
void ATTR_TCM_SECTION bflb_jump_encrypted_app(uint8_t index, uint32_t flash_addr, uint32_t len);
int main(void)
{
    board_init();
    struct bflb_device_s *gpio = bflb_device_get_by_name("gpio");

    {
        struct bootheader_t bh;

        bflb_gpio_init(gpio, BTN, GPIO_OUTPUT | GPIO_SMT_EN | GPIO_DRV_0);
        bflb_gpio_set(gpio, BTN);
        lcd_init();

        lcd_clear(LCD_COLOR_RGB(0xff, 0xff, 0xff));
        lcd_draw_rectangle(0, LCD_H / 2 - 16, LCD_W - 1, LCD_H / 2 + 48, LCD_COLOR_RGB(0xff, 0x00, 0x00));

        for (uint32_t i = 0; i < ARRAY_SIZE(supported_firmwares); i++) {
            bflb_flash_read(supported_firmwares[i].firmware_address, (void *)&bh, sizeof(struct bootheader_t));

            uint32_t fw_offset = supported_firmwares[i].firmware_address + bh.basic_cfg.group_image_offset;
            uint32_t fw_length = bh.basic_cfg.img_len_cnt;

            uint8_t version_string[16];
            bflb_flash_read(fw_offset + fw_length, (void *)version_string, sizeof(version_string));

            lcd_draw_str_ascii16(16,
                                 LCD_H / 2 - 16 + 4 + 16 * i, LCD_COLOR_RGB(0x00, 0x00, 0xff), LCD_COLOR_RGB(0xff, 0xff, 0xff),
                                 (void *)supported_firmwares[i].firmware_name, strlen(supported_firmwares[i].firmware_name));
            lcd_draw_str_ascii16(16 + 8 * (sizeof(supported_firmwares[i].firmware_name)),
                                 LCD_H / 2 - 16 + 4 + 16 * i, LCD_COLOR_RGB(0x00, 0x00, 0xff), LCD_COLOR_RGB(0xff, 0xff, 0xff),
                                 version_string, strlen((void *)version_string));
        }
        // bflb_mtimer_delay_ms(500);
    }

    uint64_t last_operate_time = bflb_mtimer_get_time_ms();

    selector_t selector;
    uint32_t selector_in_flash_address = supported_firmwares[0].firmware_address - FLASH_BLOCK_SIZE;
    bflb_flash_read(selector_in_flash_address, (uint8_t *)&selector, sizeof(selector));
    if (selector.__flags != SELECTOR_INITED_MAGIC ||
        selector.__max != ARRAY_SIZE(supported_firmwares)) {
        selector_t new_selector = selector_init(ARRAY_SIZE(supported_firmwares));
        memcpy(&selector, &new_selector, sizeof(selector_t));
        bflb_flash_erase(selector_in_flash_address, FLASH_BLOCK_SIZE);
        bflb_flash_write(selector_in_flash_address, (uint8_t *)&selector, sizeof(selector));
    }

    uint8_t started_select = selector_idx(&selector);

    bflb_gpio_init(gpio, GPIO_PIN_3, GPIO_INPUT | GPIO_SMT_EN | GPIO_DRV_0);
    bflb_gpio_init(gpio, LED1, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
    // bflb_gpio_init(gpio, BTN, GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_0);

    bflb_gpio_int_init(gpio, BTN, GPIO_INT_TRIG_MODE_SYNC_FALLING_RISING_EDGE);
    bflb_gpio_int_mask(gpio, BTN, false);

    bflb_irq_attach(gpio->irq_num, gpio_isr, &selector);
    bflb_irq_enable(gpio->irq_num);

    printf("Init SUCCESS\r\n"
           "Start LOOP\r\n");
    assert(bflb_mtimer_get_time_ms() - last_operate_time < MAX_INIT_TIME);
    uint8_t last_select = started_select;
    uint32_t state = STATE_CONFIGURING;
    while (bflb_mtimer_get_time_ms() - last_operate_time < 3 * MAX_INIT_TIME) {
        led_blink(0b11, 200 /* ms */);
    }
    while (1) {
        uint8_t curr_select = selector_idx(&selector);
        switch (state) {
            case STATE_CONFIGURING: {
                if (unlikely(!bflb_gpio_read(gpio, GPIO_PIN_3))) {
                    /* enter ota */
                    state = STATE_OTA;
                } else if (last_select != curr_select) {
                    /* next auto start */
                    last_operate_time = bflb_mtimer_get_time_ms();
                    last_select = curr_select;
                } else if (flag_long_pressed) {
                    /* manual start */
                    state = STATE_STARTING_MANUAL;
                } else if (bflb_mtimer_get_time_ms() - last_operate_time > AUTO_START_TIME) {
                    /* auto start */
                    state = STATE_STARTING_AUTO;
                }

                led_blink(curr_select, 1 /* ms */);
                for (uint32_t i = 0; i < selector.__max; i++) {
                    lcd_draw_str_ascii16(16 - 12,
                                         LCD_H / 2 - 16 + 4 + 16 * i, LCD_COLOR_RGB(0xff, 0x00, 0x00), LCD_COLOR_RGB(0xff, 0xff, 0xff),
                                         (void *)((i == curr_select) ? "*" : " "), 1);
                }
            } break;
            case STATE_STARTING_AUTO:
                printf("Auto \r");
                if (bflb_mtimer_get_time_ms() - last_operate_time < 1.5f * AUTO_START_TIME) {
                    led_blink(0b11, 300 /* ms */);
                    break;
                }
            case STATE_STARTING_MANUAL: {
                printf("Starting:\r\n"
                       "\t(%u)%s@0x%x\r\n",
                       curr_select, supported_firmwares[curr_select].firmware_name, 0 * supported_firmwares[curr_select].firmware_address);
                if (started_select != curr_select) {
                    bflb_flash_erase(selector_in_flash_address, FLASH_BLOCK_SIZE);
                    bflb_flash_write(selector_in_flash_address, (uint8_t *)&selector, sizeof(selector));
                }

                {
                    struct bootheader_t fw_h;
                    bflb_flash_read(supported_firmwares[curr_select].firmware_address, (void *)&fw_h, sizeof(struct bootheader_t));

                    uint8_t flag = fw_h.basic_cfg.encrypt_type;
                    printf("flag: %02x\r\n", flag);
                    uint32_t fw_offset = supported_firmwares[curr_select].firmware_address + fw_h.basic_cfg.group_image_offset;
                    uint32_t fw_length = fw_h.basic_cfg.img_len_cnt;

                    if (0b00 == flag) {
                        __disable_irq();
                        bflb_l1c_dcache_clean_invalidate_all();
                        bflb_l1c_icache_invalid_all();
                        bflb_flash_set_cache(true, true, 0, fw_offset);
                        void (*app_main)(void) = (void (*)(void))FLASH_XIP_BASE;
                        app_main();
                    } else if (0b01 == flag) {
                        bflb_jump_encrypted_app(0, fw_offset, fw_length);
                    }
                }

                printf("never reach\r\n");
                state = STATE_OTA;
            } break;
            case STATE_OTA: {
                bflb_gpio_init(gpio, BTN, GPIO_OUTPUT | GPIO_SMT_EN | GPIO_DRV_0);
                bflb_gpio_set(gpio, BTN);
                printf("Entering OTA......\r\n");

                extern void msc_ram_init(void);
                msc_ram_init();
                bool really_need_reboot_after_upgrade = false;
                while (1) {
                    if (really_need_reboot_after_upgrade) {
                        bflb_mtimer_delay_ms(50);
                        if (need_reboot_after_upgrade) {
                            GLB_SW_System_Reset();
                        }
                    }
                    really_need_reboot_after_upgrade = need_reboot_after_upgrade;
                    bflb_mtimer_delay_ms(500);
                }
            } break;
            case STATE_MAX: {
            } break;
        }
    }
}

#include "bflb_sf_ctrl.h"
#if defined(BL702) || defined(BL702L)
#define bflb_sf_ctrl_get_aes_region(addr, r) (addr + SF_CTRL_AES_REGION_OFFSET + (r)*0x100)
#else
#define bflb_sf_ctrl_get_aes_region(addr, r) (addr + SF_CTRL_AES_REGION_OFFSET + (r)*0x80)
#endif
#include "sf_ctrl_reg.h"
#include "bflb_sec_sha.h"
#include "bflb_flash.h"
#include "bflb_l1c.h"

__WEAK
void ATTR_TCM_SECTION bflb_sf_ctrl_aes_set_region_offset(uint8_t region, uint32_t addr_offset)
{
#if defined(BL602)
    uint32_t region_reg_base = bflb_sf_ctrl_get_aes_region(BFLB_SF_CTRL_BASE, !region);
#else
    uint32_t region_reg_base = bflb_sf_ctrl_get_aes_region(BFLB_SF_CTRL_BASE, region);
#endif

    putreg32(__REV(addr_offset), region_reg_base + SF_CTRL_SF_AES_IV_W3_OFFSET);
}

void ATTR_TCM_SECTION bflb_jump_encrypted_app(uint8_t index, uint32_t flash_addr, uint32_t len)
{
    void (*entry)(void);

    bflb_sf_ctrl_aes_set_region_offset(index, flash_addr);

    bflb_sf_ctrl_aes_set_region(index, 1 /*enable this region*/, 1 /*hardware key*/,
                                flash_addr,
                                flash_addr + len - 1,
                                0 /*lock*/);

    bflb_sf_ctrl_aes_enable_be();
    bflb_sf_ctrl_aes_enable();

    bflb_flash_set_cache(1, 1, 0, flash_addr);
    bflb_l1c_dcache_clean_invalidate_all();
    bflb_l1c_icache_invalid_all();

    entry = (void (*)(void))BFLB_FLASH_XIP_BASE;
    entry();
}
