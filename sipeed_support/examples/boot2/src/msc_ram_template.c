#include "usbd_core.h"
#include "usbd_msc.h"

#define MSC_IN_EP          0x81
#define MSC_OUT_EP         0x02

#define USBD_VID           0xFFFF
#define USBD_PID           0xFFFF
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

#define USB_CONFIG_SIZE    (9 + MSC_DESCRIPTOR_LEN)

#define BLOCK_SIZE         CONFIG_USBDEV_MSC_BLOCK_SIZE
#define BLOCK_COUNT        10

typedef uint8_t BLOCK_TYPE[BLOCK_SIZE];

static BLOCK_TYPE mass_block[BLOCK_COUNT];

static const uint8_t msc_ram_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    MSC_DESCRIPTOR_INIT(0x00, MSC_OUT_EP, MSC_IN_EP, 0x02),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'M', 0x00,                  /* wcChar10 */
    'S', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '2', 0x00,                  /* wcChar3 */
    '1', 0x00,                  /* wcChar4 */
    '2', 0x00,                  /* wcChar5 */
    '3', 0x00,                  /* wcChar6 */
    '4', 0x00,                  /* wcChar7 */
    '5', 0x00,                  /* wcChar8 */
    '6', 0x00,                  /* wcChar9 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
#endif
    0x00
};

void msc_ram_init(void)
{
    {
        uint8_t flash_0x01b0[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a, 0x4d, 0x82, 0x02, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x01, 0x0e, 0x00, 0x41, 0xff, 0x01, 0x00, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00
        };
        uint8_t flash_0x01f8[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xaa
        };
        uint8_t flash_0x1000[] = {
            0xeb, 0x3c, 0x90, 0x4d, 0x53, 0x44, 0x4f, 0x53, 0x35, 0x2e, 0x30, 0x00, 0x10, 0x01, 0x01, 0x00,
            0x02, 0x00, 0x02, 0xff, 0x01, 0xf8, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x29, 0xa6, 0xcb, 0x76, 0x38, 0x4e, 0x4f, 0x20, 0x4e, 0x41,
            0x4d, 0x45, 0x20, 0x20, 0x20, 0x20, 0x46, 0x41, 0x54, 0x31, 0x32, 0x20, 0x20, 0x20, 0x33, 0xc9,
            0x8e, 0xd1, 0xbc, 0xf0, 0x7b, 0x8e, 0xd9, 0xb8, 0x00, 0x20, 0x8e, 0xc0, 0xfc, 0xbd, 0x00, 0x7c,
            0x38, 0x4e, 0x24, 0x7d, 0x24, 0x8b, 0xc1, 0x99, 0xe8, 0x3c, 0x01, 0x72, 0x1c, 0x83, 0xeb, 0x3a,
            0x66, 0xa1, 0x1c, 0x7c, 0x26, 0x66, 0x3b, 0x07, 0x26, 0x8a, 0x57, 0xfc, 0x75, 0x06, 0x80, 0xca,
            0x02, 0x88, 0x56, 0x02, 0x80, 0xc3, 0x10, 0x73, 0xeb, 0x33, 0xc9, 0x8a, 0x46, 0x10, 0x98, 0xf7,
            0x66, 0x16, 0x03, 0x46, 0x1c, 0x13, 0x56, 0x1e, 0x03, 0x46, 0x0e, 0x13, 0xd1, 0x8b, 0x76, 0x11,
            0x60, 0x89, 0x46, 0xfc, 0x89, 0x56, 0xfe, 0xb8, 0x20, 0x00, 0xf7, 0xe6, 0x8b, 0x5e, 0x0b, 0x03,
            0xc3, 0x48, 0xf7, 0xf3, 0x01, 0x46, 0xfc, 0x11, 0x4e, 0xfe, 0x61, 0xbf, 0x00, 0x00, 0xe8, 0xe6,
            0x00, 0x72, 0x39, 0x26, 0x38, 0x2d, 0x74, 0x17, 0x60, 0xb1, 0x0b, 0xbe, 0xa1, 0x7d, 0xf3, 0xa6,
            0x61, 0x74, 0x32, 0x4e, 0x74, 0x09, 0x83, 0xc7, 0x20, 0x3b, 0xfb, 0x72, 0xe6, 0xeb, 0xdc, 0xa0,
            0xfb, 0x7d, 0xb4, 0x7d, 0x8b, 0xf0, 0xac, 0x98, 0x40, 0x74, 0x0c, 0x48, 0x74, 0x13, 0xb4, 0x0e,
            0xbb, 0x07, 0x00, 0xcd, 0x10, 0xeb, 0xef, 0xa0, 0xfd, 0x7d, 0xeb, 0xe6, 0xa0, 0xfc, 0x7d, 0xeb,
            0xe1, 0xcd, 0x16, 0xcd, 0x19, 0x26, 0x8b, 0x55, 0x1a, 0x52, 0xb0, 0x01, 0xbb, 0x00, 0x00, 0xe8,
            0x3b, 0x00, 0x72, 0xe8, 0x5b, 0x8a, 0x56, 0x24, 0xbe, 0x0b, 0x7c, 0x8b, 0xfc, 0xc7, 0x46, 0xf0,
            0x3d, 0x7d, 0xc7, 0x46, 0xf4, 0x29, 0x7d, 0x8c, 0xd9, 0x89, 0x4e, 0xf2, 0x89, 0x4e, 0xf6, 0xc6,
            0x06, 0x96, 0x7d, 0xcb, 0xea, 0x03, 0x00, 0x00, 0x20, 0x0f, 0xb6, 0xc8, 0x66, 0x8b, 0x46, 0xf8,
            0x66, 0x03, 0x46, 0x1c, 0x66, 0x8b, 0xd0, 0x66, 0xc1, 0xea, 0x10, 0xeb, 0x5e, 0x0f, 0xb6, 0xc8,
            0x4a, 0x4a, 0x8a, 0x46, 0x0d, 0x32, 0xe4, 0xf7, 0xe2, 0x03, 0x46, 0xfc, 0x13, 0x56, 0xfe, 0xeb,
            0x4a, 0x52, 0x50, 0x06, 0x53, 0x6a, 0x01, 0x6a, 0x10, 0x91, 0x8b, 0x46, 0x18, 0x96, 0x92, 0x33,
            0xd2, 0xf7, 0xf6, 0x91, 0xf7, 0xf6, 0x42, 0x87, 0xca, 0xf7, 0x76, 0x1a, 0x8a, 0xf2, 0x8a, 0xe8,
            0xc0, 0xcc, 0x02, 0x0a, 0xcc, 0xb8, 0x01, 0x02, 0x80, 0x7e, 0x02, 0x0e, 0x75, 0x04, 0xb4, 0x42,
            0x8b, 0xf4, 0x8a, 0x56, 0x24, 0xcd, 0x13, 0x61, 0x61, 0x72, 0x0b, 0x40, 0x75, 0x01, 0x42, 0x03,
            0x5e, 0x0b, 0x49, 0x75, 0x06, 0xf8, 0xc3, 0x41, 0xbb, 0x00, 0x00, 0x60, 0x66, 0x6a, 0x00, 0xeb,
            0xb0, 0x42, 0x4f, 0x4f, 0x54, 0x4d, 0x47, 0x52, 0x20, 0x20, 0x20, 0x20, 0x0d, 0x0a, 0x52, 0x65,
            0x6d, 0x6f, 0x76, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6b, 0x73, 0x20, 0x6f, 0x72, 0x20, 0x6f, 0x74,
            0x68, 0x65, 0x72, 0x20, 0x6d, 0x65, 0x64, 0x69, 0x61, 0x2e, 0xff, 0x0d, 0x0a, 0x44, 0x69, 0x73,
            0x6b, 0x20, 0x65, 0x72, 0x72, 0x6f, 0x72, 0xff, 0x0d, 0x0a, 0x50, 0x72, 0x65, 0x73, 0x73, 0x20,
            0x61, 0x6e, 0x79, 0x20, 0x6b, 0x65, 0x79, 0x20, 0x74, 0x6f, 0x20, 0x72, 0x65, 0x73, 0x74, 0x61,
            0x72, 0x74, 0x0d, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xac, 0xcb, 0xd8, 0x55, 0xaa,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        uint8_t flash_0x2000_3000[] = {
            0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f
        };
        uint8_t flash_0x4000[] = {
            0x4d, 0x50, 0x55, 0x34, 0x2d, 0x4f, 0x54, 0x41, 0x20, 0x20, 0x20, 0x08, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x67, 0x85, 0xa4, 0x56, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x42, 0x20, 0x00, 0x49, 0x00, 0x6e, 0x00, 0x66, 0x00, 0x6f, 0x00, 0x0f, 0x00, 0x72, 0x72, 0x00,
            0x6d, 0x00, 0x61, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x00, 0x00, 0x6e, 0x00, 0x00, 0x00,
            0x01, 0x53, 0x00, 0x79, 0x00, 0x73, 0x00, 0x74, 0x00, 0x65, 0x00, 0x0f, 0x00, 0x72, 0x6d, 0x00,
            0x20, 0x00, 0x56, 0x00, 0x6f, 0x00, 0x6c, 0x00, 0x75, 0x00, 0x00, 0x00, 0x6d, 0x00, 0x65, 0x00,
            0x53, 0x59, 0x53, 0x54, 0x45, 0x4d, 0x7e, 0x31, 0x20, 0x20, 0x20, 0x16, 0x00, 0xc2, 0x66, 0x85,
            0xa4, 0x56, 0xa4, 0x56, 0x00, 0x00, 0x67, 0x85, 0xa4, 0x56, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        uint8_t flash_0x8000[] = {
            0x2e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0xc2, 0x66, 0x85,
            0xa4, 0x56, 0xa4, 0x56, 0x00, 0x00, 0x67, 0x85, 0xa4, 0x56, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x2e, 0x2e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0xc2, 0x66, 0x85,
            0xa4, 0x56, 0xa4, 0x56, 0x00, 0x00, 0x67, 0x85, 0xa4, 0x56, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x42, 0x74, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xce, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
            0x01, 0x57, 0x00, 0x50, 0x00, 0x53, 0x00, 0x65, 0x00, 0x74, 0x00, 0x0f, 0x00, 0xce, 0x74, 0x00,
            0x69, 0x00, 0x6e, 0x00, 0x67, 0x00, 0x73, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x64, 0x00, 0x61, 0x00,
            0x57, 0x50, 0x53, 0x45, 0x54, 0x54, 0x7e, 0x31, 0x44, 0x41, 0x54, 0x20, 0x00, 0xc2, 0x66, 0x85,
            0xa4, 0x56, 0xa4, 0x56, 0x00, 0x00, 0x67, 0x85, 0xa4, 0x56, 0x03, 0x00, 0x0c, 0x00, 0x00, 0x00,
            0x42, 0x47, 0x00, 0x75, 0x00, 0x69, 0x00, 0x64, 0x00, 0x00, 0x00, 0x0f, 0x00, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
            0x01, 0x49, 0x00, 0x6e, 0x00, 0x64, 0x00, 0x65, 0x00, 0x78, 0x00, 0x0f, 0x00, 0xff, 0x65, 0x00,
            0x72, 0x00, 0x56, 0x00, 0x6f, 0x00, 0x6c, 0x00, 0x75, 0x00, 0x00, 0x00, 0x6d, 0x00, 0x65, 0x00,
            0x49, 0x4e, 0x44, 0x45, 0x58, 0x45, 0x7e, 0x31, 0x20, 0x20, 0x20, 0x20, 0x00, 0x68, 0x68, 0x85,
            0xa4, 0x56, 0xa4, 0x56, 0x00, 0x00, 0x69, 0x85, 0xa4, 0x56, 0x04, 0x00, 0x4c, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        uint8_t flash_0x9000_x000[] = {
            0x0c, 0x00, 0x00, 0x00, 0xe4, 0x88, 0x3c, 0x61, 0xa3, 0x58, 0x94, 0x6d, 0x00, 0x00, 0x00, 0x00
        };

        memset(mass_block, 0, sizeof(mass_block));
        memcpy(((uint8_t *)mass_block) + 0x01b0, flash_0x01b0, sizeof(flash_0x01b0));
        memcpy(((uint8_t *)mass_block) + 0x01f8, flash_0x01f8, sizeof(flash_0x01f8));
        memcpy(((uint8_t *)mass_block) + 0x1000, flash_0x1000, sizeof(flash_0x1000));
        memcpy(((uint8_t *)mass_block) + 0x2000, flash_0x2000_3000, sizeof(flash_0x2000_3000));
        memcpy(((uint8_t *)mass_block) + 0x3000, flash_0x2000_3000, sizeof(flash_0x2000_3000));
        memcpy(((uint8_t *)mass_block) + 0x4000, flash_0x4000, sizeof(flash_0x4000));
        memcpy(((uint8_t *)mass_block) + 0x8000, flash_0x8000, sizeof(flash_0x8000));
        memcpy(((uint8_t *)mass_block) + 0x9000, flash_0x9000_x000, sizeof(flash_0x9000_x000));
    }
    static struct usbd_interface intf0;
    usbd_desc_register(msc_ram_descriptor);
    usbd_add_interface(usbd_msc_init_intf(&intf0, MSC_OUT_EP, MSC_IN_EP));

    usbd_initialize();
}

void usbd_configure_done_callback(void)
{
    /* do nothing */
}

void usbd_msc_get_cap(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    *block_num = 2 * 1024 * 1024 / BLOCK_SIZE; //Pretend having so many buffer,not has actually.
    *block_size = BLOCK_SIZE;
}

int usbd_msc_sector_read(uint32_t sector, uint8_t *buffer, uint32_t length)
{
    if (sector < BLOCK_COUNT)
        memcpy(buffer, mass_block[sector], length);
    return 0;
}

#include "bflb_flash.h"
static void write_to_flash(uint8_t *buffer, uint32_t length, uint32_t target_addr, bool sync)
{
    static uint8_t flash_buf[BLOCK_SIZE];
    static uint32_t buf_idx = 0;
    static uint32_t flash_target_addr = 0;

    bool is_contiguous = target_addr == flash_target_addr + buf_idx;
    // not contiguous
    if (!is_contiguous) {
        if (buf_idx) {
            {
                // write flash_buf to flash_target_addr
                bflb_flash_erase(flash_target_addr, BLOCK_SIZE);
                bflb_flash_write(flash_target_addr, flash_buf, BLOCK_SIZE);
            }
            buf_idx = 0;
        }
        buf_idx = target_addr % BLOCK_SIZE;
        flash_target_addr = target_addr - buf_idx;
        if (buf_idx) {
            {
                // read flash_target_addr to flash_buf
                bflb_flash_read(flash_target_addr, flash_buf, BLOCK_SIZE);
            }
        }
    }

    uint32_t remained = (length + buf_idx > BLOCK_SIZE) ? (length + buf_idx - BLOCK_SIZE) : 0;

    memcpy(&flash_buf[buf_idx], buffer, length - remained);
    {
        // write flash_buf to flash_target_addr
        bflb_flash_erase(flash_target_addr, BLOCK_SIZE);
        bflb_flash_write(flash_target_addr, flash_buf, BLOCK_SIZE);
    }
    flash_target_addr += BLOCK_SIZE;
    buf_idx = 0;

    buf_idx = remained % BLOCK_SIZE;
    {
        // write buffer[length-remained : length-buf_idx] to flash_target_addr
        bflb_flash_erase(flash_target_addr, remained - buf_idx);
        bflb_flash_write(flash_target_addr, buffer + length - remained, remained - buf_idx);
    }
    flash_target_addr += remained / BLOCK_SIZE;

    if (buf_idx) {
        {
            // read flash_target_addr to flash_buf
            bflb_flash_read(flash_target_addr, flash_buf, BLOCK_SIZE);
        }
        memcpy(flash_buf, buffer + length - buf_idx, buf_idx);
    }

    if (sync) {
        if (buf_idx) {
            {
                // write flash_buf to flash_target_addr
                bflb_flash_erase(flash_target_addr, BLOCK_SIZE);
                bflb_flash_write(flash_target_addr, flash_buf, BLOCK_SIZE);
            }
            buf_idx = 0;
        }
    }
}

#include "uf2.h"

int usbd_msc_sector_write(uint32_t sector, uint8_t *buffer, uint32_t length)
{
    // printf("[MSC] write sector: %u, buffer: %p, length: %u\r\n", sector, buffer, length);
    if (uf2_is_uf2_block(((uf2_block_t *)buffer))) {
        for (uf2_block_t *puf2_blk = (uf2_block_t *)buffer; puf2_blk < (uf2_block_t *)(buffer + length); puf2_blk += 1) {
            if (UF2_FLAG_FAMILY_ID_PRESENT & puf2_blk->flags &&
                FAMILY_ID_MAIXPLAYU4 == puf2_blk->family_id) {
                // printf("[UF2] UF2HEADER:\r\n"
                //        "\tflags:\t%#8x\r\n"
                //        "\target_addr:\t%#8x\r\n"
                //        "\tpayload_size:\t%u\r\n"
                //        "\tblock_no:\t%u\r\n"
                //        "\tnum_blocks:\t%u\r\n"
                //        "\tfamily_id:\t%#8x\r\n"
                //        "\r\n",
                //        puf2_blk->flags, puf2_blk->target_addr, puf2_blk->payload_size,
                //        puf2_blk->block_no, puf2_blk->num_blocks, puf2_blk->family_id);
                printf("[flash] %04u/%04u%3s\r", puf2_blk->block_no + 1, puf2_blk->num_blocks,
                       puf2_blk->block_no + 1 == puf2_blk->num_blocks ?
                           " OK" :
                           (char *[]){ ".  ", ".. ", "..." }[puf2_blk->block_no / 5 % 3]);
                extern void ota_firmware_update_state(uint32_t blk_curr, uint32_t blk_total, uint32_t target_addr);
                ota_firmware_update_state(puf2_blk->block_no + 1, puf2_blk->num_blocks, 0x10000 /*FLASH_ADDR*/ + puf2_blk->target_addr);

                write_to_flash(puf2_blk->data, puf2_blk->payload_size,
                               0x10000 /*FLASH_ADDR*/ + puf2_blk->target_addr,
                               puf2_blk->block_no == puf2_blk->num_blocks - 1);

                extern bool need_reboot_after_upgrade;
                need_reboot_after_upgrade = puf2_blk->block_no == puf2_blk->num_blocks - 1;
            }
        }
        return 0;
    }

    if (sector < BLOCK_COUNT)
        memcpy(mass_block[sector], buffer, length);
    return 0;
}

///
///
/// 00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
/// *
/// 000001b0  00 00 00 00 00 00 00 00  3a 4d 82 02 00 00 00 00  |........:M......|
/// 000001c0  01 01 0e 00 41 ff 01 00  00 00 ff 01 00 00 00 00  |....A...........|
/// 000001d0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
/// *
/// 000001f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
/// 00000200  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
/// *
/// 00001000  eb 3c 90 4d 53 44 4f 53  35 2e 30 00 10 01 01 00  |.<.MSDOS5.0.....|
/// 00001010  02 00 02 ff 01 f8 01 00  01 00 01 00 01 00 00 00  |................|
/// 00001020  00 00 00 00 80 00 29 a6  cb 76 38 4e 4f 20 4e 41  |......)..v8NO NA|
/// 00001030  4d 45 20 20 20 20 46 41  54 31 32 20 20 20 33 c9  |ME    FAT12   3.|
/// 00001040  8e d1 bc f0 7b 8e d9 b8  00 20 8e c0 fc bd 00 7c  |....{.... .....||
/// 00001050  38 4e 24 7d 24 8b c1 99  e8 3c 01 72 1c 83 eb 3a  |8N$}$....<.r...:|
/// 00001060  66 a1 1c 7c 26 66 3b 07  26 8a 57 fc 75 06 80 ca  |f..|&f;.&.W.u...|
/// 00001070  02 88 56 02 80 c3 10 73  eb 33 c9 8a 46 10 98 f7  |..V....s.3..F...|
/// 00001080  66 16 03 46 1c 13 56 1e  03 46 0e 13 d1 8b 76 11  |f..F..V..F....v.|
/// 00001090  60 89 46 fc 89 56 fe b8  20 00 f7 e6 8b 5e 0b 03  |`.F..V.. ....^..|
/// 000010a0  c3 48 f7 f3 01 46 fc 11  4e fe 61 bf 00 00 e8 e6  |.H...F..N.a.....|
/// 000010b0  00 72 39 26 38 2d 74 17  60 b1 0b be a1 7d f3 a6  |.r9&8-t.`....}..|
/// 000010c0  61 74 32 4e 74 09 83 c7  20 3b fb 72 e6 eb dc a0  |at2Nt... ;.r....|
/// 000010d0  fb 7d b4 7d 8b f0 ac 98  40 74 0c 48 74 13 b4 0e  |.}.}....@t.Ht...|
/// 000010e0  bb 07 00 cd 10 eb ef a0  fd 7d eb e6 a0 fc 7d eb  |.........}....}.|
/// 000010f0  e1 cd 16 cd 19 26 8b 55  1a 52 b0 01 bb 00 00 e8  |.....&.U.R......|
/// 00001100  3b 00 72 e8 5b 8a 56 24  be 0b 7c 8b fc c7 46 f0  |;.r.[.V$..|...F.|
/// 00001110  3d 7d c7 46 f4 29 7d 8c  d9 89 4e f2 89 4e f6 c6  |=}.F.)}...N..N..|
/// 00001120  06 96 7d cb ea 03 00 00  20 0f b6 c8 66 8b 46 f8  |..}..... ...f.F.|
/// 00001130  66 03 46 1c 66 8b d0 66  c1 ea 10 eb 5e 0f b6 c8  |f.F.f..f....^...|
/// 00001140  4a 4a 8a 46 0d 32 e4 f7  e2 03 46 fc 13 56 fe eb  |JJ.F.2....F..V..|
/// 00001150  4a 52 50 06 53 6a 01 6a  10 91 8b 46 18 96 92 33  |JRP.Sj.j...F...3|
/// 00001160  d2 f7 f6 91 f7 f6 42 87  ca f7 76 1a 8a f2 8a e8  |......B...v.....|
/// 00001170  c0 cc 02 0a cc b8 01 02  80 7e 02 0e 75 04 b4 42  |.........~..u..B|
/// 00001180  8b f4 8a 56 24 cd 13 61  61 72 0b 40 75 01 42 03  |...V$..aar.@u.B.|
/// 00001190  5e 0b 49 75 06 f8 c3 41  bb 00 00 60 66 6a 00 eb  |^.Iu...A...`fj..|
/// 000011a0  b0 42 4f 4f 54 4d 47 52  20 20 20 20 0d 0a 52 65  |.BOOTMGR    ..Re|
/// 000011b0  6d 6f 76 65 20 64 69 73  6b 73 20 6f 72 20 6f 74  |move disks or ot|
/// 000011c0  68 65 72 20 6d 65 64 69  61 2e ff 0d 0a 44 69 73  |her media....Dis|
/// 000011d0  6b 20 65 72 72 6f 72 ff  0d 0a 50 72 65 73 73 20  |k error...Press |
/// 000011e0  61 6e 79 20 6b 65 79 20  74 6f 20 72 65 73 74 61  |any key to resta|
/// 000011f0  72 74 0d 0a 00 00 00 00  00 00 00 ac cb d8 55 aa  |rt............U.|
/// 00001200  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
/// *
/// 00002000  f8 ff ff ff ff ff ff 0f  00 00 00 00 00 00 00 00  |................|
/// 00002010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
/// *
/// 00003000  f8 ff ff ff ff ff ff 0f  00 00 00 00 00 00 00 00  |................|
/// 00003010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
/// *
/// 00004000  4d 50 55 34 2d 4f 54 41  20 20 20 08 00 00 00 00  |MPU4-OTA   .....|
/// 00004010  00 00 00 00 00 00 67 85  a4 56 00 00 00 00 00 00  |......g..V......|
/// 00004020  42 20 00 49 00 6e 00 66  00 6f 00 0f 00 72 72 00  |B .I.n.f.o...rr.|
/// 00004030  6d 00 61 00 74 00 69 00  6f 00 00 00 6e 00 00 00  |m.a.t.i.o...n...|
/// 00004040  01 53 00 79 00 73 00 74  00 65 00 0f 00 72 6d 00  |.S.y.s.t.e...rm.|
/// 00004050  20 00 56 00 6f 00 6c 00  75 00 00 00 6d 00 65 00  | .V.o.l.u...m.e.|
/// 00004060  53 59 53 54 45 4d 7e 31  20 20 20 16 00 c2 66 85  |SYSTEM~1   ...f.|
/// 00004070  a4 56 a4 56 00 00 67 85  a4 56 02 00 00 00 00 00  |.V.V..g..V......|
/// 00004080  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
/// *
/// 00008000  2e 20 20 20 20 20 20 20  20 20 20 10 00 c2 66 85  |.          ...f.|
/// 00008010  a4 56 a4 56 00 00 67 85  a4 56 02 00 00 00 00 00  |.V.V..g..V......|
/// 00008020  2e 2e 20 20 20 20 20 20  20 20 20 10 00 c2 66 85  |..         ...f.|
/// 00008030  a4 56 a4 56 00 00 67 85  a4 56 00 00 00 00 00 00  |.V.V..g..V......|
/// 00008040  42 74 00 00 00 ff ff ff  ff ff ff 0f 00 ce ff ff  |Bt..............|
/// 00008050  ff ff ff ff ff ff ff ff  ff ff 00 00 ff ff ff ff  |................|
/// 00008060  01 57 00 50 00 53 00 65  00 74 00 0f 00 ce 74 00  |.W.P.S.e.t....t.|
/// 00008070  69 00 6e 00 67 00 73 00  2e 00 00 00 64 00 61 00  |i.n.g.s.....d.a.|
/// 00008080  57 50 53 45 54 54 7e 31  44 41 54 20 00 c2 66 85  |WPSETT~1DAT ..f.|
/// 00008090  a4 56 a4 56 00 00 67 85  a4 56 03 00 0c 00 00 00  |.V.V..g..V......|
/// 000080a0  42 47 00 75 00 69 00 64  00 00 00 0f 00 ff ff ff  |BG.u.i.d........|
/// 000080b0  ff ff ff ff ff ff ff ff  ff ff 00 00 ff ff ff ff  |................|
/// 000080c0  01 49 00 6e 00 64 00 65  00 78 00 0f 00 ff 65 00  |.I.n.d.e.x....e.|
/// 000080d0  72 00 56 00 6f 00 6c 00  75 00 00 00 6d 00 65 00  |r.V.o.l.u...m.e.|
/// 000080e0  49 4e 44 45 58 45 7e 31  20 20 20 20 00 68 68 85  |INDEXE~1    .hh.|
/// 000080f0  a4 56 a4 56 00 00 69 85  a4 56 04 00 4c 00 00 00  |.V.V..i..V..L...|
/// 00008100  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
/// *
/// 00009000  0c 00 00 00 e4 88 3c 61  a3 58 94 6d 00 00 00 00  |......<a.X.m....|
/// 00009010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
/// *
/// 0000a000  0c 00 00 00 e4 88 3c 61  a3 58 94 6d 00 00 00 00  |......<a.X.m....|
/// 0000a010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
/// *
///
///