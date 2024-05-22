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
#include "util.h"

uint8_t DeviceState = DEVSTATE_DEFAULT;
SETUP Setup;
EPSTATE Ep0State;
uint8_t InEpState = 0x00;
uint8_t OutEpState = 0x00;


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

    ret = cnt = usb_read_reg(COUNT0); // COUNT0和OUTCOUNT1地址相同
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
        usb_write_reg(CSR0, csr & ~STSTL);  //?????д 0 ??????λ
        Ep0State.bState = EPSTATE_IDLE;
    }
    if (csr & SUEND)
    {
        usb_write_reg(CSR0, csr | SSUEND);  //?? SSUEND д 1 ??? SUEND ???λ
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