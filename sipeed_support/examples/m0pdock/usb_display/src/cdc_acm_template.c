#include "usbd_core.h"

/*!< endpoint address */
#define CDC_IN_EP          0x81
#define CDC_OUT_EP         0x02

#define USBD_VID           0x359F
#define USBD_PID           0x4000
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

/*!< config descriptor size */
#define USB_CONFIG_SIZE    (9 + (9 + 7 + 7))

/*!< global descriptor */
static const uint8_t cdc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0100, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),

    USB_INTERFACE_DESCRIPTOR_INIT(0x00, 0x00, 0x02, 0xff, 0xff, 0xff, 0x02),
    USB_ENDPOINT_DESCRIPTOR_INIT(CDC_IN_EP, USB_ENDPOINT_TYPE_BULK, 0x0200, 0x01),
    USB_ENDPOINT_DESCRIPTOR_INIT(CDC_OUT_EP, USB_ENDPOINT_TYPE_BULK, 0x0200, 0x01),
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
    'C', 0x00,                  /* wcChar10 */
    'D', 0x00,                  /* wcChar11 */
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
    0x02,
    0x02,
    0x01,
    0x40,
    0x01,
    0x00,
#endif
    0x00
};

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif

static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[2][CDC_MAX_MPS * 40];

static uint8_t ep_rx_flag = 0;
static bool ep_tx_busy_flag = false;

void usbd_configure_done_callback(void)
{
    /* setup first out ep read transfer */
    usbd_ep_start_read(CDC_OUT_EP, read_buffer[!ep_rx_flag], sizeof(read_buffer[0]));
}

#include "compiler/compiler_ld.h"
#include "lcd.h"
// clang-format off
static uint8_t screen_idx = 0;
static uint32_t pixel_idx = 0;
static uint8_t screen_buf[2][320*2*320] __attribute__((section(".psram_data"))) ;
// clang-format on

void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes)
{
    ep_rx_flag ^= 1;
    uint8_t *ptr = read_buffer[ep_rx_flag];
    USB_LOG_RAW("actual out len:%d\r\n", nbytes);
    if (nbytes == 2) {
        if (0x32ff == *(uint16_t *)ptr) {
            pixel_idx = 0;
            nbytes = 0;
            USB_LOG_RAW("clear screen\r\n");
        }
    }

    if (nbytes) {
        uint32_t remained = sizeof(screen_buf[0]) - pixel_idx;
        if (nbytes > remained) {
            nbytes = remained;
        }
        memcpy(&screen_buf[!screen_idx][pixel_idx], ptr, nbytes);
        pixel_idx += nbytes;
        USB_LOG_RAW("pixel_idx/size:%d/%u\r\n", pixel_idx, sizeof(screen_buf[0]));
        if (pixel_idx == sizeof(screen_buf[0])) {
            pixel_idx = 0;
            screen_idx ^= 1;
            USB_LOG_RAW("update screen\r\n");
        }
    }

    // for (int i = 0; i < nbytes; i++) {
    //     printf("%02x ", ptr[i]);
    // }
    // printf("\r\n");
    /* setup next out ep read transfer */
    // usbd_ep_start_write(CDC_IN_EP, ptr, nbytes);
    usbd_ep_start_read(CDC_OUT_EP, read_buffer[!ep_rx_flag], sizeof(read_buffer[0]));
}

void usbd_cdc_acm_bulk_in(uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual in len:%d\r\n", nbytes);

    if ((nbytes % CDC_MAX_MPS) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write(CDC_IN_EP, NULL, 0);
    } else {
        ep_tx_busy_flag = false;
    }
}

/*!< endpoint call back */
struct usbd_endpoint cdc_out_ep = {
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

struct usbd_endpoint cdc_in_ep = {
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in
};

struct usbd_interface intf0;
struct usbd_interface intf1;

static void notify_handler(uint8_t event, void *arg)
{
    switch (event) {
        default:
            break;
    }
}

static int vendor_request_handler(struct usb_setup_packet *pSetup, uint8_t **data, uint32_t *len)
{
    USB_LOG_DBG("ftdi_vendor_request 0x%02x(0x%02x), value 0x%04x, index 0x%04x, length 0x%04x\r\n", pSetup->bRequest, pSetup->bmRequestType, pSetup->wValue, pSetup->wIndex, pSetup->wLength);
    switch (pSetup->bRequest) {
        default:
            return -1;
    }
    return 0;
}

/* function ------------------------------------------------------------------*/
void cdc_acm_init(void)
{
    usbd_desc_register(cdc_descriptor);
    intf0.vendor_handler = vendor_request_handler;
    intf0.notify_handler = notify_handler;
    usbd_add_interface(&intf0);
    usbd_add_endpoint(&cdc_out_ep);
    usbd_add_endpoint(&cdc_in_ep);
    usbd_initialize();
}

void lcd_display_loop(void)
{
    if (!lcd_draw_is_busy()) {
        lcd_draw_picture_nonblocking(0, 0, 320 - 1, 320 - 1, screen_buf[screen_idx]);
    }
}