#ifndef __USB_H__
#define __USB_H__

#include <stc8h.H>
#include "stdint.h"
#include "stdbool.h"
#include "usb_config.h"

#define DESC_DEVICE             0x01
#define DESC_CONFIGURATION      0x02
#define DESC_STRING             0x03

#define IN_DIRECT               0x80                //device to host
#define OUT_DIRECT              0x00                //host to device
#define DIRECT_MASK             0x80

#define STANDARD_REQUEST        0x00
#define CLASS_REQUEST           0x20
#define VENDOR_REQUEST          0x40
#define REQUEST_MASK            0x60

#define DEVICE_RECIPIENT        0x00
#define INTERFACE_RECIPIENT     0x01
#define ENDPOINT_RECIPIENT      0x02
#define RECIPIENT_MASK          0x0f

#define DEVICE_REMOTE_WAKEUP    0x01
#define ENDPOINT_HALT           0x00

#define EP1_IN                  0x81
#define EP1_OUT                 0x01
#define EP2_IN                  0x82
#define EP2_OUT                 0x02
#define EP3_IN                  0x83
#define EP3_OUT                 0x03
#define EP4_IN                  0x84
#define EP4_OUT                 0x04
#define EP5_IN                  0x85
#define EP5_OUT                 0x05

#define DEVSTATE_ATTACHED       0
#define DEVSTATE_POWERED        1
#define DEVSTATE_DEFAULT        2
#define DEVSTATE_ADDRESS        3
#define DEVSTATE_CONFIGURED     4
#define DEVSTATE_SUSPENDED      5

#define EPSTATE_IDLE            0
#define EPSTATE_HALT            1
#define EPSTATE_STATUS          2
#define EPSTATE_DATAIN          3
#define EPSTATE_DATAOUT         4
#define EPSTATE_STALL           5


typedef struct
{
    uint8_t    bmRequestType;
    uint8_t    bRequest;
    uint8_t    wValueL;
    uint8_t    wValueH;
    uint8_t    wIndexL;
    uint8_t    wIndexH;
    uint16_t    wLength;
}SETUP;

typedef struct
{
    uint8_t    bState;
    uint16_t    wSize;
    uint8_t    *pData;
}EPSTATE;

void usb_init();
uint8_t usb_read_reg(uint8_t addr);
void usb_write_reg(uint8_t addr, uint8_t dat);
uint8_t usb_read_fifo(uint8_t fifo, uint8_t *pdat);
void usb_write_fifo(uint8_t fifo, uint8_t *pdat, uint8_t cnt);

void usb_setup_stall();
void usb_setup_in();
void usb_setup_out();
void usb_setup_status();

void usb_ctrl_in();
void usb_ctrl_out();
void usb_bulk_intr_in(uint8_t *pData, uint8_t bSize, uint8_t ep);
uint8_t usb_bulk_intr_out(uint8_t *pData, uint8_t ep);

void usb_resume();
void usb_reset();
void usb_suspend();
void usb_setup();

extern uint8_t DeviceState;
extern SETUP Setup;
extern EPSTATE Ep0State;
extern uint8_t InEpState;
extern uint8_t OutEpState;

#endif
