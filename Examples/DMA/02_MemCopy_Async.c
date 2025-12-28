/**
 * @file 02_MemCopy_Async.c
 * @brief ตัวอย่างการใช้ DMA แบบ Non-blocking พร้อม Callbacks
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * ตัวอย่างนี้แสดงการใช้ DMA แบบ asynchronous ที่ CPU สามารถทำงานอื่นได้
 * ระหว่างที่ DMA กำลังถ่ายโอนข้อมูล พร้อมใช้ callback functions
 * 
 * **สิ่งที่เรียนรู้:**
 * - การใช้ DMA_MemCopyAsync() แบบ non-blocking
 * - การใช้ callback functions
 * - การทำงานพร้อมกันระหว่าง CPU และ DMA
 */

#include "SimpleHAL/SimpleHAL.h"
#include <stdio.h>
#include <string.h>

/* ========== Configuration ========== */

#define BUFFER_SIZE  500

/* ========== Global Variables ========== */

uint8_t source_buffer[BUFFER_SIZE];
uint8_t dest_buffer[BUFFER_SIZE];

volatile uint8_t transfer_complete = 0;
volatile uint8_t transfer_error = 0;
volatile uint32_t cpu_work_count = 0;

/* ========== Callback Functions ========== */

/**
 * @brief Callback เมื่อการถ่ายโอนเสร็จสิ้น
 */
void on_transfer_complete(DMA_Channel channel) {
    transfer_complete = 1;
    printf("\r\n[Callback] Transfer complete on channel %d!\r\n", channel);
}

/**
 * @brief Callback เมื่อเกิด error
 */
void on_transfer_error(DMA_Channel channel) {
    transfer_error = 1;
    printf("\r\n[Callback] Transfer error on channel %d!\r\n", channel);
}

/* ========== Function Prototypes ========== */

void fill_test_data(void);
void do_cpu_work(void);

/* ========== Main Function ========== */

int main(void) {
    // เริ่มต้นระบบ
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    printf("\r\n=== DMA Async Copy with Callbacks ===\r\n");
    printf("Buffer size: %d bytes\r\n\r\n", BUFFER_SIZE);
    
    // เตรียมข้อมูล
    fill_test_data();
    memset(dest_buffer, 0, BUFFER_SIZE);
    
    // ตั้งค่า callbacks
    DMA_SetTransferCompleteCallback(DMA_CH1, on_transfer_complete);
    DMA_SetErrorCallback(DMA_CH1, on_transfer_error);
    
    printf("Starting async DMA transfer...\r\n");
    printf("CPU will do work while DMA is transferring...\r\n\r\n");
    
    // เริ่มการถ่ายโอนแบบ async
    uint32_t start_time = micros();
    DMA_MemCopyAsync(DMA_CH1, dest_buffer, source_buffer, BUFFER_SIZE);
    
    // CPU ทำงานอื่นได้ระหว่างที่ DMA กำลังทำงาน
    printf("CPU is working...\r\n");
    
    while (!transfer_complete && !transfer_error) {
        do_cpu_work();
        
        // แสดงความคืบหน้าทุกๆ 1000 iterations
        if (cpu_work_count % 1000 == 0) {
            printf(".");
        }
    }
    
    uint32_t total_time = micros() - start_time;
    
    printf("\r\n\r\n=== Results ===\r\n");
    printf("Total time: %lu us\r\n", total_time);
    printf("CPU work iterations: %lu\r\n", cpu_work_count);
    
    // ตรวจสอบความถูกต้อง
    uint8_t errors = 0;
    for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
        if (source_buffer[i] != dest_buffer[i]) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("✓ Data verification: PASS\r\n");
    } else {
        printf("✗ Data verification: FAIL (%d errors)\r\n", errors);
    }
    
    printf("\r\n=== Key Points ===\r\n");
    printf("- CPU did %lu iterations while DMA was working\r\n", cpu_work_count);
    printf("- This demonstrates concurrent CPU and DMA operation\r\n");
    printf("- Callbacks provide notification when transfer completes\r\n");
    
    printf("\r\n=== Test Complete ===\r\n");
    
    while (1) {
        Delay_Ms(1000);
    }
}

/* ========== Helper Functions ========== */

/**
 * @brief เติมข้อมูลทดสอบ
 */
void fill_test_data(void) {
    for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
        source_buffer[i] = (uint8_t)((i * 7 + 13) & 0xFF);
    }
}

/**
 * @brief จำลองการทำงานของ CPU
 */
void do_cpu_work(void) {
    // ทำงานบางอย่าง (เช่น คำนวณ)
    volatile uint32_t dummy = 0;
    for (uint8_t i = 0; i < 10; i++) {
        dummy += i;
    }
    cpu_work_count++;
}

/**
 * @brief ข้อสังเกต
 * 
 * **ข้อดีของ Async DMA:**
 * - CPU สามารถทำงานอื่นได้ระหว่างการถ่ายโอน
 * - เพิ่มประสิทธิภาพโดยรวมของระบบ
 * - Callbacks ช่วยให้รู้ว่าเมื่อไหร่เสร็จ
 * 
 * **กรณีใช้งานจริง:**
 * - ส่งข้อมูลผ่าน USART ขณะที่ CPU ประมวลผลข้อมูลชุดถัดไป
 * - อ่าน ADC ด้วย DMA ขณะที่ CPU คำนวณค่าเฉลี่ย
 * - ส่งข้อมูลไปยัง display ขณะที่ CPU เตรียม frame ถัดไป
 */
