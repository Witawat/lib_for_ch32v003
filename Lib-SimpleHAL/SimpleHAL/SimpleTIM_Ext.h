/**
 * @file SimpleTIM_Ext.h
 * @brief SimpleTIM Extensions - Stopwatch และ Countdown Timer Library
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * Library นี้เพิ่มฟังก์ชันการจับเวลาระดับสูงให้กับ SimpleTIM
 * รองรับทั้ง Stopwatch (นับขึ้น) และ Countdown (นับลง)
 * 
 * **คุณสมบัติ:**
 * - Stopwatch: นับเวลาขึ้นจาก 00:00:00
 * - Countdown: นับเวลาลงจากเวลาที่กำหนด
 * - รองรับ 3 รูปแบบเวลา: HH:MM:SS, MM:SS, SS
 * - 2 โหมดแสดงผล: NORMALIZED และ RAW
 * - Pause/Resume/Reset functionality
 * - Alarm callback สำหรับ countdown
 * 
 * **Time Formats:**
 * - HH:MM:SS: ชั่วโมง:นาที:วินาที (เช่น 02:30:45)
 * - MM:SS: นาที:วินาที (เช่น 146:30)
 * - SS: วินาทีเท่านั้น (เช่น 450)
 * 
 * **Display Modes:**
 * - NORMALIZED: แปลงหน่วยตามปกติ (60s=1min, 60min=1hr)
 * - RAW: แสดงค่าตรงๆ ไม่แปลง (เช่น 450 วินาที, 146 นาที)
 * 
 * **ข้อจำกัด:**
 * - Maximum time: ~49 วัน (uint32_t milliseconds overflow)
 * 
 * @example
 * // Stopwatch example
 * #include "SimpleTIM_Ext.h"
 * 
 * char buffer[32];
 * 
 * int main(void) {
 *     Stopwatch_Init();
 *     Stopwatch_Start();
 *     
 *     while(1) {
 *         Stopwatch_GetTimeString(buffer, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);
 *         printf("Time: %s\r\n", buffer);
 *         Delay_Ms(1000);
 *     }
 * }
 * 
 * @note ใช้ TIM2 เป็น default timer (ไม่ขัดแย้งกับ SimplePWM ที่ใช้ TIM1)
 */

#ifndef __SIMPLE_TIM_EXT_H
#define __SIMPLE_TIM_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SimpleTIM.h"
#include <stdint.h>
#include <string.h>

/* ========== Type Definitions ========== */

/**
 * @brief Time format types
 */
typedef enum {
    TIME_FORMAT_HHMMSS = 0,  /**< Hours:Minutes:Seconds (เช่น 02:30:45) */
    TIME_FORMAT_MMSS,        /**< Minutes:Seconds (เช่น 146:30) */
    TIME_FORMAT_SS           /**< Seconds only (เช่น 450) */
} TimeFormat_t;

/**
 * @brief Display mode types
 */
typedef enum {
    TIME_DISPLAY_NORMALIZED = 0,  /**< แปลงหน่วยปกติ (60s=1min, 60min=1hr) */
    TIME_DISPLAY_RAW             /**< แสดงค่าตรงๆ ไม่แปลง */
} TimeDisplayMode_t;

/**
 * @brief Time structure
 */
typedef struct {
    uint16_t hours;      /**< ชั่วโมง (0-1193 สำหรับ max time) */
    uint8_t minutes;     /**< นาที (0-59 normalized, 0-255 raw) */
    uint8_t seconds;     /**< วินาที (0-59 normalized, 0-255 raw) */
} Time_t;

/* ========== Buffer Size Recommendations ========== */

/**
 * @brief แนะนำขนาด buffer สำหรับแต่ละ format
 */
#define TIME_BUFFER_SIZE_SS      12   /**< "4294967295\0" */
#define TIME_BUFFER_SIZE_MMSS    16   /**< "71582788:07\0" */
#define TIME_BUFFER_SIZE_HHMMSS  20   /**< "1193046:28:07\0" */

/* ========== Stopwatch Functions ========== */

/**
 * @brief เริ่มต้น stopwatch
 * 
 * @note ต้องเรียกฟังก์ชันนี้ก่อนใช้งาน stopwatch
 * @note ใช้ TIM2 ที่ 1000Hz (1ms resolution)
 * 
 * @example
 * Stopwatch_Init();
 */
void Stopwatch_Init(void);

/**
 * @brief เริ่มการนับของ stopwatch
 * 
 * @note ถ้าเคย pause ไว้ จะนับต่อจากเวลาที่หยุด
 * 
 * @example
 * Stopwatch_Start();
 */
void Stopwatch_Start(void);

/**
 * @brief หยุดการนับของ stopwatch (pause)
 * 
 * @note เวลาจะถูกเก็บไว้ สามารถ start ต่อได้
 * 
 * @example
 * Stopwatch_Stop();
 */
void Stopwatch_Stop(void);

/**
 * @brief Reset stopwatch กลับเป็น 00:00:00
 * 
 * @note Stopwatch จะหยุดทำงาน ต้อง start ใหม่
 * 
 * @example
 * Stopwatch_Reset();
 */
void Stopwatch_Reset(void);

/**
 * @brief อ่านเวลาปัจจุบันของ stopwatch
 * @param time Pointer ไปยัง Time_t structure
 * 
 * @note เวลาจะถูกแปลงเป็น HH:MM:SS แบบ normalized
 * 
 * @example
 * Time_t current_time;
 * Stopwatch_GetTime(&current_time);
 * printf("%02u:%02u:%02u\n", current_time.hours, current_time.minutes, current_time.seconds);
 */
void Stopwatch_GetTime(Time_t* time);

/**
 * @brief อ่านเวลาเป็น string ตามรูปแบบที่กำหนด
 * @param buffer Buffer สำหรับเก็บ string (ดูขนาดที่แนะนำใน TIME_BUFFER_SIZE_*)
 * @param format รูปแบบเวลา (HHMMSS, MMSS, SS)
 * @param mode โหมดแสดงผล (NORMALIZED, RAW)
 * 
 * @example
 * char buf[32];
 * // แสดง 450 วินาทีแบบ normalized: "00:07:30"
 * Stopwatch_GetTimeString(buf, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);
 * 
 * // แสดง 450 วินาทีแบบ raw: "450"
 * Stopwatch_GetTimeString(buf, TIME_FORMAT_SS, TIME_DISPLAY_RAW);
 */
void Stopwatch_GetTimeString(char* buffer, TimeFormat_t format, TimeDisplayMode_t mode);

/**
 * @brief อ่านเวลาทั้งหมดเป็นวินาที
 * @return จำนวนวินาทีทั้งหมด
 * 
 * @example
 * uint32_t total_sec = Stopwatch_GetTotalSeconds();
 * printf("Total: %lu seconds\n", total_sec);
 */
uint32_t Stopwatch_GetTotalSeconds(void);

/**
 * @brief อ่านเวลาทั้งหมดเป็น milliseconds
 * @return จำนวน milliseconds ทั้งหมด
 * 
 * @example
 * uint32_t total_ms = Stopwatch_GetTotalMilliseconds();
 * printf("Total: %lu ms\n", total_ms);
 */
uint32_t Stopwatch_GetTotalMilliseconds(void);

/**
 * @brief ตรวจสอบว่า stopwatch กำลังทำงานอยู่หรือไม่
 * @return 1 ถ้ากำลังทำงาน, 0 ถ้าหยุด
 * 
 * @example
 * if (Stopwatch_IsRunning()) {
 *     printf("Stopwatch is running\n");
 * }
 */
uint8_t Stopwatch_IsRunning(void);

/* ========== Countdown Functions ========== */

/**
 * @brief เริ่มต้น countdown timer ด้วยเวลาที่กำหนด
 * @param hours ชั่วโมง
 * @param minutes นาที
 * @param seconds วินาที
 * 
 * @note รองรับค่าไม่จำกัด (เช่น 450 วินาที, 146 นาที)
 * @note ใช้ TIM2 ที่ 1000Hz (1ms resolution)
 * 
 * @example
 * Countdown_Init(0, 5, 0);  // 5 นาที
 * Countdown_Init(0, 0, 450);  // 450 วินาที
 * Countdown_Init(0, 146, 0);  // 146 นาที
 */
void Countdown_Init(uint16_t hours, uint8_t minutes, uint8_t seconds);

/**
 * @brief เริ่มต้น countdown timer จากจำนวนวินาทีทั้งหมด
 * @param total_seconds จำนวนวินาทีทั้งหมด
 * 
 * @example
 * Countdown_InitFromSeconds(300);  // 5 นาที
 * Countdown_InitFromSeconds(450);  // 450 วินาที
 */
void Countdown_InitFromSeconds(uint32_t total_seconds);

/**
 * @brief เริ่มการนับถอยหลัง
 * 
 * @example
 * Countdown_Start();
 */
void Countdown_Start(void);

/**
 * @brief หยุดการนับถอยหลัง (pause)
 * 
 * @example
 * Countdown_Stop();
 */
void Countdown_Stop(void);

/**
 * @brief Reset countdown กลับเป็นเวลาเริ่มต้น
 * 
 * @example
 * Countdown_Reset();
 */
void Countdown_Reset(void);

/**
 * @brief อ่านเวลาที่เหลืออยู่
 * @param time Pointer ไปยัง Time_t structure
 * 
 * @example
 * Time_t remaining;
 * Countdown_GetTime(&remaining);
 */
void Countdown_GetTime(Time_t* time);

/**
 * @brief อ่านเวลาที่เหลือเป็น string
 * @param buffer Buffer สำหรับเก็บ string
 * @param format รูปแบบเวลา
 * @param mode โหมดแสดงผล
 * 
 * @example
 * char buf[32];
 * Countdown_GetTimeString(buf, TIME_FORMAT_MMSS, TIME_DISPLAY_RAW);
 */
void Countdown_GetTimeString(char* buffer, TimeFormat_t format, TimeDisplayMode_t mode);

/**
 * @brief ตรวจสอบว่า countdown หมดเวลาแล้วหรือไม่
 * @return 1 ถ้าหมดเวลา (00:00:00), 0 ถ้ายังเหลือเวลา
 * 
 * @example
 * if (Countdown_IsFinished()) {
 *     printf("Time's up!\n");
 * }
 */
uint8_t Countdown_IsFinished(void);

/**
 * @brief ตั้งค่า callback ที่จะถูกเรียกเมื่อหมดเวลา
 * @param callback ฟังก์ชัน callback
 * 
 * @note Callback จะถูกเรียกครั้งเดียวเมื่อ countdown ถึง 00:00:00
 * 
 * @example
 * void alarm_callback(void) {
 *     printf("ALARM!\n");
 *     digitalWrite(BUZZER_PIN, HIGH);
 * }
 * 
 * Countdown_SetAlarmCallback(alarm_callback);
 */
void Countdown_SetAlarmCallback(void (*callback)(void));

/**
 * @brief อ่านเวลาที่เหลือเป็นวินาที
 * @return จำนวนวินาทีที่เหลือ
 * 
 * @example
 * uint32_t remaining = Countdown_GetRemainingSeconds();
 */
uint32_t Countdown_GetRemainingSeconds(void);

/**
 * @brief ตรวจสอบว่า countdown กำลังทำงานอยู่หรือไม่
 * @return 1 ถ้ากำลังทำงาน, 0 ถ้าหยุด
 * 
 * @example
 * if (Countdown_IsRunning()) {
 *     printf("Countdown is running\n");
 * }
 */
uint8_t Countdown_IsRunning(void);

/* ========== Utility Functions ========== */

/**
 * @brief แปลง Time_t เป็น string
 * @param time Pointer ไปยัง Time_t structure
 * @param buffer Buffer สำหรับเก็บ string
 * @param format รูปแบบเวลา
 * @param mode โหมดแสดงผล
 * 
 * @example
 * Time_t t = {2, 30, 45};
 * char buf[32];
 * Time_ToString(&t, buf, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);
 * // buf = "02:30:45"
 */
void Time_ToString(Time_t* time, char* buffer, TimeFormat_t format, TimeDisplayMode_t mode);

/**
 * @brief แปลงวินาทีเป็น Time_t structure
 * @param total_seconds จำนวนวินาทีทั้งหมด
 * @param time Pointer ไปยัง Time_t structure
 * @param mode โหมดแสดงผล
 * 
 * @example
 * Time_t t;
 * Time_FromSeconds(450, &t, TIME_DISPLAY_NORMALIZED);
 * // t.hours = 0, t.minutes = 7, t.seconds = 30
 * 
 * Time_FromSeconds(450, &t, TIME_DISPLAY_RAW);
 * // t.hours = 0, t.minutes = 0, t.seconds = 450 (for SS format)
 */
void Time_FromSeconds(uint32_t total_seconds, Time_t* time, TimeDisplayMode_t mode);

/**
 * @brief แปลง Time_t เป็นวินาที
 * @param time Pointer ไปยัง Time_t structure
 * @return จำนวนวินาทีทั้งหมด
 * 
 * @example
 * Time_t t = {2, 30, 45};
 * uint32_t sec = Time_ToSeconds(&t);
 * // sec = 9045
 */
uint32_t Time_ToSeconds(Time_t* time);

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_TIM_EXT_H
