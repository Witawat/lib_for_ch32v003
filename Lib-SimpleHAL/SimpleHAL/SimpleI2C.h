/**
 * @file SimpleI2C.h
 * @brief Simple I2C Library สำหรับ CH32V003 แบบ Arduino-style
 * @version 1.0
 * @date 2025-12-12
 * 
 * @details
 * Library นี้ห่อหุ้ม Hardware I2C ให้ใช้งานง่ายแบบ Arduino Wire
 * รองรับ pin remapping และมีฟังก์ชัน helper สำหรับ register read/write
 * 
 * **คุณสมบัติ:**
 * - เริ่มต้นใช้งานได้ 1 บรรทัด
 * - รองรับ 2 pin configurations
 * - ฟังก์ชัน read/write แบบ Arduino Wire
 * - Helper functions สำหรับ register access
 * - Error handling ด้วย return status
 * 
 * @example
 * // ตัวอย่างการใช้งาน
 * #include "SimpleI2C.h"
 * 
 * int main(void) {
 *     // เริ่มต้น I2C ที่ 100kHz, ใช้ default pins
 *     I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
 *     
 *     // เขียนข้อมูลไปยัง EEPROM
 *     uint8_t data[] = {0x55, 0xAA};
 *     I2C_Write(0x50, data, 2);
 *     
 *     // อ่าน/เขียน register
 *     I2C_WriteReg(0x50, 0x00, 0x55);
 *     uint8_t val = I2C_ReadReg(0x50, 0x00);
 * }
 * 
 * @note ต้องต่อ pull-up resistor (4.7kΩ แนะนำ) ที่ SDA และ SCL
 */

#ifndef __SIMPLE_I2C_H
#define __SIMPLE_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x_i2c.h>
#include <stdint.h>

/* ========== Enumerations ========== */

/**
 * @brief ความเร็ว I2C ที่รองรับ
 */
typedef enum {
    I2C_100KHZ = 100000,     /**< 100 kHz (Standard mode) */
    I2C_400KHZ = 400000      /**< 400 kHz (Fast mode) */
} I2C_Speed;

/**
 * @brief การเลือก Pin Configuration
 * 
 * @details Pin mapping สำหรับ I2C1:
 * - I2C_PINS_DEFAULT: SCL=PC2, SDA=PC1
 * - I2C_PINS_REMAP:   SCL=PD0, SDA=PD1
 */
typedef enum {
    I2C_PINS_DEFAULT = 0,    /**< Default pins: SCL=PC2, SDA=PC1 */
    I2C_PINS_REMAP   = 1     /**< Remap: SCL=PD0, SDA=PD1 */
} I2C_PinConfig;

/**
 * @brief I2C Error Codes
 */
typedef enum {
    I2C_OK = 0,              /**< Success */
    I2C_ERROR_TIMEOUT,       /**< Timeout error */
    I2C_ERROR_NACK,          /**< NACK received */
    I2C_ERROR_BUS_BUSY       /**< Bus busy */
} I2C_Status;

/* ========== Constants ========== */

#define I2C_TIMEOUT_MS  100  /**< Timeout ในหน่วย milliseconds */

/* ========== Function Prototypes ========== */

/**
 * @brief เริ่มต้นการใช้งาน I2C
 * @param speed ความเร็ว I2C
 * @param pin_config การเลือก pin configuration
 * 
 * @note ฟังก์ชันนี้จะ:
 *       1. เปิด Clock สำหรับ I2C และ GPIO
 *       2. ตั้งค่า Pin remapping ตามที่เลือก
 *       3. ตั้งค่า I2C speed และ mode
 *       4. เปิดใช้งาน I2C
 * 
 * @example
 * I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
 */
void I2C_SimpleInit(I2C_Speed speed, I2C_PinConfig pin_config);

/**
 * @brief เขียนข้อมูลไปยัง I2C device
 * @param addr ที่อยู่ของ device (7-bit address)
 * @param data pointer ไปยังข้อมูลที่ต้องการส่ง
 * @param len จำนวน bytes ที่ต้องการส่ง
 * @return I2C_Status (I2C_OK = สำเร็จ)
 * 
 * @example
 * uint8_t data[] = {0x55, 0xAA};
 * if(I2C_Write(0x50, data, 2) == I2C_OK) {
 *     // Success
 * }
 */
I2C_Status I2C_Write(uint8_t addr, uint8_t* data, uint16_t len);

/**
 * @brief อ่านข้อมูลจาก I2C device
 * @param addr ที่อยู่ของ device (7-bit address)
 * @param data pointer ไปยัง buffer สำหรับเก็บข้อมูล
 * @param len จำนวน bytes ที่ต้องการอ่าน
 * @return I2C_Status (I2C_OK = สำเร็จ)
 * 
 * @example
 * uint8_t buffer[10];
 * if(I2C_Read(0x50, buffer, 10) == I2C_OK) {
 *     // Success
 * }
 */
I2C_Status I2C_Read(uint8_t addr, uint8_t* data, uint16_t len);

/**
 * @brief เขียนข้อมูลไปยัง register ของ device
 * @param addr ที่อยู่ของ device (7-bit address)
 * @param reg ที่อยู่ของ register
 * @param data ข้อมูลที่ต้องการเขียน
 * @return I2C_Status (I2C_OK = สำเร็จ)
 * 
 * @example
 * // เขียนค่า 0x55 ไปยัง register 0x00 ของ device 0x50
 * I2C_WriteReg(0x50, 0x00, 0x55);
 */
I2C_Status I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data);

/**
 * @brief อ่านข้อมูลจาก register ของ device
 * @param addr ที่อยู่ของ device (7-bit address)
 * @param reg ที่อยู่ของ register
 * @return ข้อมูลที่อ่านได้ (0xFF ถ้าเกิด error)
 * 
 * @example
 * uint8_t value = I2C_ReadReg(0x50, 0x00);
 */
uint8_t I2C_ReadReg(uint8_t addr, uint8_t reg);

/**
 * @brief เขียนหลาย bytes ไปยัง register
 * @param addr ที่อยู่ของ device (7-bit address)
 * @param reg ที่อยู่ของ register เริ่มต้น
 * @param data pointer ไปยังข้อมูล
 * @param len จำนวน bytes
 * @return I2C_Status (I2C_OK = สำเร็จ)
 * 
 * @example
 * uint8_t data[] = {0x11, 0x22, 0x33};
 * I2C_WriteRegMulti(0x50, 0x00, data, 3);
 */
I2C_Status I2C_WriteRegMulti(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len);

/**
 * @brief อ่านหลาย bytes จาก register
 * @param addr ที่อยู่ของ device (7-bit address)
 * @param reg ที่อยู่ของ register เริ่มต้น
 * @param data pointer ไปยัง buffer
 * @param len จำนวน bytes
 * @return I2C_Status (I2C_OK = สำเร็จ)
 * 
 * @example
 * uint8_t buffer[3];
 * I2C_ReadRegMulti(0x50, 0x00, buffer, 3);
 */
I2C_Status I2C_ReadRegMulti(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len);

/**
 * @brief สแกนหา I2C devices บน bus
 * @param found_devices pointer ไปยัง array สำหรับเก็บ addresses ที่พบ
 * @param max_devices ขนาดสูงสุดของ array
 * @return จำนวน devices ที่พบ
 * 
 * @example
 * uint8_t devices[10];
 * uint8_t count = I2C_Scan(devices, 10);
 * for(int i = 0; i < count; i++) {
 *     printf("Found: 0x%02X\n", devices[i]);
 * }
 */
uint8_t I2C_Scan(uint8_t* found_devices, uint8_t max_devices);

/**
 * @brief ตรวจสอบว่า device ตอบสนองหรือไม่
 * @param addr ที่อยู่ของ device (7-bit address)
 * @return 1 = device ตอบสนอง, 0 = ไม่ตอบสนอง
 * 
 * @example
 * if(I2C_IsDeviceReady(0x50)) {
 *     // Device is ready
 * }
 */
uint8_t I2C_IsDeviceReady(uint8_t addr);

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_I2C_H
