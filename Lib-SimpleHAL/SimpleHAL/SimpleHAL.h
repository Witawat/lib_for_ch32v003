/**
 * @file SimpleHAL.h
 * @brief Simple Hardware Abstraction Layer สำหรับ CH32V003
 * @version 1.0
 * @date 2025-12-12
 * 
 * @details
 * Header file รวมสำหรับ Simple HAL Library
 * ใช้งานง่ายแบบ Arduino สำหรับ CH32V003
 * 
 * **Peripherals ที่รองรับ:**
 * - GPIO: การควบคุม digital I/O และ interrupts
 * - TIM: Timer peripherals พร้อม interrupt callbacks
 * - PWM: PWM output สำหรับควบคุม LED, มอเตอร์, servo
 * - USART: การสื่อสารแบบ serial
 * - I2C: การสื่อสารกับ sensors และ EEPROM
 * - SPI: การสื่อสารความเร็วสูง
 * - ADC: การอ่านค่า analog
 * - Flash: Flash memory storage
 * - IWDG: Independent Watchdog (ป้องกันระบบค้าง)
 * - WWDG: Window Watchdog (ตรวจสอบ timing เข้มงวด)
 * - DMA: Direct Memory Access (ถ่ายโอนข้อมูลความเร็วสูง)
 * 
 * **คุณสมบัติหลัก:**
 * - API แบบ Arduino-style
 * - Pin remapping support
 * - Comments ภาษาไทยครบถ้วน
 * - ใช้ Hardware peripheral (เร็วกว่า Software)
 * 
 * @example
 * #include <SimpleHAL.h>
 * 
 * int main(void) {
 *     SystemCoreClockUpdate();
 *     Timer_Init();
 *     
 *     // USART
 *     USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
 *     USART_Print(<Hello!\r\n<);
 *     
 *     // I2C
 *     I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
 *     I2C_WriteReg(0x50, 0x00, 0x55);
 *     
 *     // SPI
 *     SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
 *     uint8_t rx = SPI_Transfer(0xAA);
 * }
 * 
 * @author CH32V003 Simple HAL Team
 * @copyright MIT License
 */

#ifndef __SIMPLE_HAL_H
#define __SIMPLE_HAL_H

#ifdef __cplusplus
extern "C" {
#endif


/* ========== Include All SimpleHAL Libraries ========== */

#include <ch32v00x.h>
#include "SimpleUSART.h" // IWYU pragma: keep
#include "SimpleI2C.h" // IWYU pragma: keep
#include "SimpleSPI.h" // IWYU pragma: keep
#include "SimpleADC.h" // IWYU pragma: keep
#include "SimpleGPIO.h" // IWYU pragma: keep
#include "SimpleTIM.h" // IWYU pragma: keep
#include "SimpleTIM_Ext.h" // IWYU pragma: keep
#include "SimplePWM.h" // IWYU pragma: keep
#include "SimpleOPAMP.h" // IWYU pragma: keep
#include "SimpleFlash.h" // IWYU pragma: keep
#include "SimpleIWDG.h" // IWYU pragma: keep
#include "SimpleWWDG.h" // IWYU pragma: keep
#include "SimpleDelay.h" // IWYU pragma: keep
#include "Simple1Wire.h" // IWYU pragma: keep
#include "SimpleDMA.h" // IWYU pragma: keep
#include "SimplePWR.h" // IWYU pragma: keep

/* ========== Version Information ========== */

#define SIMPLE_HAL_VERSION_MAJOR  1
#define SIMPLE_HAL_VERSION_MINOR  9
#define SIMPLE_HAL_VERSION_PATCH  0


/* ========== Helper Macros ========== */

/**
 * @brief Initialize SimpleHAL (Optional)
 * 
 * @note Auto-initialization is already enabled
 *       Call this only if you need explicit control
 * 
 * @details
 * This function is optional because SimpleHAL components
 * auto-initialize themselves (e.g., SimpleDelay uses constructor)
 * 
 * @example
 * int main(void) {
 *     SimpleHAL_Init();  // Optional
 *     Delay_Ms(1000);
 * }
 */
void SimpleHAL_Init(void);

/* ========== Helper Macros ========== */

/**
 * @brief แปลง GPIO port เป็น RCC peripheral
 */
#ifndef GPIO_TO_RCC_PERIPH
#define GPIO_TO_RCC_PERIPH(port) \
    ((port == GPIOA) ? RCC_APB2Periph_GPIOA : \
     (port == GPIOC) ? RCC_APB2Periph_GPIOC : \
     (port == GPIOD) ? RCC_APB2Periph_GPIOD : 0)
#endif

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_HAL_H
