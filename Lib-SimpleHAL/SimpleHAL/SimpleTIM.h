/**
 * @file SimpleTIM.h
 * @brief Simple Timer Library สำหรับ CH32V003 แบบ Arduino-style
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * Library นี้ให้ API ง่ายๆ สำหรับการใช้งาน Timer peripherals
 * รองรับ TIM1 และ TIM2 พร้อม interrupt callbacks
 * 
 * **คุณสมบัติ:**
 * - ตั้งค่า timer ด้วยความถี่ที่ต้องการ (Hz)
 * - Auto-calculate prescaler และ period
 * - Interrupt callback support
 * - Start/Stop timer control
 * - Counter reading/writing
 * 
 * **Timer Resources:**
 * - TIM1: Advanced timer (16-bit, 4 channels)
 * - TIM2: General purpose timer (16-bit, 4 channels)
 * 
 * @example
 * // ตัวอย่างการใช้งาน
 * #include "SimpleTIM.h"
 * 
 * void timer_callback(void) {
 *     // ทำงานทุก 1 วินาที
 *     printf("Tick!\n");
 * }
 * 
 * int main(void) {
 *     TIM_SimpleInit(TIM_1, 1);  // 1 Hz
 *     TIM_AttachInterrupt(TIM_1, timer_callback);
 *     TIM_Start(TIM_1);
 *     while(1);
 * }
 * 
 * @note ห้ามใช้ TIM1 ร่วมกับ SimplePWM ที่ใช้ TIM1
 */

#ifndef __SIMPLE_TIM_H
#define __SIMPLE_TIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x_tim.h>
#include <stdint.h>

/* ========== Timer Definitions ========== */

/**
 * @brief Timer instance identifiers
 */
typedef enum {
    TIM_1 = 0,  /**< TIM1 - Advanced timer */
    TIM_2 = 1   /**< TIM2 - General purpose timer */
} TIM_Instance;

/**
 * @brief Timer counting modes
 */
typedef enum {
    TIM_MODE_UP = 0,    /**< Count up mode */
    TIM_MODE_DOWN       /**< Count down mode */
} TIM_Mode;

/* ========== Function Prototypes ========== */

/**
 * @brief เริ่มต้น timer ด้วยความถี่ที่ต้องการ
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * @param frequency_hz ความถี่ที่ต้องการ (Hz)
 * 
 * @note ฟังก์ชันนี้จะคำนวณ prescaler และ period อัตโนมัติ
 * @note Timer จะยังไม่เริ่มทำงาน ต้องเรียก TIM_Start() ก่อน
 * 
 * @example
 * TIM_SimpleInit(TIM_1, 1000);  // 1 kHz
 * TIM_SimpleInit(TIM_2, 1);     // 1 Hz
 */
void TIM_SimpleInit(TIM_Instance timer, uint32_t frequency_hz);

/**
 * @brief เริ่มการนับของ timer
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * 
 * @example
 * TIM_Start(TIM_1);
 */
void TIM_Start(TIM_Instance timer);

/**
 * @brief หยุดการนับของ timer
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * 
 * @example
 * TIM_Stop(TIM_1);
 */
void TIM_Stop(TIM_Instance timer);

/**
 * @brief เปลี่ยนความถี่ของ timer
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * @param frequency_hz ความถี่ใหม่ (Hz)
 * 
 * @note Timer จะถูก reset และต้อง start ใหม่
 * 
 * @example
 * TIM_SetFrequency(TIM_1, 2000);  // เปลี่ยนเป็น 2 kHz
 */
void TIM_SetFrequency(TIM_Instance timer, uint32_t frequency_hz);

/**
 * @brief อ่านค่า counter ปัจจุบัน
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * @return ค่า counter (0 - period)
 * 
 * @example
 * uint16_t count = Simple_TIM_GetCounter(TIM_1);
 */
uint16_t Simple_TIM_GetCounter(TIM_Instance timer);

/**
 * @brief ตั้งค่า counter
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * @param value ค่าที่ต้องการตั้ง
 * 
 * @example
 * Simple_TIM_SetCounter(TIM_1, 0);  // Reset counter
 */
void Simple_TIM_SetCounter(TIM_Instance timer, uint16_t value);

/**
 * @brief อ่านค่า period ของ timer
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * @return ค่า period (ARR register)
 * 
 * @example
 * uint16_t period = TIM_GetPeriod(TIM_1);
 */
uint16_t TIM_GetPeriod(TIM_Instance timer);

/**
 * @brief ตั้งค่า update interrupt callback
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * @param callback ฟังก์ชันที่จะถูกเรียกเมื่อ timer overflow
 * 
 * @note Callback จะถูกเรียกทุกครั้งที่ counter overflow (update event)
 * @note ความถี่ของ callback = timer frequency
 * 
 * @example
 * void my_callback(void) {
 *     // ทำงานทุก timer overflow
 * }
 * TIM_AttachInterrupt(TIM_1, my_callback);
 */
void TIM_AttachInterrupt(TIM_Instance timer, void (*callback)(void));

/**
 * @brief ยกเลิก update interrupt
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * 
 * @example
 * TIM_DetachInterrupt(TIM_1);
 */
void TIM_DetachInterrupt(TIM_Instance timer);

/* ========== Advanced Functions ========== */

/**
 * @brief ตั้งค่า timer แบบละเอียด (manual prescaler และ period)
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * @param prescaler ค่า prescaler (0-65535)
 * @param period ค่า period/ARR (0-65535)
 * @param mode โหมดการนับ (TIM_MODE_UP หรือ TIM_MODE_DOWN)
 * 
 * @note ใช้สำหรับการตั้งค่าแบบละเอียด
 * @note Actual frequency = SystemCoreClock / ((prescaler+1) * (period+1))
 * 
 * @example
 * // 1 kHz @ 48MHz: prescaler=47, period=999
 * TIM_AdvancedInit(TIM_1, 47, 999, TIM_MODE_UP);
 */
void TIM_AdvancedInit(TIM_Instance timer, uint16_t prescaler, uint16_t period, TIM_Mode mode);

/**
 * @brief ตั้งค่า prescaler
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * @param prescaler ค่า prescaler ใหม่
 * 
 * @example
 * TIM_SetPrescaler(TIM_1, 47);
 */
void TIM_SetPrescaler(TIM_Instance timer, uint16_t prescaler);

/**
 * @brief อ่านค่า prescaler
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * @return ค่า prescaler
 * 
 * @example
 * uint16_t psc = Simple_TIM_GetPrescaler(TIM_1);
 */
uint16_t Simple_TIM_GetPrescaler(TIM_Instance timer);

/**
 * @brief ตั้งค่าโหมดการนับ
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * @param mode โหมดการนับ
 * 
 * @example
 * TIM_SetMode(TIM_1, TIM_MODE_DOWN);
 */
void TIM_SetMode(TIM_Instance timer, TIM_Mode mode);

/**
 * @brief Generate update event (reload registers)
 * @param timer Timer instance (TIM_1 หรือ TIM_2)
 * 
 * @note ใช้สำหรับ reload prescaler/period ทันที
 * 
 * @example
 * TIM_GenerateUpdate(TIM_1);
 */
void TIM_GenerateUpdate(TIM_Instance timer);

/* ========== Helper Macros ========== */

/**
 * @brief คำนวณ prescaler และ period จากความถี่
 * @note ใช้ภายใน library
 */
#define TIM_CALC_PSC_PERIOD(freq, psc, per) \
    do { \
        uint32_t ticks = SystemCoreClock / (freq); \
        if (ticks <= 65536) { \
            psc = 0; \
            per = ticks - 1; \
        } else { \
            psc = (ticks / 65536); \
            per = (ticks / (psc + 1)) - 1; \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_TIM_H
