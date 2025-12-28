/**
 * @file 07_DoubleBuffer.c
 * @brief ตัวอย่าง Double Buffering Technique
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * แสดงเทคนิค double buffering โดยใช้ 2 buffers สลับกัน
 * ขณะที่ DMA กำลังประมวลผล buffer หนึ่ง CPU เตรียมข้อมูลใน buffer อีกอัน
 */

#include "SimpleHAL/SimpleHAL.h"
#include <stdio.h>

#define BUFFER_SIZE 100

uint8_t buffer_a[BUFFER_SIZE];
uint8_t buffer_b[BUFFER_SIZE];
uint8_t* current_buffer = buffer_a;
uint8_t* next_buffer = buffer_b;

volatile uint8_t transfer_done = 0;

void on_complete(DMA_Channel ch) {
    transfer_done = 1;
}

void prepare_data(uint8_t* buf, uint8_t value) {
    for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
        buf[i] = value + i;
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    printf("\r\n=== Double Buffering Example ===\r\n\r\n");
    
    DMA_SetTransferCompleteCallback(DMA_CH1, on_complete);
    
    for (uint8_t frame = 0; frame < 10; frame++) {
        // เตรียมข้อมูลใน next buffer
        prepare_data(next_buffer, frame * 10);
        
        // เริ่ม DMA transfer
        transfer_done = 0;
        DMA_MemCopyAsync(DMA_CH1, current_buffer, next_buffer, BUFFER_SIZE);
        
        // สลับ buffers
        uint8_t* temp = current_buffer;
        current_buffer = next_buffer;
        next_buffer = temp;
        
        // รอให้ transfer เสร็จ
        while (!transfer_done);
        
        printf("Frame %d processed\r\n", frame);
    }
    
    printf("\r\nDouble buffering complete!\r\n");
    
    while (1) {
        Delay_Ms(1000);
    }
}
