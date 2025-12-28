/**
 * @file SimplePWM.h
 * @brief Simple PWM Library สำหรับ CH32V003 แบบ Arduino-style
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * Library นี้ให้ API แบบ Arduino สำหรับการควบคุม PWM output
 * รองรับ TIM1 และ TIM2 (4 channels แต่ละตัว)
 * 
 * **คุณสมบัติ:**
 * - analogWrite() แบบ Arduino
 * - ตั้งค่า duty cycle (0-100%)
 * - ตั้งค่าความถี่ PWM
 * - รองรับ pin remapping
 * - Start/Stop PWM output
 * 
 * **PWM Channels และ Pins:**
 * 
 * **TIM1 (Default pins):**
 * - PWM1_CH1: PD2
 * - PWM1_CH2: PA1
 * - PWM1_CH3: PC3
 * - PWM1_CH4: PC4
 * 
 * **TIM2 (Default pins):**
 * - PWM2_CH1: PD4
 * - PWM2_CH2: PD3
 * - PWM2_CH3: PC0
 * - PWM2_CH4: PD7
 * 
 * @example
 * // ตัวอย่างการใช้งาน
 * #include "SimplePWM.h"
 * 
 * int main(void) {
 *     // LED fade
 *     PWM_Init(PWM1_CH1, 1000);  // 1kHz
 *     PWM_Start(PWM1_CH1);
 *     
 *     while(1) {
 *         for(int i=0; i<=100; i++) {
 *             PWM_SetDutyCycle(PWM1_CH1, i);
 *             Delay_Ms(10);
 *         }
 *     }
 * }
 * 
 * @note ห้ามใช้ SimpleTIM กับ timer เดียวกับ SimplePWM
 */

#ifndef __SIMPLE_PWM_H
#define __SIMPLE_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include <stdint.h>

/* ========== PWM Channel Definitions ========== */

/**
 * @brief PWM Channel identifiers
 */
typedef enum {
    // TIM1 Channels
    PWM1_CH1 = 0,   /**< TIM1 Channel 1 - PD2 */
    PWM1_CH2 = 1,   /**< TIM1 Channel 2 - PA1 */
    PWM1_CH3 = 2,   /**< TIM1 Channel 3 - PC3 */
    PWM1_CH4 = 3,   /**< TIM1 Channel 4 - PC4 */
    
    // TIM2 Channels
    PWM2_CH1 = 4,   /**< TIM2 Channel 1 - PD4 */
    PWM2_CH2 = 5,   /**< TIM2 Channel 2 - PD3 */
    PWM2_CH3 = 6,   /**< TIM2 Channel 3 - PC0 */
    PWM2_CH4 = 7    /**< TIM2 Channel 4 - PD7 */
} PWM_Channel;

/**
 * @brief PWM Pin Remap options
 */
typedef enum {
    PWM_REMAP_NONE = 0,      /**< No remap (default pins) */
    PWM_REMAP_PARTIAL1,      /**< Partial remap 1 */
    PWM_REMAP_PARTIAL2,      /**< Partial remap 2 */
    PWM_REMAP_FULL           /**< Full remap */
} PWM_Remap;

/* ========== Function Prototypes ========== */

/**
 * @brief เริ่มต้น PWM channel ด้วยความถี่ที่ต้องการ
 * @param channel PWM channel (PWM1_CH1 - PWM2_CH4)
 * @param frequency_hz ความถี่ PWM (Hz)
 * 
 * @note ฟังก์ชันนี้จะตั้งค่า GPIO และ timer อัตโนมัติ
 * @note Duty cycle เริ่มต้นที่ 0%
 * @note ต้องเรียก PWM_Start() เพื่อเริ่ม output
 * 
 * @example
 * PWM_Init(PWM1_CH1, 1000);  // 1 kHz PWM
 */
void PWM_Init(PWM_Channel channel, uint32_t frequency_hz);

/**
 * @brief เริ่มต้น PWM พร้อม pin remapping
 * @param channel PWM channel
 * @param frequency_hz ความถี่ PWM (Hz)
 * @param remap Pin remap option
 * 
 * @note ใช้สำหรับเปลี่ยน pin ของ PWM output
 * 
 * @example
 * PWM_InitRemap(PWM1_CH1, 1000, PWM_REMAP_FULL);
 */
void PWM_InitRemap(PWM_Channel channel, uint32_t frequency_hz, PWM_Remap remap);

/**
 * @brief ตั้งค่า duty cycle (0-100%)
 * @param channel PWM channel
 * @param duty_percent Duty cycle ในเปอร์เซ็นต์ (0-100)
 * 
 * @note 0% = always low, 100% = always high
 * 
 * @example
 * PWM_SetDutyCycle(PWM1_CH1, 50);  // 50% duty cycle
 */
void PWM_SetDutyCycle(PWM_Channel channel, uint8_t duty_percent);

/**
 * @brief ตั้งค่า duty cycle แบบ raw value
 * @param channel PWM channel
 * @param duty_value ค่า duty cycle (0 - period)
 * 
 * @note ใช้สำหรับความละเอียดสูงสุด
 * @note duty_value ต้องไม่เกิน period ของ timer
 * 
 * @example
 * uint16_t period = PWM_GetPeriod(PWM1_CH1);
 * PWM_SetDutyCycleRaw(PWM1_CH1, period / 2);  // 50%
 */
void PWM_SetDutyCycleRaw(PWM_Channel channel, uint16_t duty_value);

/**
 * @brief เปลี่ยนความถี่ PWM
 * @param channel PWM channel
 * @param frequency_hz ความถี่ใหม่ (Hz)
 * 
 * @note จะรีเซ็ต duty cycle เป็น 0%
 * @note ถ้าใช้หลาย channels บน timer เดียวกัน ความถี่จะเปลี่ยนทุก channels
 * 
 * @example
 * PWM_SetFrequency(PWM1_CH1, 2000);  // เปลี่ยนเป็น 2 kHz
 */
void PWM_SetFrequency(PWM_Channel channel, uint32_t frequency_hz);

/**
 * @brief เริ่ม PWM output
 * @param channel PWM channel
 * 
 * @example
 * PWM_Start(PWM1_CH1);
 */
void PWM_Start(PWM_Channel channel);

/**
 * @brief หยุด PWM output
 * @param channel PWM channel
 * 
 * @note Pin จะถูกตั้งเป็น low
 * 
 * @example
 * PWM_Stop(PWM1_CH1);
 */
void PWM_Stop(PWM_Channel channel);

/**
 * @brief Arduino-style analogWrite
 * @param channel PWM channel
 * @param value ค่า PWM (0-255)
 * 
 * @note 0 = 0%, 255 = 100%
 * @note ถ้า channel ยังไม่ได้ init จะ init อัตโนมัติที่ 1kHz
 * 
 * @example
 * PWM_Write(PWM1_CH1, 128);  // 50% duty cycle
 */
void PWM_Write(PWM_Channel channel, uint8_t value);

/* ========== Advanced Functions ========== */

/**
 * @brief อ่านค่า period ของ timer
 * @param channel PWM channel
 * @return ค่า period (ARR register)
 * 
 * @example
 * uint16_t period = PWM_GetPeriod(PWM1_CH1);
 */
uint16_t PWM_GetPeriod(PWM_Channel channel);

/**
 * @brief อ่านค่า duty cycle ปัจจุบัน (raw)
 * @param channel PWM channel
 * @return ค่า duty cycle (CCR register)
 * 
 * @example
 * uint16_t duty = PWM_GetDutyCycleRaw(PWM1_CH1);
 */
uint16_t PWM_GetDutyCycleRaw(PWM_Channel channel);

/**
 * @brief อ่านค่า duty cycle ปัจจุบัน (%)
 * @param channel PWM channel
 * @return Duty cycle ในเปอร์เซ็นต์ (0-100)
 * 
 * @example
 * uint8_t duty = PWM_GetDutyCycle(PWM1_CH1);
 */
uint8_t PWM_GetDutyCycle(PWM_Channel channel);

/**
 * @brief ตั้งค่า PWM แบบละเอียด
 * @param channel PWM channel
 * @param prescaler ค่า prescaler
 * @param period ค่า period
 * @param duty_value ค่า duty cycle
 * 
 * @note ใช้สำหรับการตั้งค่าแบบละเอียด
 * 
 * @example
 * PWM_AdvancedInit(PWM1_CH1, 47, 999, 500);
 */
void PWM_AdvancedInit(PWM_Channel channel, uint16_t prescaler, 
                      uint16_t period, uint16_t duty_value);

/**
 * @brief ตั้งค่า polarity ของ PWM output
 * @param channel PWM channel
 * @param inverted 0 = normal, 1 = inverted
 * 
 * @note Normal: high = active, Inverted: low = active
 * 
 * @example
 * PWM_SetPolarity(PWM1_CH1, 1);  // Inverted output
 */
void PWM_SetPolarity(PWM_Channel channel, uint8_t inverted);

/* ========== Helper Macros ========== */

/**
 * @brief แปลงเปอร์เซ็นต์เป็น raw value
 */
#define PWM_PERCENT_TO_RAW(percent, period) \
    ((uint16_t)(((uint32_t)(percent) * (period)) / 100))

/**
 * @brief แปลง raw value เป็นเปอร์เซ็นต์
 */
#define PWM_RAW_TO_PERCENT(raw, period) \
    ((uint8_t)(((uint32_t)(raw) * 100) / (period)))

/**
 * @brief แปลง Arduino value (0-255) เป็นเปอร์เซ็นต์
 */
#define PWM_ARDUINO_TO_PERCENT(value) \
    ((uint8_t)(((uint16_t)(value) * 100) / 255))

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_PWM_H
