/**
 * @file SimpleSPI.h
 * @brief Simple SPI Library สำหรับ CH32V003 แบบ Arduino-style
 * @version 1.0
 * @date 2025-12-12
 * 
 * @details
 * Library นี้ห่อหุ้ม Hardware SPI ให้ใช้งานง่ายแบบ Arduino
 * รองรับ pin remapping และ SPI modes ต่างๆ
 * 
 * **คุณสมบัติ:**
 * - เริ่มต้นใช้งานได้ 1 บรรทัด
 * - รองรับ 2 pin configurations
 * - รองรับ SPI Mode 0-3
 * - ฟังก์ชัน transfer แบบ Arduino
 * - รองรับ buffer transfer
 * 
 * @example
 * // ตัวอย่างการใช้งาน
 * #include "SimpleSPI.h"
 * 
 * int main(void) {
 *     // เริ่มต้น SPI mode 0, 1MHz, default pins
 *     SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
 *     
 *     // ส่ง/รับข้อมูล
 *     SPI_SetCS(0);  // CS = LOW
 *     uint8_t rx = SPI_Transfer(0xAA);
 *     SPI_SetCS(1);  // CS = HIGH
 * }
 */

#ifndef __SIMPLE_SPI_H
#define __SIMPLE_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x_spi.h>
#include <stdint.h>

/* ========== Enumerations ========== */

/**
 * @brief SPI Mode (CPOL และ CPHA)
 * 
 * @details
 * - Mode 0: CPOL=0, CPHA=0 (Clock idle LOW, sample on rising edge)
 * - Mode 1: CPOL=0, CPHA=1 (Clock idle LOW, sample on falling edge)
 * - Mode 2: CPOL=1, CPHA=0 (Clock idle HIGH, sample on falling edge)
 * - Mode 3: CPOL=1, CPHA=1 (Clock idle HIGH, sample on rising edge)
 */
typedef enum {
    SPI_MODE0 = 0,  /**< CPOL=0, CPHA=0 */
    SPI_MODE1 = 1,  /**< CPOL=0, CPHA=1 */
    SPI_MODE2 = 2,  /**< CPOL=1, CPHA=0 */
    SPI_MODE3 = 3   /**< CPOL=1, CPHA=1 */
} SPI_Mode;

/**
 * @brief ความเร็ว SPI
 */
typedef enum {
    SPI_125KHZ = 0,   /**< 125 kHz (PCLK/256) */
    SPI_250KHZ = 1,   /**< 250 kHz (PCLK/128) */
    SPI_500KHZ = 2,   /**< 500 kHz (PCLK/64) */
    SPI_1MHZ   = 3,   /**< 1 MHz (PCLK/32) */
    SPI_2MHZ   = 4,   /**< 2 MHz (PCLK/16) */
    SPI_4MHZ   = 5,   /**< 4 MHz (PCLK/8) */
    SPI_8MHZ   = 6,   /**< 8 MHz (PCLK/4) */
    SPI_12MHZ  = 7    /**< 12 MHz (PCLK/2) */
} SPI_Speed;

/**
 * @brief การเลือก Pin Configuration
 * 
 * @details Pin mapping สำหรับ SPI1:
 * - SPI_PINS_DEFAULT: SCK=PC5, MISO=PC7, MOSI=PC6, NSS=PC4
 * - SPI_PINS_REMAP:   SCK=PC6, MISO=PC8, MOSI=PC7, NSS=PC5
 */
typedef enum {
    SPI_PINS_DEFAULT = 0,  /**< Default pins */
    SPI_PINS_REMAP   = 1   /**< Remapped pins */
} SPI_PinConfig;

/**
 * @brief Bit Order
 */
typedef enum {
    SPI_MSB_FIRST = 0,  /**< MSB first (ปกติ) */
    SPI_LSB_FIRST = 1   /**< LSB first */
} SPI_BitOrder;

/* ========== Function Prototypes ========== */

/**
 * @brief เริ่มต้นการใช้งาน SPI
 * @param mode SPI mode (0-3)
 * @param speed ความเร็ว SPI
 * @param pin_config การเลือก pin configuration
 * 
 * @note ฟังก์ชันนี้จะ:
 *       1. เปิด Clock สำหรับ SPI และ GPIO
 *       2. ตั้งค่า Pin remapping ตามที่เลือก
 *       3. ตั้งค่า SPI mode และ speed
 *       4. เปิดใช้งาน SPI
 * 
 * @example
 * SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
 */
void SPI_SimpleInit(SPI_Mode mode, SPI_Speed speed, SPI_PinConfig pin_config);

/**
 * @brief ส่งและรับข้อมูล 1 byte
 * @param data ข้อมูลที่ต้องการส่ง
 * @return ข้อมูลที่รับได้
 * 
 * @note SPI เป็น full-duplex ส่งและรับพร้อมกัน
 * 
 * @example
 * uint8_t rx = SPI_Transfer(0xAA);
 */
uint8_t SPI_Transfer(uint8_t data);

/**
 * @brief ส่งและรับข้อมูลหลาย bytes
 * @param tx_data pointer ไปยังข้อมูลที่ต้องการส่ง (NULL = ส่ง 0x00)
 * @param rx_data pointer ไปยัง buffer สำหรับเก็บข้อมูลที่รับ (NULL = ไม่เก็บ)
 * @param len จำนวน bytes
 * 
 * @example
 * uint8_t tx[] = {0x01, 0x02, 0x03};
 * uint8_t rx[3];
 * SPI_TransferBuffer(tx, rx, 3);
 */
void SPI_TransferBuffer(uint8_t* tx_data, uint8_t* rx_data, uint16_t len);

/**
 * @brief ส่งข้อมูลอย่างเดียว (ไม่สนใจข้อมูลที่รับ)
 * @param data pointer ไปยังข้อมูลที่ต้องการส่ง
 * @param len จำนวน bytes
 * 
 * @example
 * uint8_t data[] = {0x01, 0x02, 0x03};
 * SPI_Write(data, 3);
 */
void SPI_Write(uint8_t* data, uint16_t len);

/**
 * @brief รับข้อมูลอย่างเดียว (ส่ง 0x00 หรือ 0xFF)
 * @param data pointer ไปยัง buffer สำหรับเก็บข้อมูล
 * @param len จำนวน bytes
 * @param dummy_byte ค่าที่ส่งไปขณะรับข้อมูล (ปกติใช้ 0x00 หรือ 0xFF)
 * 
 * @example
 * uint8_t buffer[10];
 * SPI_Read(buffer, 10, 0xFF);
 */
void SPI_Read(uint8_t* data, uint16_t len, uint8_t dummy_byte);

/**
 * @brief ควบคุม CS (Chip Select) pin
 * @param state 0 = LOW (active), 1 = HIGH (inactive)
 * 
 * @note ต้องควบคุม CS เองก่อนและหลังการ transfer
 * 
 * @example
 * SPI_SetCS(0);  // เลือก device
 * SPI_Transfer(0xAA);
 * SPI_SetCS(1);  // ยกเลิกการเลือก
 */
void SPI_SetCS(uint8_t state);

/**
 * @brief ตั้งค่า bit order
 * @param order SPI_MSB_FIRST หรือ SPI_LSB_FIRST
 * 
 * @example
 * SPI_SetBitOrder(SPI_LSB_FIRST);
 */
void SPI_SetBitOrder(SPI_BitOrder order);

/**
 * @brief เปลี่ยนความเร็ว SPI
 * @param speed ความเร็วใหม่
 * 
 * @example
 * SPI_SetSpeed(SPI_8MHZ);
 */
void SPI_SetSpeed(SPI_Speed speed);

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_SPI_H
