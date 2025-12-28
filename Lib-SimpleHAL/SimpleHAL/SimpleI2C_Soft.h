/**
 * @file SimpleI2C_Soft.h
 * @brief Software I2C (Bit-bang) Library for CH32V003
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * Library นี้ใช้เทคนิค Bit-bang เพื่อสร้างสัญญาณ I2C ผ่าน GPIO ทั่วไป
 * ออกแบบมาเพื่อใช้ทดสอบกับ Logic Analyzer โดยเฉพาะ (สามารถข้ามการเช็ค ACK ได้)
 */

#ifndef __SIMPLE_I2C_SOFT_H
#define __SIMPLE_I2C_SOFT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "SimpleGPIO.h"

/**
 * @brief ความเร็ว Software I2C
 */
typedef enum {
    I2C_SOFT_100KHZ = 100000,
    I2C_SOFT_400KHZ = 400000
} I2C_Soft_Speed;

/**
 * @brief สถานะการทำงาน
 */
typedef enum {
    I2C_SOFT_OK = 0,
    I2C_SOFT_ERROR_NACK = 1,
    I2C_SOFT_ERROR_BUSY = 2
} I2C_Soft_Status;

/**
 * @brief เริ่มต้น Software I2C
 * @param scl_pin Pin สำหรับ SCL
 * @param sda_pin Pin สำหรับ SDA
 * @param speed ความเร็วมาตรฐาน (I2C_SOFT_100KHZ, I2C_SOFT_400KHZ)
 */
void I2C_Soft_Init(uint8_t scl_pin, uint8_t sda_pin, I2C_Soft_Speed speed);

/**
 * @brief เขียนข้อมูลไปยัง Software I2C
 * @param addr Address 7-bit
 * @param data Array ข้อมูล
 * @param len ความยาวข้อมูล
 * @param ignore_ack 1 = ส่งต่อแม้ไม่ได้รับ ACK, 0 = หยุดส่งถ้าเจอ NACK
 * @return I2C_Soft_Status
 */
I2C_Soft_Status I2C_Soft_Write(uint8_t addr, uint8_t* data, uint16_t len, uint8_t ignore_ack);

/**
 * @brief อ่านข้อมูลจาก Software I2C
 * @param addr Address 7-bit
 * @param data Buffer เก็บข้อมูล
 * @param len ความยาว
 * @return I2C_Soft_Status
 */
I2C_Soft_Status I2C_Soft_Read(uint8_t addr, uint8_t* data, uint16_t len);

/**
 * @brief ฟังก์ชันระดับพื้นฐาน (Low-level)
 */
void I2C_Soft_Start(void);
void I2C_Soft_Stop(void);
uint8_t I2C_Soft_WriteByte(uint8_t byte);
uint8_t I2C_Soft_ReadByte(uint8_t ack);

#ifdef __cplusplus
}
#endif

#endif // __SIMPLE_I2C_SOFT_H
