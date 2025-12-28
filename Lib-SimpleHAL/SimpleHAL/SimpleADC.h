/**
 * @file SimpleADC.h
 * @brief Simple ADC Library สำหรับ CH32V003 แบบ Arduino-style
 * @version 1.0
 * @date 2025-12-12
 * 
 * @details
 * Library นี้ห่อหุ้ม Hardware ADC ให้ใช้งานง่ายแบบ Arduino analogRead()
 * รองรับการอ่านหลาย channels และแปลงค่าเป็น voltage
 * 
 * **คุณสมบัติ:**
 * - อ่าน ADC ช่องเดียวหรือหลายช่อง
 * - แปลงค่าเป็น voltage อัตโนมัติ
 * - รองรับ 10-bit resolution
 * - API แบบ Arduino analogRead()
 * 
 * **ADC Channels ของ CH32V003:**
 * - Channel 0 (PA2) - GPIOA Pin 2
 * - Channel 1 (PA1) - GPIOA Pin 1
 * - Channel 2 (PC4) - GPIOC Pin 4
 * - Channel 3 (PD2) - GPIOD Pin 2
 * - Channel 4 (PD3) - GPIOD Pin 3
 * - Channel 5 (PD5) - GPIOD Pin 5
 * - Channel 6 (PD6) - GPIOD Pin 6
 * - Channel 7 (PD4) - GPIOD Pin 4
 * 
 * **Backward Compatibility:**
 * - ADC_CH_A0-A7 aliases ยังใช้ได้ (deprecated)
 * - แนะนำให้ใช้ชื่อใหม่ที่ชัดเจนกว่า (ADC_CH_PD2, ADC_CH_PA1, etc.)
 * 
 * @example
 * // ตัวอย่างการใช้งาน
 * #include "SimpleADC.h"
 * 
 * int main(void) {
 *     // เริ่มต้น ADC
 *     ADC_SimpleInit();
 *     
 *     // อ่านค่า ADC ช่อง 0 (PA2)
 *     uint16_t value = ADC_Read(ADC_CH_0);
 *     // หรือใช้ alias
 *     value = ADC_Read(ADC_CH_PA2);
 *     
 *     // แปลงเป็น voltage (3.3V reference)
 *     float voltage = ADC_ToVoltage(value, 3.3);
 * }
 * 
 * @note CH32V003 มี ADC 10-bit (0-1023)
 */

#ifndef __SIMPLE_ADC_H
#define __SIMPLE_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include <stdint.h>

/* ========== Enumerations ========== */

/**
 * @brief ADC Channel definitions
 * 
 * @details แมป ADC channels ตาม CH32V003 datasheet
 * @note ADC pins: PA1, PA2, PC4, PD2, PD3, PD4, PD5, PD6
 */
typedef enum {
    ADC_CH_0 = 0,  /**< ADC Channel 0 - PA2 */
    ADC_CH_1 = 1,  /**< ADC Channel 1 - PA1 */
    ADC_CH_2 = 2,  /**< ADC Channel 2 - PC4 */
    ADC_CH_3 = 3,  /**< ADC Channel 3 - PD2 */
    ADC_CH_4 = 4,  /**< ADC Channel 4 - PD3 */
    ADC_CH_5 = 5,  /**< ADC Channel 5 - PD5 */
    ADC_CH_6 = 6,  /**< ADC Channel 6 - PD6 */
    ADC_CH_7 = 7,  /**< ADC Channel 7 - PD4 */
    
    // Aliases สำหรับความสะดวก
    ADC_CH_PA2 = 0,  /**< PA2 - ADC Channel 0 */
    ADC_CH_PA1 = 1,  /**< PA1 - ADC Channel 1 */
    ADC_CH_PC4 = 2,  /**< PC4 - ADC Channel 2 */
    ADC_CH_PD2 = 3,  /**< PD2 - ADC Channel 3 */
    ADC_CH_PD3 = 4,  /**< PD3 - ADC Channel 4 */
    ADC_CH_PD5 = 5,  /**< PD5 - ADC Channel 5 */
    ADC_CH_PD6 = 6,  /**< PD6 - ADC Channel 6 */
    ADC_CH_PD4 = 7,  /**< PD4 - ADC Channel 7 */
    
    // ========== Internal Channels ==========
    ADC_CH_8 = 8,  /**< ADC Channel 8 - Internal Vref */
    ADC_CH_9 = 9,  /**< ADC Channel 9 - Internal Vcal */
    
    ADC_CH_VREFINT = 8,  /**< Internal Reference Voltage (≈1.2V) */
    ADC_CH_VCALINT = 9,  /**< Internal Calibration Voltage */
    
    // ========== Backward Compatibility Aliases (DEPRECATED) ==========
    // เก็บไว้เพื่อ backward compatibility กับโค้ดเดิม
    // แมปตาม hardware channels ที่ถูกต้อง
    ADC_CH_A0 = 0,   /**< @deprecated ใช้ ADC_CH_PA2 แทน - Channel 0 (PA2) */
    ADC_CH_A1 = 1,   /**< @deprecated ใช้ ADC_CH_PA1 แทน - Channel 1 (PA1) */
    ADC_CH_A2 = 2,   /**< @deprecated ใช้ ADC_CH_PC4 แทน - Channel 2 (PC4) */
    ADC_CH_A3 = 3,   /**< @deprecated ใช้ ADC_CH_PD2 แทน - Channel 3 (PD2) */
    ADC_CH_A4 = 4,   /**< @deprecated ใช้ ADC_CH_PD3 แทน - Channel 4 (PD3) */
    ADC_CH_A5 = 5,   /**< @deprecated ใช้ ADC_CH_PD5 แทน - Channel 5 (PD5) */
    ADC_CH_A6 = 6,   /**< @deprecated ใช้ ADC_CH_PD6 แทน - Channel 6 (PD6) */
    ADC_CH_A7 = 7    /**< @deprecated ใช้ ADC_CH_PD4 แทน - Channel 7 (PD4) */
} ADC_Channel;

/**
 * @brief ADC Resolution
 */
#define ADC_RESOLUTION  1024    /**< 10-bit ADC (0-1023) */
#define ADC_MAX_VALUE   1023    /**< Maximum ADC value */

/**
 * @brief Internal Vref Constants
 * @note ค่าเหล่านี้อาจต้องปรับตาม datasheet หรือ calibration
 */
#define ADC_VREFINT_VOLTAGE  1.2f   /**< Internal Vref voltage (V) - typical 1.2V */
#define ADC_VREFINT_CAL      512    /**< Typical Vrefint ADC value at 3.3V VDD */

/* ========== Function Prototypes ========== */

/**
 * @brief เริ่มต้นการใช้งาน ADC (เปิดทุก channels)
 * 
 * @note ฟังก์ชันนี้จะเปิดใช้งาน ADC pins ทั้งหมด (A0-A7)
 *       ถ้าต้องการเปิดเฉพาะบางช่อง ใช้ ADC_SimpleInitChannels() แทน
 * 
 * @example
 * ADC_SimpleInit();  // เปิดทุก channels
 */
void ADC_SimpleInit(void);

/**
 * @brief เริ่มต้นการใช้งาน ADC แบบกำหนด channels เอง
 * @param channels array ของ channels ที่ต้องการเปิดใช้งาน
 * @param count จำนวน channels
 * 
 * @note ฟังก์ชันนี้จะเปิดเฉพาะ GPIO pins ที่ระบุ ประหยัดกว่า
 * 
 * @example
 * ADC_Channel ch[] = {ADC_CH_A0, ADC_CH_A1};
 * ADC_SimpleInitChannels(ch, 2);  // เปิดแค่ A0 และ A1
 */
void ADC_SimpleInitChannels(ADC_Channel* channels, uint8_t count);

/**
 * @brief เปิดใช้งาน ADC channel เพิ่มเติม
 * @param channel ADC channel ที่ต้องการเปิด
 * 
 * @note ต้องเรียก ADC_SimpleInit() หรือ ADC_SimpleInitChannels() ก่อน
 * 
 * @example
 * ADC_SimpleInit();
 * ADC_EnableChannel(ADC_CH_A2);  // เปิด A2 เพิ่ม
 */
void ADC_EnableChannel(ADC_Channel channel);

/**
 * @brief อ่านค่า ADC จากช่องที่ระบุ
 * @param channel ADC channel ที่ต้องการอ่าน
 * @return ค่า ADC (0-1023)
 * 
 * @example
 * uint16_t value = ADC_Read(ADC_CH_A0);
 */
uint16_t ADC_Read(ADC_Channel channel);

/**
 * @brief อ่านค่า ADC จากหลายช่อง
 * @param channels array ของ channels ที่ต้องการอ่าน
 * @param values array สำหรับเก็บค่าที่อ่านได้
 * @param count จำนวน channels
 * 
 * @example
 * ADC_Channel channels[] = {ADC_CH_A0, ADC_CH_A1, ADC_CH_A2};
 * uint16_t values[3];
 * ADC_ReadMultiple(channels, values, 3);
 */
void ADC_ReadMultiple(ADC_Channel* channels, uint16_t* values, uint8_t count);

/**
 * @brief แปลงค่า ADC เป็น voltage
 * @param adc_value ค่า ADC (0-1023)
 * @param vref แรงดันอ้างอิง (V) - ปกติใช้ 3.3V
 * @return แรงดัน (V)
 * 
 * @example
 * uint16_t adc = ADC_Read(ADC_CH_A0);
 * float voltage = ADC_ToVoltage(adc, 3.3);
 */
float ADC_ToVoltage(uint16_t adc_value, float vref);

/**
 * @brief อ่านค่า ADC และแปลงเป็น voltage ทันที
 * @param channel ADC channel
 * @param vref แรงดันอ้างอิง (V)
 * @return แรงดัน (V)
 * 
 * @example
 * float voltage = ADC_ReadVoltage(ADC_CH_A0, 3.3);
 */
float ADC_ReadVoltage(ADC_Channel channel, float vref);

/**
 * @brief อ่านค่า ADC แบบ average หลายครั้ง
 * @param channel ADC channel
 * @param samples จำนวนครั้งที่ต้องการอ่าน
 * @return ค่าเฉลี่ย ADC
 * 
 * @note ใช้สำหรับลด noise
 * 
 * @example
 * uint16_t avg = ADC_ReadAverage(ADC_CH_A0, 10);
 */
uint16_t ADC_ReadAverage(ADC_Channel channel, uint8_t samples);

/**
 * @brief แปลงค่า ADC เป็นเปอร์เซ็นต์
 * @param adc_value ค่า ADC (0-1023)
 * @return เปอร์เซ็นต์ (0-100)
 * 
 * @example
 * uint16_t adc = ADC_Read(ADC_CH_A0);
 * float percent = ADC_ToPercent(adc);
 */
float ADC_ToPercent(uint16_t adc_value);

/* ========== Internal Channel Functions ========== */

/**
 * @brief อ่านค่า Internal Reference Voltage (Vrefint)
 * @return ค่า ADC ของ Vrefint (0-1023)
 * 
 * @note ใช้สำหรับคำนวณแรงดัน VDD จริง
 * 
 * @example
 * uint16_t vref = ADC_ReadVrefInt();
 */
uint16_t ADC_ReadVrefInt(void);

/**
 * @brief คำนวณแรงดัน VDD จริงจาก Vrefint
 * @return แรงดัน VDD (V)
 * 
 * @note ใช้สูตร: VDD = VREFINT_VOLTAGE × 1023 / ADC_ReadVrefInt()
 * 
 * @example
 * float vdd = ADC_GetVDD();
 * // vdd จะเป็นแรงดันจ่ายจริงของ MCU (แบตเตอรี่)
 */
float ADC_GetVDD(void);

/**
 * @brief อ่านค่า ADC พร้อมชดเชยความผันผวนของ VDD
 * @param channel ADC channel ที่ต้องการอ่าน
 * @return แรงดันที่ชดเชยแล้ว (V)
 * 
 * @note ใช้ Vrefint เพื่อชดเชย VDD ให้ผลลัพธ์แม่นยำกว่า
 * 
 * @example
 * float voltage = ADC_ReadVoltageCompensated(ADC_CH_PD2);
 */
float ADC_ReadVoltageCompensated(ADC_Channel channel);

/**
 * @brief คำนวณเปอร์เซ็นต์แบตเตอรี่
 * @param vdd แรงดัน VDD ปัจจุบัน (V)
 * @param v_min แรงดันต่ำสุดของแบตเตอรี่ (V)
 * @param v_max แรงดันสูงสุดของแบตเตอรี่ (V)
 * @return เปอร์เซ็นต์แบตเตอรี่ (0-100)
 * 
 * @note สูตร: percent = (vdd - v_min) / (v_max - v_min) × 100
 *       ผลลัพธ์จะถูก clamp ให้อยู่ในช่วง 0-100%
 * 
 * @example
 * // สำหรับ Li-ion (4.2V เต็ม, 3.0V หมด)
 * float vdd = ADC_GetVDD();
 * float percent = ADC_GetBatteryPercent(vdd, 3.0, 4.2);
 * 
 * // สำหรับ Alkaline 2xAA (3.2V เต็ม, 2.0V หมด)
 * float percent = ADC_GetBatteryPercent(vdd, 2.0, 3.2);
 */
float ADC_GetBatteryPercent(float vdd, float v_min, float v_max);

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_ADC_H
