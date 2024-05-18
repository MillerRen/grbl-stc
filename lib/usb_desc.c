/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCAI.com ---------------------------------------------*/
/* --- Web: www.STCMCUDATA.com  ---------------------------------------*/
/* --- BBS: www.STCAIMCU.com  -----------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* ���Ҫ�ڳ�����ʹ�ô˴���,���ڳ�����ע��ʹ����STC�����ϼ�����        */
/*---------------------------------------------------------------------*/

#include "usb_desc.h"

char code DEVICEDESC[18] =
{
    0x12,                   //bLength(18);
    0x01,                   //bDescriptorType(Device);
    0x00,0x02,              //bcdUSB(2.00);
    0x02,                   //bDeviceClass(2:Communication Device Class);
    0x00,                   //bDeviceSubClass0);
    0x00,                   //bDeviceProtocol(0);
    0x40,                   //bMaxPacketSize0(64);
    0xbf,0x34,              //idVendor(34bf);
    0x02,0xff,              //idProduct(ff02);
    0x00,0x01,              //bcdDevice(1.00);
    0x01,                   //iManufacturer(1);
    0x02,                   //iProduct(2);
    0x00,                   //iSerialNumber(0);
    0x01,                   //bNumConfigurations(1);
};

char code CONFIGDESC[67] =
{
    0x09,                   //bLength(9);
    0x02,                   //bDescriptorType(Configuration);
    0x43,0x00,              //wTotalLength(67);
    0x02,                   //bNumInterfaces(2);
    0x01,                   //bConfigurationValue(1);
    0x00,                   //iConfiguration(0);
    0x80,                   //bmAttributes(BUSPower);
    0x32,                   //MaxPower(100mA);

    0x09,                   //bLength(9);
    0x04,                   //bDescriptorType(Interface);
    0x00,                   //bInterfaceNumber(0);
    0x00,                   //bAlternateSetting(0);
    0x01,                   //bNumEndpoints(1);
    0x02,                   //bInterfaceClass(Communication Interface Class);
    0x02,                   //bInterfaceSubClass(Abstract Control Model);
    0x01,                   //bInterfaceProtocol(Common AT commands);
    0x00,                   //iInterface(0);

    0x05,                   //bLength(5);
    0x24,                   //bDescriptorType(CS_INTERFACE);
    0x00,                   //bDescriptorSubtype(Header Functional Descriptor);
    0x10,0x01,              //bcdCDC(1.10);

    0x05,                   //bLength(5);
    0x24,                   //bDescriptorType(CS_INTERFACE);
    0x01,                   //bDescriptorSubtype(Call Management Functional Descriptor);
    0x00,                   //bmCapabilities(Device does not handles call management itself);
    0x01,                   //bDataInterface(1);

    0x04,                   //bLength(4);
    0x24,                   //bDescriptorType(CS_INTERFACE);
    0x02,                   //bDescriptorSubtype(Abstract Control Management Functional Descriptor);
    0x02,                   //bmCapabilities(Set/Get_Line_Coding,Serial_State,Set_Control_Line_State);

    0x05,                   //bLength(5);
    0x24,                   //bDescriptorType(CS_INTERFACE);
    0x06,                   //bDescriptorSubtype(Union Functional descriptor);
    0x00,                   //bMasterInterface(0);
    0x01,                   //bSlaveInterface0(1);

    0x07,                   //bLength(7);
    0x05,                   //bDescriptorType(Endpoint);
    0x82,                   //bEndpointAddress(EndPoint2 as IN);
    0x03,                   //bmAttributes(Interrupt);
    0x40,0x00,              //wMaxPacketSize(64);
    0xff,                   //bInterval(255ms);

    0x09,                   //bLength(9);
    0x04,                   //bDescriptorType(Interface);
    0x01,                   //bInterfaceNumber(1);
    0x00,                   //bAlternateSetting(0);
    0x02,                   //bNumEndpoints(2);
    0x0a,                   //bInterfaceClass(Data Interface Class);
    0x00,                   //bInterfaceSubClass(AData Interface Class SubClass Codes);
    0x00,                   //bInterfaceProtocol(USB SPEC);
    0x00,                   //iInterface(0);

    0x07,                   //bLength(7);
    0x05,                   //bDescriptorType(Endpoint);
    0x81,                   //bEndpointAddress(EndPoint1 as IN);
    0x02,                   //bmAttributes(Bulk);
    0x40,0x00,              //wMaxPacketSize(64);
    0x00,                   //bInterval(Ignored);

    0x07,                   //bLength(7);
    0x05,                   //bDescriptorType(Endpoint);
    0x01,                   //bEndpointAddress(EndPoint1 as OUT);
    0x02,                   //bmAttributes(Bulk);
    0x40,0x00,              //wMaxPacketSize(64);
    0x00,                   //bInterval(Ignored);
};

char code LANGIDDESC[4] =
{
    0x04,0x03,
    0x09,0x04,
};

char code MANUFACTDESC[8] =
{
    0x08,0x03,
    'S',0,
    'T',0,
    'C',0,
};

char code PRODUCTDESC[30] =
{
    0x1e,0x03,
    'S',0,
    'T',0,
    'C',0,
    ' ',0,
    'U',0,
    'S',0,
    'B',0,
    ' ',0,
    'S',0,
    'e',0,
    'r',0,
    'i',0,
    'a',0,
    'l',0,
};

char code PACKET0[2] = 
{
    0, 0,
};

char code PACKET1[2] = 
{
    1, 0,
};
