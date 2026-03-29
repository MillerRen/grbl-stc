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

// 采用 XDATA 的 512 字节（1 个扇区）静态缓冲池，确保读改写安全，极大保护 Flash 寿命并防止同扇区数据覆写
static unsigned char xdata sector_buf[512];

void eeprom_put_char (uint16_t addr, unsigned char dat) {
    uint16_t sector_base = addr & 0xFE00; // 获取512字节扇区对齐基址
    uint16_t offset = addr & 0x01FF;      // 获取扇区内偏移
    uint16_t i;

    // 1. 将整个扇区拉入 RAM
    for (i = 0; i < 512; i++) {
        sector_buf[i] = eeprom_get_char(sector_base + i);
    }
    
    // 检查是否需要更新，避免徒劳写擦除
    if (sector_buf[offset] == dat) {
        return; 
    }
    
    // 2. 在 RAM 中修改单字节
    sector_buf[offset] = dat;
    
    // 3. 擦除目标片区
    eeprom_erase(sector_base);
    
    // 4. 重写整个 512 字节的数据入片区
    IAP_ENABLE();
    for (i = 0; i < 512; i++) {
        if (sector_buf[i] != 0xFF) { // 优化：按 STC IAP 手册，只写非 FF 可以提高寿命
            IAP_WRITE();
            IAP_ADDRL = (sector_base + i) & 0xFF; // 地址递增
            IAP_ADDRH = (sector_base + i) >> 8;
            IAP_DATA = sector_buf[i];
            IAP_TRIG();
            NOP(4);
        }
    }
    IAP_DISABLE();
}

int memcpy_from_eeprom_with_checksum (char *destination, uint16_t source, uint16_t size) {
    unsigned char dat, check_sum = 0;
    for(; size>0; size--) {
        dat = eeprom_get_char(source++);
        check_sum = (check_sum << 1) | (check_sum >> 7);
        check_sum += dat;
        *(destination++) = dat;
    }
    return (check_sum == eeprom_get_char(source));
}

void memcpy_to_eeprom_with_checksum (uint16_t destination, char *source, uint16_t size) {
    unsigned char check_sum = 0;
    uint16_t sector_base = destination & 0xFE00;
    uint16_t offset = destination & 0x01FF;
    uint16_t i;

    // 1. 获取整个目标扇区的快照
    for (i = 0; i < 512; i++) {
        sector_buf[i] = eeprom_get_char(sector_base + i);
    }
    
    // 2. 将传入数组和校验和覆盖到快照内存中对应的映射处
    for (i = 0; i < size; i++) {
        check_sum = (check_sum << 1) | (check_sum >> 7);
        check_sum += source[i];
        sector_buf[offset + i] = source[i];
    }
    sector_buf[offset + size] = check_sum;

    // 3. 抹除目标物理扇区
    eeprom_erase(sector_base);
    
    // 4. 将拼接完成的新扇区影像整块写回
    IAP_ENABLE();
    for (i = 0; i < 512; i++) {
        if (sector_buf[i] != 0xFF) {
            IAP_WRITE();
            IAP_ADDRL = (sector_base + i) & 0xFF;
            IAP_ADDRH = (sector_base + i) >> 8;
            IAP_DATA = sector_buf[i];
            IAP_TRIG();
            NOP(4);
        }
    }
    IAP_DISABLE();
}