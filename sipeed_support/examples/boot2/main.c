#include <assert.h>
#include <stdint.h>

#include "csi_rv32_gcc.h"

#include "bflb_gpio.h"
#include "bflb_mtimer.h"
#include "board.h"

#include "lcd.h"

#include "selector.h"
#include "coroutine.h"

#define BTN              GPIO_PIN_3
#define LED1             GPIO_PIN_16

#define FLASH_BLOCK_SIZE 4096
#define MAX_INIT_TIME    300
#define AUTO_START_TIME  2000

const uint16_t logo_fg[] = {
    2, 58, 299, 357, 596, 655, 894, 954, 1192, 1252, 1490, 1499, 1502, 1550, 1788, 1795, 1802, 1848, 2086, 2092, 2101, 2111, 2131, 2146, 2384, 2390, 2399, 2407, 2429, 2444, 2474, 2490, 2502, 2510, 2533, 2542, 2577, 2595, 2618, 2636, 2645, 2652, 2656, 2667, 2682, 2688, 2692, 2693, 2697, 2704, 2727, 2742, 2770, 2789, 2800, 2809, 2829, 2843, 2872, 2894, 2914, 2935, 2942, 2951, 2953, 2967, 2980, 2986, 2990, 2991, 2995, 3001, 3025, 3040, 3067, 3087, 3098, 3107, 3126, 3142, 3169, 3192, 3211, 3233, 3240, 3249, 3251, 3266, 3278, 3284, 3293, 3298, 3323, 3338, 3364, 3385, 3396, 3405, 3423, 3441, 3466, 3490, 3508, 3531, 3538, 3547, 3549, 3565, 3576, 3583, 3590, 3595, 3621, 3636, 3661, 3682, 3694, 3703, 3720, 3740, 3763, 3788, 3804, 3829, 3836, 3845, 3847, 3864, 3874, 3882, 3887, 3892, 3919, 3934, 3958, 3980, 3992, 4001, 4017, 4039, 4060, 4086, 4101, 4127, 4134, 4143, 4145, 4163, 4172, 4189, 4217, 4232, 4255, 4277, 4290, 4299, 4314, 4338, 4357, 4384, 4398, 4425, 4432, 4441, 4443, 4462, 4470, 4486, 4515, 4530, 4552, 4574, 4588, 4597, 4610, 4637, 4654, 4682, 4695, 4723, 4730, 4739, 4741, 4762, 4768, 4783, 4813, 4828, 4849, 4871, 4886, 4894, 4907, 4936, 4951, 4979, 4992, 5021, 5028, 5037, 5039, 5061, 5066, 5080, 5089, 5126, 5146, 5153, 5204, 5212, 5228, 5236, 5247, 5255, 5289, 5297, 5326, 5335, 5353, 5360, 5364, 5377, 5387, 5424, 5443, 5450, 5501, 5509, 5527, 5535, 5544, 5552, 5586, 5593, 5624, 5633, 5651, 5659, 5662, 5674, 5685, 5722, 5740, 5748, 5798, 5806, 5825, 5834, 5841, 5849, 5883, 5891, 5922, 5931, 5949, 5958, 5960, 5971, 5983, 6020, 6038, 6046, 6095, 6104, 6123, 6132, 6139, 6147, 6180, 6188, 6220, 6229, 6247, 6256, 6258, 6270, 6281, 6318, 6336, 6344, 6393, 6402, 6421, 6430, 6436, 6445, 6478, 6486, 6518, 6527, 6545, 6569, 6579, 6616, 6634, 6642, 6691, 6700, 6719, 6727, 6734, 6743, 6775, 6784, 6816, 6825, 6843, 6868, 6877, 6914, 6933, 6940, 6989, 6998, 7017, 7024, 7032, 7041, 7073, 7082, 7114, 7123, 7141, 7167, 7175, 7212, 7232, 7239, 7287, 7296, 7313, 7321, 7330, 7339, 7371, 7380, 7412, 7421, 7439, 7466, 7487, 7510, 7531, 7549, 7570, 7577, 7585, 7594, 7596, 7618, 7628, 7637, 7639, 7658, 7669, 7678, 7681, 7700, 7710, 7719, 7737, 7765, 7786, 7808, 7830, 7848, 7867, 7875, 7883, 7892, 7894, 7915, 7926, 7935, 7937, 7958, 7967, 7976, 7979, 7999, 8008, 8017, 8035, 8064, 8085, 8106, 8129, 8147, 8165, 8173, 8181, 8190, 8192, 8212, 8224, 8233, 8235, 8257, 8265, 8274, 8277, 8298, 8306, 8315, 8333, 8363, 8384, 8404, 8428, 8446, 8462, 8471, 8479, 8488, 8490, 8509, 8522, 8531, 8533, 8555, 8563, 8572, 8575, 8596, 8604, 8613, 8631, 8662, 8683, 8702, 8727, 8745, 8760, 8769, 8777, 8786, 8788, 8806, 8820, 8829, 8831, 8853, 8861, 8870, 8873, 8895, 8902, 8911, 8929, 8961, 8982, 9000, 9026, 9044, 9058, 9067, 9075, 9084, 9086, 9103, 9118, 9127, 9129, 9152, 9159, 9168, 9171, 9193, 9200, 9209, 9227, 9260, 9281, 9298, 9325, 9343, 9356, 9365, 9373, 9382, 9384, 9399, 9416, 9425, 9427, 9450, 9457, 9466, 9469, 9491, 9498, 9507, 9525, 9559, 9580, 9596, 9624, 9642, 9654, 9663, 9671, 9680, 9682, 9696, 9714, 9723, 9725, 9748, 9755, 9764, 9767, 9789, 9796, 9805, 9823, 9858, 9879, 9894, 9934, 9941, 9952, 9961, 9969, 9978, 10012, 10021, 10053, 10062, 10094, 10103, 10121, 10170, 10178, 10192, 10233, 10240, 10250, 10259, 10267, 10276, 10310, 10319, 10351, 10360, 10392, 10401, 10419, 10468, 10477, 10490, 10531, 10539, 10548, 10557, 10565, 10574, 10608, 10617, 10649, 10658, 10690, 10699, 10717, 10766, 10776, 10788, 10829, 10838, 10846, 10855, 10863, 10872, 10906, 10915, 10948, 10956, 10988, 10997, 11015, 11064, 11075, 11086, 11127, 11136, 11144, 11153, 11161, 11170, 11205, 11213, 11246, 11254, 11286, 11295, 11313, 11322, 11324, 11362, 11373, 11384, 11425, 11433, 11442, 11451, 11459, 11468, 11503, 11511, 11545, 11553, 11584, 11593, 11611, 11620, 11622, 11660, 11670, 11682, 11723, 11731, 11740, 11749, 11757, 11766, 11802, 11810, 11844, 11851, 11882, 11891, 11909, 11917, 11920, 11958, 11967, 11980, 12020, 12028, 12038, 12047, 12055, 12064, 12101, 12110, 12143, 12151, 12180, 12189, 12207, 12214, 12218, 12256, 12264, 12278, 12302, 12325, 12336, 12345, 12353, 12362, 12401, 12429, 12442, 12471, 12478, 12487, 12489, 12511, 12516, 12532, 12561, 12576, 12599, 12622, 12634, 12643, 12651, 12660, 12700, 12728, 12741, 12769, 12776, 12785, 12787, 12808, 12814, 12829, 12858, 12874, 12896, 12919, 12932, 12941, 12949, 12958, 12999, 13026, 13040, 13067, 13074, 13083, 13085, 13104, 13112, 13127, 13155, 13172, 13193, 13216, 13230, 13239, 13247, 13256, 13298, 13324, 13339, 13365, 13372, 13381, 13383, 13401, 13410, 13425, 13452, 13470, 13491, 13513, 13528, 13537, 13545, 13554, 13597, 13622, 13638, 13663, 13670, 13679, 13681, 13698, 13708, 13723, 13749, 13768, 13789, 13810, 13826, 13835, 13843, 13852, 13896, 13920, 13938, 13961, 13968, 13977, 13979, 13995, 14006, 14021, 14046, 14066, 14087, 14107, 14124, 14133, 14141, 14150, 14195, 14218, 14237, 14259, 14266, 14275, 14277, 14292, 14304, 14319, 14343, 14364, 14385, 14404, 14422, 14431, 14439, 14448, 14494, 14516, 14536, 14557, 14564, 14573, 14575, 14589, 14602, 14617, 14640, 14662, 14683, 14700, 14720, 14728, 14738, 14745, 14795, 14813, 14836, 14854, 14863, 14870, 14874, 14885, 14900, 14915, 14937, 14960, 15198, 15214, 15234, 15258, 15496, 15556, 15794, 15854, 16092, 16152, 16390, 16449, 16689, 16747, 16988, 17044, 17287, 17341
};

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
static volatile char *OTA_FIRMWARE_STATES[ARRAY_SIZE(supported_firmwares)] = { "Wait...", "Wait..." };
void ota_firmware_update_state(uint32_t blk_curr, uint32_t blk_total, uint32_t target_addr)
{
    static char states[ARRAY_SIZE(supported_firmwares)][8];

    uint32_t curr_state = blk_curr * 100 / blk_total;

    // printf("curr: %3u%%, addr: %x\r\n", curr_state, target_addr);
    if (curr_state % 4)
        return;

    int curr_firmware = -1;
    for (uint32_t i = ARRAY_SIZE(supported_firmwares) - 1; i >= 0; i--) {
        if (target_addr >= supported_firmwares[i].firmware_address) {
            curr_firmware = i;
            break;
        }
    }
    if (-1 == curr_firmware)
        return;

    if (curr_state >= 0 && curr_state < 100) {
        snprintf(states[curr_firmware], sizeof(states[0]), "%3u%%%3s",
                 curr_state,
                 (char *[]){ ".  ", ".. ", "..." }[blk_curr / 5 % 3]);
        OTA_FIRMWARE_STATES[curr_firmware] = states[curr_firmware];
    } else if (curr_state == 100) {
        OTA_FIRMWARE_STATES[curr_firmware] = "Finish.";
    }
}
void ATTR_TCM_SECTION bflb_jump_encrypted_app(uint8_t index, uint32_t flash_addr, uint32_t len);
int main(void)
{
    board_init();
    struct bflb_device_s *gpio = bflb_device_get_by_name("gpio");
    bflb_gpio_init(gpio, BTN, GPIO_INPUT | GPIO_SMT_EN | GPIO_DRV_0);
    bool need_enter_OTA = !bflb_gpio_read(gpio, BTN);

    {
        struct bootheader_t bh;

        bflb_gpio_init(gpio, GPIO_PIN_2, GPIO_OUTPUT | GPIO_SMT_EN | GPIO_DRV_0);
        bflb_gpio_set(gpio, GPIO_PIN_2);
        lcd_init();

        lcd_clear(LCD_COLOR_RGB(0x00, 0x00, 0x00));

        /* LOGO */
        uint16_t idx = 0;
        for (uint32_t h = 0; h < 59; h++) {
            for (uint32_t w = 0; w < 298; w++) {
                uint16_t fg_color = LCD_COLOR_RGB(227, 69, 72); //0xe21b; //0xda1c;
                uint32_t curr_idx = h * 298 + w;

                if (idx < ARRAY_SIZE(logo_fg) - 1 && curr_idx > logo_fg[idx + 1]) {
                    idx += 2;
                }

                if (curr_idx >= logo_fg[idx] && curr_idx <= logo_fg[idx + 1]) {
                    lcd_draw_point(w, LCD_H - 59 + h, fg_color);
                }
            }
        }

        /* TIPs */
        lcd_draw_str_ascii16(0, 0,
                             LCD_COLOR_RGB(0xff, 0x00, 0x00), LCD_COLOR_RGB(0x00, 0x00, 0x00),
                             "SWITCH Firmware:\n Press side keys, select, wait 2s will enter the function", 74);

        lcd_draw_str_ascii16(0, 0 + 16 + 16 + 16 + 4,
                             LCD_COLOR_RGB(0xff, 0x00, 0xff), LCD_COLOR_RGB(0x00, 0x00, 0x00),
                             "UPDATE Firmware:\n Press side key, hold, press RESET, "
                             "USB Mass Storage will come out on PC, drop firmware to it",
                             110);

        lcd_draw_rectangle(0, LCD_H / 2 - 16, LCD_W - 1, LCD_H / 2 + 48, LCD_COLOR_RGB(0xff, 0x00, 0x00));
        lcd_draw_str_ascii16(16,
                             LCD_H / 2 - 16 - 4 - 16, LCD_COLOR_RGB(0xff, 0x00, 0x00), LCD_COLOR_RGB(0x00, 0x00, 0x00),
                             "BOOT", 4);
        for (uint32_t i = 0; i < ARRAY_SIZE(supported_firmwares); i++) {
            bflb_flash_read(supported_firmwares[i].firmware_address, (void *)&bh, sizeof(struct bootheader_t));

            uint32_t fw_offset = supported_firmwares[i].firmware_address + bh.basic_cfg.group_image_offset;
            uint32_t fw_length = bh.basic_cfg.img_len_cnt;

            uint8_t version_string[16];
            bflb_flash_read(fw_offset + fw_length, (void *)version_string, sizeof(version_string));

            lcd_draw_str_ascii16(16,
                                 LCD_H / 2 - 16 + 4 + 16 * i, LCD_COLOR_RGB(0x00, 0x00, 0xff), LCD_COLOR_RGB(0x00, 0x00, 0x00),
                                 (void *)supported_firmwares[i].firmware_name, strlen(supported_firmwares[i].firmware_name));
            lcd_draw_str_ascii16(16 + 8 * (sizeof(supported_firmwares[i].firmware_name)),
                                 LCD_H / 2 - 16 + 4 + 16 * i, LCD_COLOR_RGB(0x00, 0x00, 0xff), LCD_COLOR_RGB(0x00, 0x00, 0x00),
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

    bflb_gpio_init(gpio, LED1, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);

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
                if (unlikely(need_enter_OTA)) {
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
                                         LCD_H / 2 - 16 + 4 + 16 * i, LCD_COLOR_RGB(0xff, 0x00, 0x00), LCD_COLOR_RGB(0x00, 0x00, 0x00),
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
                printf("Entering OTA......\r\n");
                lcd_draw_str_ascii16(16,
                                     LCD_H / 2 - 16 - 4 - 16, LCD_COLOR_RGB(0x00, 0x00, 0x00), LCD_COLOR_RGB(0x00, 0x00, 0x00),
                                     "    ", 4);
                lcd_draw_str_ascii16(LCD_W - 8 * 8,
                                     LCD_H / 2 - 16 - 4 - 16, LCD_COLOR_RGB(0xff, 0x00, 0xff), LCD_COLOR_RGB(0x00, 0x00, 0x00),
                                     "OTA", 3);
                bflb_gpio_int_mask(gpio, BTN, true);
                bflb_gpio_deinit(gpio, BTN);
                extern void msc_ram_init(void);
                msc_ram_init();
                bool really_need_reboot_after_upgrade = false;
                while (1) {
                    for (uint32_t i = 0; i < selector.__max; i++) {
                        lcd_draw_str_ascii16(LCD_W - 8 * 8,
                                             LCD_H / 2 - 16 + 4 + 16 * i, LCD_COLOR_RGB(0xff, 0x00, 0xff), LCD_COLOR_RGB(0x00, 0x00, 0x00),
                                             (void *)OTA_FIRMWARE_STATES[i], strlen(OTA_FIRMWARE_STATES[i]));
                    }

                    if (really_need_reboot_after_upgrade) {
                        bflb_mtimer_delay_ms(50);
                        if (need_reboot_after_upgrade) {
                            GLB_SW_System_Reset();
                        }
                    }
                    really_need_reboot_after_upgrade = need_reboot_after_upgrade;
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
