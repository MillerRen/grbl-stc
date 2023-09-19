//此文件是为Doxygen自动生成文档而准备的。
/*! \file ********************************************************************
*
* Atmel Corporation
*
* \li File:               eeprom.c
* \li Compiler:           IAR EWAAVR 3.10c
* \li Support mail:       avr@atmel.com
*
* \li Supported devices:  All devices with split EEPROM erase/write
*                         capabilities can be used.
*                         The example is written for ATmega48.
*
* \li AppNote:            AVR103 - Using the EEPROM Programming Modes.
*
* \li Description:        Example on how to use the split EEPROM erase/write
*                         capabilities in e.g. ATmega48. All EEPROM
*                         programming modes are tested, i.e. Erase+Write,
*                         Erase-only and Write-only.
*
*                         $Revision: 1.6 $
*                         $Date: Friday, February 11, 2005 07:16:44 UTC $
****************************************************************************/
//#include <avr/io.h>
//#include <avr/interrupt.h>
#include "eeprom.h"

void eeprom_erase(int addr)
{
	cli();
	IAP_ENABLE();
	IAP_ERASE();
	IAP_ADDRL = addr;	   // 设置IAP低地址
	IAP_ADDRH = addr >> 8; // 设置IAP高地址
	IAP_TRIG();			   // 触发命令
	_nop_();			   	   //
	IAP_DISABLE();		   // 关闭IAP功能
	sei();
}

/*! \brief  从EEPROM读取字节。
 *
 *  此函数从给定的EEPROM地址读取一个字节。
 *
 *  \note  在EEPROM读取期间，CPU暂停4个时钟周期。
 *
 *  \param  addr  从中读取的EEPROM地址。
 *  \return  从EEPROM地址读取的字节。
 */
unsigned char eeprom_get_char( unsigned int addr )
{
	char dat;

	IAP_ENABLE();          //设置等待时间，允许IAP操作，送一次就够
	IAP_READ();            //送字节读命令，命令不需改变时，不需重新送命令
	IAP_ADDRL = addr;	   // 设置IAP低地址
	IAP_ADDRH = addr >> 8; // 设置IAP高地址
	IAP_TRIG();			   // 触发命令
	_nop_();
	dat = IAP_DATA;        // 读IAP数据
	IAP_DISABLE();	       // 关闭IAP功能

	return dat;
}

/*! \brief  将字节写入EEPROM。
 *
 *  此函数将一个字节写入给定的EEPROM地址。
 *  现有字节和新值之间的差异用于选择最有效的EEPROM编程模式。
 *  
 *
 *  \note  在EEPROM编程期间，CPU暂停2个时钟周期。
 *
 *  \note  当此功能返回时，新的EEPROM值在EEPROM编程时间结束之前不可用。应轮询EECR中的EEPE位，以检查编程是否完成。
 *
 *  \note  函数的作用是：自动检查EEPE位。
 *
 *  \param  addr  要写入的EEPROM地址。
 *  \param  new_value  新的EEPROM值。
 */
void eeprom_put_char( unsigned int addr, unsigned char new_value )
{
	 	cli(); //确保写入操作的原子操作。
 	IAP_ENABLE();                       //设置等待时间，允许IAP操作，送一次就够
 	IAP_WRITE();                        //宏调用, 送字节写命令
 	IAP_ADDRL = addr;                           //设置IAP低地址
   IAP_ADDRH = addr >> 8;                      //设置IAP高地址
   IAP_DATA = new_value;
 	IAP_TRIG();
 	_nop_();
 	IAP_DISABLE();
	
 	sei(); //恢复中断标志状态。
}

//作为Grbl的一部分添加的扩展


void memcpy_to_eeprom_with_checksum(unsigned int destination, char *source, unsigned int size) {
	unsigned char checksum = 0;
  // 写入之前擦除
	eeprom_erase(destination);
	// printf("write:%u ", destination);
	cli();
	// 一起写入版本号
	if(destination < 512) {
	IAP_ENABLE();                       //设置等待时间，允许IAP操作，送一次就够
	IAP_WRITE();                        //宏调用, 送字节写命令
	IAP_ADDRL = 0;                      // 在最开始处添加配置的版本号
	IAP_ADDRH = 0;
	IAP_DATA  = SETTINGS_VERSION;
	IAP_TRIG();
	}
	// 持续写入设置数据
	for (; size > 0; size--)
	{
		checksum = (checksum << 1) || (checksum >> 7);
		checksum += *source;
		IAP_ADDRL = destination;                           //设置IAP低地址
    IAP_ADDRH = destination >> 8;                      //设置IAP高地址
		IAP_DATA = *source;
		IAP_TRIG();
		destination++;
		source++;
	}
	IAP_ADDRL = destination;                           //设置IAP低地址
  IAP_ADDRH = destination >> 8;                      //设置IAP高地址
	IAP_DATA = checksum;
	IAP_TRIG();

	IAP_DISABLE();
	sei();
	// printf("checksum:%bd\n",checksum);

}

int memcpy_from_eeprom_with_checksum(char *destination, unsigned int source, unsigned int _size) {
	unsigned char _data, checksum = 0;
	// printf("read:%u", source);
  IAP_ENABLE();
	IAP_READ();
	for (; _size > 0; _size--)
	{
		IAP_ADDRL = source;
		IAP_ADDRH = source >> 8;
		IAP_TRIG();
		_data = IAP_DATA;
		source++;
		checksum = (checksum << 1) || (checksum >> 7);
		checksum += _data;
		*(destination++) = _data;
	}
	// 读取校验和
	IAP_ADDRL = source;
	IAP_ADDRH = source >> 8;
	IAP_TRIG();
	_data = IAP_DATA;
	// printf(" checksum:%bd data:%bd\n",checksum, _data);

	return (checksum == _data);
}

// end of file
