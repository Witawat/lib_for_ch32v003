/**
 * @file main.c
 * @brief ตัวอย่างการใช้ Software I2C ทดสอบกับ Logic Analyzer
 */

#include "SimpleGPIO.h"
#include "SimpleDelay.h"
#include "SimpleI2C_Soft.h"

int main(void) {
    SystemInit();
    
    // ตั้งค่า Software I2C ใช้ SCL=PC2, SDA=PC1 
    // ใช้ความเร็วมาตรฐาน 100kHz
    I2C_Soft_Init(PC2, PC1, I2C_SOFT_100KHZ);
    
    while(1) {
        // ข้อมูลที่จะส่ง
        uint8_t data[] = {0x55, 0xAA, 0xBE, 0xEF};
        
        // ส่งไปยัง Address 0x50
        // พารามิเตอร์สุดท้าย = 1 (ignore_ack) เพื่อให้ส่งได้แม้ไม่มีอุปกรณ์ต่ออยู่
        I2C_Soft_Write(0x50, data, 4, 1);
        
        // หน่วงเวลา 1 วินาทีก่อนส่งใหม่
        Delay_Ms(1000);
    }
}
