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
/* ??????????????????,?????????????????STC???????????        */
/*---------------------------------------------------------------------*/

#include "usb.h"
#include "usb_req_std.h"
#include "usb_req_class.h"
#include "usb_req_vendor.h"

uint8_t DeviceState;
SETUP Setup;
EPSTATE Ep0State;
uint8_t InEpState;
uint8_t OutEpState;

uint16_t reverse2(uint16_t w)
{
    uint16_t ret;
    
    ((uint8_t *)&ret)[0] = ((uint8_t *)&w)[1];
    ((uint8_t *)&ret)[1] = ((uint8_t *)&w)[0];

    return ret;
}

#ifdef EN_EP1IN
void usb_in_ep1()
{
    uint8_t csr;

    usb_write_reg(INDEX, 1);
    csr = usb_read_reg(INCSR1);
    if (csr & INSTSTL)
    {
        usb_write_reg(INCSR1, INCLRDT);
    }
    if (csr & INUNDRUN)
    {
      usb_write_reg(INCSR1, 0);
    }

}
#endif
#ifdef EN_EP2IN
void usb_in_ep2() // �˵�2����AT����
{
    unsigned char csr;

    usb_write_reg(INDEX, 2);
    csr = usb_read_reg(INCSR1);
    if (csr & INSTSTL)
    {
        usb_write_reg(INCSR1, INCLRDT);
    }
    if (csr & INUNDRUN)
    {
        usb_write_reg(INCSR1, 0);
    }
}
#endif

#ifdef EN_EP1OUT
void usb_out_ep1() // �������ݴ���
{
    uint8_t csr;
    uint8_t cnt;

    usb_write_reg(INDEX, 1);
    csr = usb_read_reg(OUTCSR1);
    if (csr & OUTSTSTL)
    {
        usb_write_reg(OUTCSR1, OUTCLRDT);
    }
    if (csr & OUTOPRDY)
    {
        cnt = usb_read_reg(OUTCOUNT1);
        while (cnt--)
        {
          usb_out_callback(usb_read_reg(FIFO1));
        }
       usb_write_reg(OUTCSR1, 0);
    }
}
#endif

void usb_isr() interrupt USB_VECTOR
{
    uint8_t intrusb;
    uint8_t intrin;
    uint8_t introut;

    intrusb = usb_read_reg(INTRUSB);
    intrin = usb_read_reg(INTRIN1);
    introut = usb_read_reg(INTROUT1);

    if (intrusb & RSUIF) usb_resume();
    if (intrusb & RSTIF) usb_reset();

    if (intrin & EP0IF) usb_setup();

#ifdef EN_EP1IN
    if (intrin & EP1INIF) usb_in_ep1();
#endif
#ifdef EN_EP2IN
    if (intrin & EP2INIF) usb_in_ep2();
#endif

#ifdef EN_EP1OUT
    if (introut & EP1OUTIF) usb_out_ep1();
#endif

    if (intrusb & SUSIF) usb_suspend();
}


void usb_init()
{
    DeviceState = DEVSTATE_DEFAULT;
    Ep0State.bState = EPSTATE_IDLE;
    InEpState = 0x00;
    OutEpState = 0x00;
    LineCoding.dwDTERate = 0x00c20100;  //115200
    LineCoding.bCharFormat = 0;
    LineCoding.bParityType = 0;
    LineCoding.bDataBits = 8;

    P3M0 &= ~0x03;
    P3M1 |= 0x03;
    
    IRC48MCR = 0x80;
    while (!(IRC48MCR & 0x01));
    
    USBCLK = 0x00;
    USBCON = 0x90;

    usb_write_reg(FADDR, 0x00);
    usb_write_reg(POWER, 0x08);
    usb_write_reg(INTRIN1E, 0x3);  // USB�˵�IN�ж�ʹ��
    usb_write_reg(INTROUT1E, 0x3); // USB�˵�OUT�ж�ʹ��
    usb_write_reg(INTRUSBE, 0x07);
    usb_write_reg(POWER, 0x00);

    IE2 |= 0x80;    //EUSB = 1;
}


uint8_t usb_read_reg(uint8_t addr)
{
	uint8_t dat;

	while (USBADR & 0x80);
	USBADR = addr | 0x80;
	while (USBADR & 0x80);
	dat = USBDAT;

	return dat;
}

void usb_write_reg(uint8_t addr, uint8_t dat)
{
	while (USBADR & 0x80);
	USBADR = addr & 0x7f;
	USBDAT = dat;
}

uint8_t usb_read_fifo(uint8_t fifo, uint8_t *pdat)
{
    uint8_t cnt;
    uint8_t ret;

    ret = cnt = usb_read_reg(COUNT0); // COUNT0��OUTCOUNT1��ַ��ͬ
    while (cnt--)
    {
    	*pdat++ = usb_read_reg(fifo);
    }

    return ret;
}

void usb_write_fifo(uint8_t fifo, uint8_t *pdat, uint8_t cnt)
{
	while (cnt--)
	{
        usb_write_reg(fifo, *pdat++);
    }
}



void usb_resume()
{
}

void usb_reset()
{
    usb_write_reg(FADDR, 0x00);
    DeviceState = DEVSTATE_DEFAULT;
    Ep0State.bState = EPSTATE_IDLE;

#ifdef EN_EP1IN
    usb_write_reg(INDEX, 1);
    usb_write_reg(INCSR1, INCLRDT | INFLUSH);
#endif
#ifdef EN_EP2IN
    usb_write_reg(INDEX, 2);
    usb_write_reg(INCSR1, INCLRDT | INFLUSH);
#endif
#ifdef EN_EP1OUT
    usb_write_reg(INDEX, 1);
    usb_write_reg(OUTCSR1, OUTCLRDT | OUTFLUSH);
#endif
    usb_write_reg(INDEX, 0);
}

void usb_suspend()
{
}

void usb_setup()
{
    uint8_t csr;

    usb_write_reg(INDEX, 0);
    csr = usb_read_reg(CSR0);

    if (csr & STSTL)
    {
        usb_write_reg(CSR0, csr & ~STSTL);  //?????�� 0 ??????��
        Ep0State.bState = EPSTATE_IDLE;
    }
    if (csr & SUEND)
    {
        usb_write_reg(CSR0, csr | SSUEND);  //?? SSUEND �� 1 ??? SUEND ???��
    }

    switch (Ep0State.bState)
    {
    case EPSTATE_IDLE:
        if (csr & OPRDY)
        {
            usb_read_fifo(FIFO0, (uint8_t *)&Setup);
            Setup.wLength = reverse2(Setup.wLength);
            switch (Setup.bmRequestType & REQUEST_MASK)
            {
            case STANDARD_REQUEST:
                usb_req_std();
                break;
            case CLASS_REQUEST:
                usb_req_class();
                break;
            case VENDOR_REQUEST:
                usb_req_vendor();
                break;
            default:
                usb_setup_stall();
                return;
            }
        }
        break;
    case EPSTATE_DATAIN:
        usb_ctrl_in();
        break;
    case EPSTATE_DATAOUT:
        usb_ctrl_out();
        break;
    }
}

void usb_setup_stall()
{
    Ep0State.bState = EPSTATE_STALL;
    usb_write_reg(CSR0, SOPRDY | SDSTL);
}

void usb_setup_in()
{
    Ep0State.bState = EPSTATE_DATAIN;
    usb_write_reg(CSR0, SOPRDY);
    usb_ctrl_in();
}

void usb_setup_out()
{
    Ep0State.bState = EPSTATE_DATAOUT;
    usb_write_reg(CSR0, SOPRDY);
}

void usb_setup_status()
{
    Ep0State.bState = EPSTATE_IDLE;
    usb_write_reg(CSR0, SOPRDY | DATEND);
}

void usb_ctrl_in()
{
    uint8_t csr;
    uint8_t cnt;

    usb_write_reg(INDEX, 0);
    csr = usb_read_reg(CSR0);
    if (csr & IPRDY) return;

    cnt = Ep0State.wSize > EP0_SIZE ? EP0_SIZE : Ep0State.wSize;
    usb_write_fifo(FIFO0, Ep0State.pData, cnt);
    Ep0State.wSize -= cnt;
    Ep0State.pData += cnt;
    if (Ep0State.wSize == 0)
    {
        usb_write_reg(CSR0, IPRDY | DATEND);
        Ep0State.bState = EPSTATE_IDLE;
    }
    else
    {
        usb_write_reg(CSR0, IPRDY);
    }
}

void usb_ctrl_out()
{
    uint8_t csr;
    uint8_t cnt;

    usb_write_reg(INDEX, 0);
    csr = usb_read_reg(CSR0);
    if (!(csr & OPRDY)) return;

    cnt = usb_read_fifo(FIFO0, Ep0State.pData);
    Ep0State.wSize -= cnt;
    Ep0State.pData += cnt;
    if (Ep0State.wSize == 0)
    {
        usb_write_reg(CSR0, SOPRDY | DATEND);
        Ep0State.bState = EPSTATE_IDLE;
    }
    else
    {
        usb_write_reg(CSR0, SOPRDY);
    }

    if (Setup.bRequest == SET_LINE_CODING)
    {
        LineCoding.bCharFormat = 0;
        LineCoding.bDataBits = 8;
    }
}

void usb_bulk_intr_in(uint8_t *pData, uint8_t bSize, uint8_t ep)
{
    usb_write_fifo((uint8_t)(FIFO0 + ep), pData, bSize);
    usb_write_reg(INCSR1, INIPRDY);
}

uint8_t usb_bulk_intr_out(uint8_t *pData, uint8_t ep)
{
    uint8_t cnt;

    cnt = usb_read_fifo((uint8_t)(FIFO0 + ep), pData);
    usb_write_reg(OUTCSR1, 0);

    return cnt;
}

bool usb_bulk_intr_in_busy()
{
    return (usb_read_reg(INCSR1) & INIPRDY);
}

bool usb_bulk_intr_out_ready()
{
    return (usb_read_reg(OUTCSR1) & OUTOPRDY);
}