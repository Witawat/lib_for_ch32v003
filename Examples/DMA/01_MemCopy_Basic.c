/**
 * @file 01_MemCopy_Basic.c
 * @brief ตัวอย่างการใช้ DMA สำหรับ Memory-to-Memory Transfer แบบพื้นฐาน
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * ตัวอย่างนี้แสดงการใช้ DMA เพื่อ copy ข้อมูลจาก memory หนึ่งไปยังอีก memory หนึ่ง
 * และเปรียบเทียบความเร็วกับการใช้ memcpy() ปกติ
 * 
 * **สิ่งที่เรียนรู้:**
 * - การใช้ DMA_MemCopy() แบบ blocking
 * - การเปรียบเทียบความเร็วระหว่าง DMA และ CPU
 * - การตรวจสอบความถูกต้องของข้อมูล
 */

#include "SimpleHAL/SimpleHAL.h"
#include <stdio.h>
#include <string.h>

/* ========== Configuration ========== */

#define BUFFER_SIZE  1000  // ขนาด buffer สำหรับทดสอบ

/* ========== Global Variables ========== */

uint8_t source_buffer[BUFFER_SIZE];
uint8_t dest_buffer_dma[BUFFER_SIZE];
uint8_t dest_buffer_cpu[BUFFER_SIZE];

/* ========== Function Prototypes ========== */

void fill_test_data(void);
uint8_t verify_data(uint8_t* src, uint8_t* dst, uint16_t size);
void print_results(uint32_t dma_time, uint32_t cpu_time);

/* ========== Main Function ========== */

int main(void) {
    uint32_t start_time, dma_time, cpu_time;
    
    // เริ่มต้นระบบ
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART สำหรับแสดงผล
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    printf("\r\n=== DMA Memory Copy Example ===\r\n");
    printf("Buffer size: %d bytes\r\n\r\n", BUFFER_SIZE);
    
    // เตรียมข้อมูลทดสอบ
    fill_test_data();
    
    /* ===== ทดสอบ DMA Copy ===== */
    
    printf("Testing DMA copy...\r\n");
    
    // Clear destination buffer
    memset(dest_buffer_dma, 0, BUFFER_SIZE);
    
    // เริ่มจับเวลา
    start_time = micros();
    
    // Copy ด้วย DMA
    DMA_MemCopy(dest_buffer_dma, source_buffer, BUFFER_SIZE);
    
    // หยุดจับเวลา
    dma_time = micros() - start_time;
    
    // ตรวจสอบความถูกต้อง
    if (verify_data(source_buffer, dest_buffer_dma, BUFFER_SIZE)) {
        printf("✓ DMA copy successful!\r\n");
    } else {
        printf("✗ DMA copy failed!\r\n");
    }
    
    printf("DMA time: %lu us\r\n\r\n", dma_time);
    
    /* ===== ทดสอบ CPU Copy (memcpy) ===== */
    
    printf("Testing CPU copy (memcpy)...\r\n");
    
    // Clear destination buffer
    memset(dest_buffer_cpu, 0, BUFFER_SIZE);
    
    // เริ่มจับเวลา
    start_time = micros();
    
    // Copy ด้วย CPU
    memcpy(dest_buffer_cpu, source_buffer, BUFFER_SIZE);
    
    // หยุดจับเวลา
    cpu_time = micros() - start_time;
    
    // ตรวจสอบความถูกต้อง
    if (verify_data(source_buffer, dest_buffer_cpu, BUFFER_SIZE)) {
        printf("✓ CPU copy successful!\r\n");
    } else {
        printf("✗ CPU copy failed!\r\n");
    }
    
    printf("CPU time: %lu us\r\n\r\n", cpu_time);
    
    /* ===== แสดงผลการเปรียบเทียบ ===== */
    
    print_results(dma_time, cpu_time);
    
    printf("\r\n=== Test Complete ===\r\n");
    
    while (1) {
        // หยุดโปรแกรม
        Delay_Ms(1000);
    }
}

/* ========== Helper Functions ========== */

/**
 * @brief เติมข้อมูลทดสอบใน source buffer
 */
void fill_test_data(void) {
    for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
        source_buffer[i] = (uint8_t)(i & 0xFF);
    }
}

/**
 * @brief ตรวจสอบความถูกต้องของข้อมูล
 */
uint8_t verify_data(uint8_t* src, uint8_t* dst, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        if (src[i] != dst[i]) {
            printf("Error at index %d: expected 0x%02X, got 0x%02X\r\n", 
                   i, src[i], dst[i]);
            return 0;
        }
    }
    return 1;
}

/**
 * @brief แสดงผลการเปรียบเทียบ
 */
void print_results(uint32_t dma_time, uint32_t cpu_time) {
    printf("=== Performance Comparison ===\r\n");
    printf("DMA time: %lu us\r\n", dma_time);
    printf("CPU time: %lu us\r\n", cpu_time);
    
    if (dma_time < cpu_time) {
        float speedup = (float)cpu_time / (float)dma_time;
        printf("DMA is %.2fx faster!\r\n", speedup);
    } else if (cpu_time < dma_time) {
        float slowdown = (float)dma_time / (float)cpu_time;
        printf("CPU is %.2fx faster (DMA overhead)\r\n", slowdown);
    } else {
        printf("Same performance\r\n");
    }
    
    // คำนวณ throughput
    float throughput_dma = (float)BUFFER_SIZE / (float)dma_time;  // MB/s
    float throughput_cpu = (float)BUFFER_SIZE / (float)cpu_time;
    
    printf("\r\nThroughput:\r\n");
    printf("DMA: %.2f MB/s\r\n", throughput_dma);
    printf("CPU: %.2f MB/s\r\n", throughput_cpu);
}

/**
 * @brief ข้อสังเกต
 * 
 * - DMA เหมาะสำหรับการ copy ข้อมูลขนาดใหญ่
 * - สำหรับข้อมูลขนาดเล็ก (<100 bytes) CPU อาจเร็วกว่า (overhead ของ DMA)
 * - DMA ช่วยให้ CPU ทำงานอื่นได้ระหว่างการ copy (ดูตัวอย่างถัดไป)
 */
