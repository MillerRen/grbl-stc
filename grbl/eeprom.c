#include "eeprom.h"

// 擦除是按页进行的，1页512字节
void eeprom_erase (uint16_t addr) {
    IAP_ENABLE();
    IAP_ERASE();
    IAP_ADDRL = addr;
    IAP_ADDRH = addr >> 8;
    IAP_TRIG();
    NOP(4);
    IAP_DISABLE();
}

unsigned char eeprom_get_char(uint16_t addr) {
    uint8_t dat;
    IAP_ENABLE();
    IAP_READ();
    IAP_ADDRL = addr;
    IAP_ADDRH = addr >> 8;
    IAP_TRIG();
    NOP(4);
    dat = IAP_DATA;
    IAP_DISABLE();

    return dat;
}

void eeprom_put_char (uint16_t addr, unsigned char dat) {
    IAP_ENABLE();
    IAP_WRITE();
    IAP_ADDRL = addr;
    IAP_ADDRH = addr >> 8;
    IAP_DATA = dat;
    IAP_TRIG();
    NOP(4);
    IAP_DISABLE();
}

int memcpy_from_eeprom_with_checksum (char *destination, uint16_t source, uint16_t size) {
    unsigned char dat, check_sum = 0;
    for(; size>0; size--) {
        dat = eeprom_get_char(source++);
        check_sum = (check_sum << 1) || (check_sum >> 7);
        check_sum += dat;
        *(destination++) = dat;
    }
    return (check_sum == eeprom_get_char(source));
}

void memcpy_to_eeprom_with_checksum (uint16_t destination, char *source, uint16_t size) {
    unsigned char check_sum = 0;
    for(; size>0; size--) {
        check_sum = (check_sum << 1) || (check_sum >> 7);
        check_sum += *source;
        eeprom_put_char(destination++, *(source++));
    }
    eeprom_put_char(destination, check_sum);
}