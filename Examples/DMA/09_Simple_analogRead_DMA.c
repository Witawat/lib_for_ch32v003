/**
 * @file 09_Simple_analogRead_DMA.c
 * @brief ตัวอย่างการใช้ DMA กับ analogRead แบบง่ายมาก!
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * ตัวอย่างนี้แสดงการใช้ DMA กับ analogRead() แบบง่ายที่สุด
 * ไม่ต้องตั้งค่า ADC เอง ไม่ต้องตั้งค่า DMA เอง
 * เพียงแค่เรียก DMA_analogReadStart() เท่านั้น!
 */

#include "SimpleHAL/SimpleHAL.h"
#include <stdio.h>

/* ========== Configuration ========== */

#define BUFFER_SIZE 100  // เก็บค่า ADC 100 ตัวอย่าง

/* ========== Global Variables ========== */

uint16_t adc_buffer[BUFFER_SIZE];

/* ========== Main Function ========== */

int main(void) {
    // เริ่มต้นระบบ
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    printf("\r\n=== Simple analogRead + DMA Example ===\r\n");
    printf("Reading ADC from PD2 continuously...\r\n\r\n");
    
    /* ===== วิธีที่ 1: ใช้ DMA แบบง่ายมาก! ===== */
    
    // เริ่มอ่าน ADC ด้วย DMA (เพียง 1 บรรทัด!)
    DMA_analogReadStart(PD2, adc_buffer, BUFFER_SIZE, 1);  // 1 = continuous mode
    
    // รอให้ buffer เต็มครั้งแรก
    Delay_Ms(100);
    
    printf("DMA is running! CPU is free to do other work.\r\n\r\n");
    
    /* ===== แสดงผลทุกๆ 500ms ===== */
    
    while (1) {
        // อ่านค่าล่าสุดจาก buffer
        uint16_t latest = adc_buffer[BUFFER_SIZE - 1];
        
        // คำนวณค่าเฉลี่ย (ใช้ helper function)
        uint16_t average = DMA_analogReadAverage(adc_buffer, BUFFER_SIZE);
        
        // แปลงเป็น voltage
        float voltage_latest = ADC_ToVoltage(latest, 3.3);
        float voltage_avg = ADC_ToVoltage(average, 3.3);
        
        // แสดงผล
        printf("Latest: %4d (%.3fV)  |  Average: %4d (%.3fV)\r\n", 
               latest, voltage_latest, average, voltage_avg);
        
        Delay_Ms(500);
    }
}

/**
 * @brief ข้อสังเกต
 * 
 * **ง่ายแค่ไหน?**
 * - ไม่ต้องตั้งค่า ADC เอง
 * - ไม่ต้องตั้งค่า DMA เอง
 * - เพียงแค่เรียก DMA_analogReadStart() เท่านั้น!
 * 
 * **ข้อดี:**
 * - CPU ไม่ต้องรอ ADC conversion
 * - ได้ข้อมูลต่อเนื่องอัตโนมัติ
 * - คำนวณค่าเฉลี่ยได้ง่าย (ลด noise)
 * - ใช้งานง่ายเหมือน analogRead() ปกติ
 * 
 * **เปรียบเทียบ:**
 * 
 * // แบบเดิม (ไม่ใช้ DMA)
 * while (1) {
 *     uint16_t value = analogRead(PD2);  // CPU blocked!
 *     Delay_Ms(10);
 * }
 * 
 * // แบบใหม่ (ใช้ DMA)
 * DMA_analogReadStart(PD2, buffer, 100, 1);  // เริ่มครั้งเดียว
 * while (1) {
 *     uint16_t value = buffer[99];  // อ่านจาก buffer (ไม่ blocked!)
 *     // CPU ทำงานอื่นได้
 * }
 */
