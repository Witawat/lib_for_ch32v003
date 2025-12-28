/********************************** (C) COPYRIGHT
 * ******************************* File Name          : SimpleDelay.c Author    :
 * Extracted from WCH debug.c Version            : V1.0.0 Date               :
 * 2025/12/21 Description        : Delay and Timing Library Implementation
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "SimpleDelay.h"

/* Global Variables */
static uint32_t us_per_tick = 0; // จำนวน microseconds ต่อ 1 tick
volatile uint32_t millis = 0;    // ตัวนับ milliseconds (ถูกแก้ไขใน interrupt)

/*================= INITIALIZATION ==================*/

/**
 * @brief เริ่มต้นระบบ Timer โดยใช้ SysTick
 *
 * ฟังก์ชันนี้จะ:
 * 1. ตั้งค่า SysTick ให้ทำงานที่ความถี่ 1ms
 * 2. เปิดใช้งาน SysTick interrupt
 * 3. เปิดใช้งาน NVIC สำหรับ SysTick
 *
 * @note ต้องเรียกใช้ก่อนใช้งานฟังก์ชัน timer อื่นๆ
 */
void Timer_Init(void) {
  us_per_tick = 1000;                    // ตั้งค่า SysTick interrupt ให้เกิดทุก 1ms
  SysTick->CTLR = 0;                     // ปิดการทำงานของ SysTick ก่อนตั้งค่า
  SysTick->SR = 0;                       // ล้างสถานะ interrupt flag
  SysTick->CNT = 0;                      // เคลียร์ตัวนับ
  SysTick->CMP = SystemCoreClock / 1000; // ตั้งค่าคอมแพร์เพื่อให้เกิด interrupt ทุก 1ms
  SysTick->CTLR = 0xF;          // เปิดการทำงานของ SysTick พร้อม interrupt
  NVIC_EnableIRQ(SysTick_IRQn); // เปิดใช้งาน SysTick ใน NVIC
}

/**
 * @brief Auto-initialization function
 *
 * ฟังก์ชันนี้จะถูกเรียกอัตโนมัติก่อน main()
 * ทำให้ไม่ต้องเรียก Timer_Init() เอง
 */
__attribute__((constructor))
static void SimpleDelay_AutoInit(void) {
  Timer_Init();
}

/**
 * @brief SysTick Interrupt Handler
 *
 * ฟังก์ชันนี้จะถูกเรียกทุกๆ 1ms โดย SysTick interrupt
 * ทำหน้าที่เพิ่มค่าตัวนับ millis
 */
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void SysTick_Handler(void) {
  SysTick->SR = 0; // ล้าง interrupt flag
  millis++;        // เพิ่มค่า millis ทุกๆ 1ms
}

/*================= BLOCKING DELAYS ==================*/

/**
 * @brief หน่วงเวลาแบบ blocking ในหน่วย microseconds
 *
 * @param n จำนวน microseconds ที่ต้องการหน่วง
 *
 * @warning ฟังก์ชันนี้จะบล็อกการทำงานของโปรแกรม
 * @note ใช้ SysTick timer สำหรับความแม่นยำสูง
 *       รองรับ overflow ด้วย unsigned arithmetic
 */
void Delay_Us(uint32_t n) {
  if (n == 0) return;
  
  uint32_t start = Get_CurrentUs();
  // ใช้ unsigned arithmetic เพื่อจัดการ overflow อัตโนมัติ
  while ((Get_CurrentUs() - start) < n) {
    // รอจนกว่าเวลาจะผ่านไป
  }
}

/**
 * @brief หน่วงเวลาแบบ blocking ในหน่วย milliseconds
 *
 * @param n จำนวน milliseconds ที่ต้องการหน่วง
 *
 * @warning ฟังก์ชันนี้จะบล็อกการทำงานของโปรแกรม
 * @note ใช้ SysTick timer สำหรับความแม่นยำสูง
 *       รองรับ overflow ด้วย unsigned arithmetic
 */
void Delay_Ms(uint32_t n) {
  if (n == 0) return;
  
  uint32_t start = Get_CurrentMs();
  // ใช้ unsigned arithmetic เพื่อจัดการ overflow อัตโนมัติ
  while ((Get_CurrentMs() - start) < n) {
    // รอจนกว่าเวลาจะผ่านไป
  }
}

/*================= TIME READING ==================*/

/**
 * @brief อ่านค่า microseconds จากตัวนับ SysTick ปัจจุบัน
 *
 * @return ค่า microseconds ในช่วง 0-999 us ของ tick ปัจจุบัน
 *
 * @note ใช้สำหรับการวัดเวลาที่ละเอียดกว่า millisecond
 */
uint32_t Get_TickMicros(void) {
  return (SysTick->CNT * us_per_tick) / SysTick->CMP;
}

/**
 * @brief อ่านค่าเวลาปัจจุบันในหน่วย milliseconds
 *
 * @return จำนวน milliseconds นับตั้งแต่เริ่มต้นระบบ
 *
 * @note ค่านี้จะ overflow ทุกๆ 49.7 วัน (2^32 ms)
 */
uint32_t Get_CurrentMs(void) { return millis; }

/**
 * @brief อ่านค่าเวลาปัจจุบันในหน่วย microseconds
 *
 * @return จำนวน microseconds นับตั้งแต่เริ่มต้นระบบ
 *
 * @note ค่านี้จะ overflow ทุกๆ 71.6 นาที (2^32 us)
 *       ฟังก์ชันนี้ปิด interrupt ชั่วคราวเพื่อความแม่นยำ
 */
uint32_t Get_CurrentUs(void) {
  uint32_t current_millis;
  uint32_t current_tick;

  __disable_irq();             // ปิด interrupt ชั่วคราว
  current_millis = millis;     // อ่านค่า millis
  current_tick = SysTick->CNT; // อ่านค่าตัวนับ SysTick
  __enable_irq();              // เปิด interrupt คืน

  uint32_t us_in_tick = (current_tick * us_per_tick) / SysTick->CMP;

  return (current_millis * 1000) + us_in_tick; // รวมค่า ms และ us
}

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
void Start_Timer(Timer_t *timer, uint32_t ms, uint8_t repeat) {
  if (timer != NULL) {
    timer->start_time = millis; // บันทึกเวลาปัจจุบัน
    timer->duration = ms;       // ตั้งค่าระยะเวลา
    timer->active = 1;          // เปิดการทำงาน
    timer->repeat = repeat;     // ตั้งค่าการทำงานซ้ำ
  }
}

/**
 * @brief รีเซ็ต timer ให้เริ่มนับใหม่
 *
 * @param timer ตัวชี้ไปยังโครงสร้าง Timer_t
 * @param repeat 1 = ทำงานซ้ำอัตโนมัติ, 0 = ทำงานครั้งเดียว
 */
void Reset_Timer(Timer_t *timer, uint8_t repeat) {
  if (timer != NULL) {
    timer->start_time = millis; // รีเซ็ตเวลาเริ่มต้น
    timer->active = 1;          // เปิดการทำงาน
    timer->repeat = repeat;     // ตั้งค่าการทำงานซ้ำ
  }
}

/**
 * @brief ตรวจสอบว่า timer หมดเวลาแล้วหรือไม่
 *
 * @param timer ตัวชี้ไปยังโครงสร้าง Timer_t
 * @return 1 = หมดเวลาแล้ว, 0 = ยังไม่หมดเวลา
 *
 * @note ถ้า timer เป็นแบบ repeat จะรีเซ็ตอัตโนมัติ
 *       ถ้าไม่ repeat จะปิดการทำงานหลังหมดเวลา
 */
uint8_t Is_Timer_Expired(Timer_t *timer) {
  if (timer == NULL) {
    return 0;
  }

  // ตรวจสอบว่า timer ทำงานอยู่และเวลาที่ผ่านไปครบหรือยัง
  // ใช้ unsigned arithmetic เพื่อจัดการกับ overflow
  if (timer->active && ((millis - timer->start_time) >= timer->duration)) {
    if (timer->repeat) {
      timer->start_time = millis; // รีเซ็ตเวลาถ้าเป็นแบบ repeat
    } else {
      timer->active = 0; // ปิดการทำงานถ้าไม่ repeat
    }
    return 1; // หมดเวลาแล้ว
  }
  return 0; // ยังไม่หมดเวลา
}

/**
 * @brief หยุดการทำงานของ timer
 *
 * @param timer ตัวชี้ไปยังโครงสร้าง Timer_t
 */
void Stop_Timer(Timer_t *timer) {
  if (timer != NULL) {
    timer->active = 0;
  }
}
