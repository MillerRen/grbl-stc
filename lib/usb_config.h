#ifndef __CONFIG_H__
#define __CONFIG_H__

#define EN_EP1IN
#define EN_EP2IN
//#define EN_EP3IN
//#define EN_EP4IN
//#define EN_EP5IN
#define EN_EP1OUT
//#define EN_EP2OUT
//#define EN_EP3OUT
//#define EN_EP4OUT
//#define EN_EP5OUT

#define EP0_SIZE                64

#ifdef EN_EP1IN
#define EP1IN_SIZE              64
#endif
#ifdef EN_EP2IN
#define EP2IN_SIZE              64
#endif
#ifdef EN_EP1OUT
#define EP1OUT_SIZE             64
#endif

#endif
