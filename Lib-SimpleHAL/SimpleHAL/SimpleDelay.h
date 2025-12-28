/********************************** (C) COPYRIGHT
 * ******************************* File Name          : SimpleDelay.h Author    :
 * Extracted from WCH debug.h Version            : V1.0.0 Date               :
 * 2025/12/21 Description        : Delay and Timing Library for CH32V003 Provides
 * both blocking and non-blocking timing functions
 *********************************************************************************
 * Features:
 * - SysTick-based timing (1ms resolution)
 * - Auto-initialization (no need to call Timer_Init)
 * - Non-blocking timers with repeat support
 * - Blocking microsecond/millisecond delays
 * - High-precision time reading (millis/micros)
 *******************************************************************************/
#ifndef __SIMPLE_DELAY_H
#define __SIMPLE_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include <stdint.h>

/*================= TIMER STRUCTURE ==================*/

/**
 * @brief โครงสร้างข้อมูลสำหรับ Non-blocking Timer
 *
 * @param start_time เวลาเริ่มต้นของ timer (milliseconds)
 * @param duration ระยะเวลาที่ต้องการให้ timer ทำงาน (milliseconds)
 * @param active สถานะการทำงานของ timer (1 = active, 0 = inactive)
 * @param repeat โหมดการทำงานซ้ำ (0 = run once, 1 = repeat auto)
 */
typedef struct {
  uint32_t start_time;
  uint32_t duration;
  uint8_t active;
  uint8_t repeat;
} Timer_t;

/*================= INITIALIZATION ==================*/

/**
 * @brief เริ่มต้นระบบ Timer โดยใช้ SysTick
 *
 * ฟังก์ชันนี้จะตั้งค่า SysTick ให้ทำงานที่ความถี่ 1ms และเปิดใช้งาน interrupt
 * ต้องเรียกใช้ฟังก์ชันนี้ก่อนใช้งานฟังก์ชัน timer อื่นๆ
 *
 * @note ควรเรียกใช้ในตอนต้นของ main() หรือ boot sequence
 */
void Timer_Init(void);

/*================= NON-BLOCKING TIMERS ==================*/

/**
 * @brief เริ่มต้นการทำงานของ timer แบบ non-blocking
 *
 * @param timer ตัวชี้ไปยังโครงสร้าง Timer_t
 * @param ms ระยะเวลาในหน่วย milliseconds
 * @param repeat 1 = ทำงานซ้ำอัตโนมัติ, 0 = ทำงานครั้งเดียว
 *
 * @example
 * Timer_t my_timer;
 * Start_Timer(&my_timer, 1000, 1); // ทำงานซ้ำทุก 1 วินาที
 */
void Start_Timer(Timer_t *timer, uint32_t ms, uint8_t repeat);

/**
 * @brief รีเซ็ต timer ให้เริ่มนับใหม่
 *
 * @param timer ตัวชี้ไปยังโครงสร้าง Timer_t
 * @param repeat 1 = ทำงานซ้ำอัตโนมัติ, 0 = ทำงานครั้งเดียว
 */
void Reset_Timer(Timer_t *timer, uint8_t repeat);

/**
 * @brief ตรวจสอบว่า timer หมดเวลาแล้วหรือไม่
 *
 * @param timer ตัวชี้ไปยังโครงสร้าง Timer_t
 * @return 1 = หมดเวลาแล้ว, 0 = ยังไม่หมดเวลา
 *
 * @note ถ้า timer เป็นแบบ repeat จะรีเซ็ตอัตโนมัติ
 *       ถ้าไม่ repeat จะปิดการทำงานหลังหมดเวลา
 */
uint8_t Is_Timer_Expired(Timer_t *timer);

/**
 * @brief หยุดการทำงานของ timer
 *
 * @param timer ตัวชี้ไปยังโครงสร้าง Timer_t
 */
void Stop_Timer(Timer_t *timer);

/*================= BLOCKING DELAYS ==================*/

/**
 * @brief หน่วงเวลาแบบ blocking ในหน่วย microseconds
 *
 * @param n จำนวน microseconds ที่ต้องการหน่วง
 *
 * @warning ฟังก์ชันนี้จะบล็อกการทำงานของโปรแกรม
 * @note ใช้ SysTick timer สำหรับความแม่นยำสูง (ความละเอียด ~1us)
 *       รองรับ overflow อัตโนมัติด้วย unsigned arithmetic
 *       ไม่ขึ้นกับ compiler optimization
 */
void Delay_Us(uint32_t n);

/**
 * @brief หน่วงเวลาแบบ blocking ในหน่วย milliseconds
 *
 * @param n จำนวน milliseconds ที่ต้องการหน่วง
 *
 * @warning ฟังก์ชันนี้จะบล็อกการทำงานของโปรแกรม
 * @note ใช้ SysTick timer สำหรับความแม่นยำสูง (ความละเอียด 1ms)
 *       รองรับ overflow อัตโนมัติด้วย unsigned arithmetic
 *       ไม่ขึ้นกับ compiler optimization
 */
void Delay_Ms(uint32_t n);

/*================= TIME READING ==================*/

/**
 * @brief อ่านค่า microseconds จากตัวนับ SysTick ปัจจุบัน
 *
 * @return ค่า microseconds ในช่วง 0-999 us ของ tick ปัจจุบัน
 *
 * @note ใช้สำหรับการวัดเวลาที่ละเอียดกว่า millisecond
 */
uint32_t Get_TickMicros(void);

/**
 * @brief อ่านค่าเวลาปัจจุบันในหน่วย milliseconds
 *
 * @return จำนวน milliseconds นับตั้งแต่เริ่มต้นระบบ
 *
 * @note ค่านี้จะ overflow ทุกๆ 49.7 วัน (2^32 ms)
 *       ใช้การคำนวณแบบ overflow-safe เมื่อเปรียบเทียบเวลา
 */
uint32_t Get_CurrentMs(void);

/**
 * @brief อ่านค่าเวลาปัจจุบันในหน่วย microseconds
 *
 * @return จำนวน microseconds นับตั้งแต่เริ่มต้นระบบ
 *
 * @note ค่านี้จะ overflow ทุกๆ 71.6 นาที (2^32 us)
 *       ฟังก์ชันนี้ปิด interrupt ชั่วคราวเพื่อความแม่นยำ
 */
uint32_t Get_CurrentUs(void);

/*================= HELPER MACROS ==================*/

/**
 * @brief คำนวณระยะเวลาที่ผ่านไปแบบ overflow-safe
 *
 * @param start เวลาเริ่มต้น (milliseconds)
 * @param current เวลาปัจจุบัน (milliseconds)
 * @return ระยะเวลาที่ผ่านไป (milliseconds)
 *
 * @note ใช้ unsigned arithmetic เพื่อจัดการกับ overflow
 */
#define ELAPSED_TIME(start, current) ((uint32_t)((current) - (start)))

/**
 * @brief ตรวจสอบว่าเวลาที่ผ่านไปเกินกำหนดหรือไม่
 *
 * @param start เวลาเริ่มต้น (milliseconds)
 * @param timeout ระยะเวลา timeout (milliseconds)
 * @return 1 = timeout แล้ว, 0 = ยังไม่ timeout
 */
#define IS_TIMEOUT(start, timeout)                                             \
  (ELAPSED_TIME(start, Get_CurrentMs()) >= (timeout))

#ifdef __cplusplus
}
#endif

#endif /* __SIMPLE_DELAY_H */
