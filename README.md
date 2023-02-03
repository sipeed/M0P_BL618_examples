# M0P_BL618_examples
M0P_BL618_examples

M0PDock uses DIO mode for spi flash, so flashing and starting need some extra operations.

If you ignored, either failed on flashing or can not start.

## Preoperations

First of all, please refer to `utils/flash/bl616/flashcfg_list.csv` to find out which conf file is associated with the `flash ID` fetched from `BL LAB DEV CUBE`.

1. chips/bl616/eflash_loader/eflash_loader_cfg.ini

    - flash_io_mode = 1

2. chips/bl616/img_create_mcu/efuse_bootheader_cfg.ini

    - cmds_wrap_mode = 1

    - cmds_wrap_len = 9

3. utils/flash/bl616/W25Q64JW_ef6017.conf **not always `W25Q64JW_ef6017.conf`, refer to `flashcfg_list.csv` or table below**

    - io_mode = 0x13

    - cont_read_support = 0/1 **depends on different chips, please test all options**

4. comment `bflb_flash_init()` in your code.

|FLASH ID|     true_cfg_file    |cont_read_support|
|   ---  |         ---          |       ---       |
| ef4018 | W25Q64JW_ef6017.conf |        0        |
| 0b4018 | GD25Q80E_c84014.conf |        1        |
| 0b4017 | GD25Q80E_c84014.conf |        1        |

Now can normally flash your firmware and run it.


## SDK

Official [MCU SDK](https://github.com/bouffalolab/bl_mcu_sdk).

Download it and read its document.

[Here](https://bl-mcu-sdk.readthedocs.io/zh_CN/latest/api_reference/peripherals/index.html) is the API of its peripherals.

## DOC

[DS and RM](https://dl.sipeed.com/shareURL/Maix-Zero/M0P/6_Chip_Manual)

