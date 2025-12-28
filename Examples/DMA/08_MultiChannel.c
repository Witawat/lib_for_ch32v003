/**
 * @file 08_MultiChannel.c
 * @brief ตัวอย่างการใช้หลาย DMA Channels พร้อมกัน
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * แสดงการใช้งาน DMA หลาย channels พร้อมกันพร้อม priority management
 */

#include "SimpleHAL/SimpleHAL.h"
#include <stdio.h>

#define BUF_SIZE 200

uint8_t src1[BUF_SIZE], dst1[BUF_SIZE];
uint8_t src2[BUF_SIZE], dst2[BUF_SIZE];
uint8_t src3[BUF_SIZE], dst3[BUF_SIZE];

volatile uint8_t ch1_done = 0, ch2_done = 0, ch3_done = 0;

void on_ch1_complete(DMA_Channel ch) { ch1_done = 1; printf("CH1 done\r\n"); }
void on_ch2_complete(DMA_Channel ch) { ch2_done = 1; printf("CH2 done\r\n"); }
void on_ch3_complete(DMA_Channel ch) { ch3_done = 1; printf("CH3 done\r\n"); }

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    printf("\r\n=== Multi-Channel DMA Example ===\r\n\r\n");
    
    // เตรียมข้อมูล
    for (uint16_t i = 0; i < BUF_SIZE; i++) {
        src1[i] = i;
        src2[i] = i + 1;
        src3[i] = i + 2;
    }
    
    // ตั้งค่า callbacks
    DMA_SetTransferCompleteCallback(DMA_CH1, on_ch1_complete);
    DMA_SetTransferCompleteCallback(DMA_CH2, on_ch2_complete);
    DMA_SetTransferCompleteCallback(DMA_CH3, on_ch3_complete);
    
    printf("Starting 3 DMA transfers simultaneously...\r\n");
    printf("CH1: High priority\r\n");
    printf("CH2: Medium priority\r\n");
    printf("CH3: Low priority\r\n\r\n");
    
    // เริ่ม transfers พร้อมกัน (priority ต่างกัน)
    DMA_MemCopyAsync(DMA_CH1, dst1, src1, BUF_SIZE);  // High priority
    DMA_MemCopyAsync(DMA_CH2, dst2, src2, BUF_SIZE);  // Medium priority
    DMA_MemCopyAsync(DMA_CH3, dst3, src3, BUF_SIZE);  // Low priority
    
    // รอให้ทุก channels เสร็จ
    while (!ch1_done || !ch2_done || !ch3_done) {
        Delay_Ms(1);
    }
    
    printf("\r\nAll transfers complete!\r\n");
    printf("Note: Higher priority channels may finish first\r\n");
    
    while (1) {
        Delay_Ms(1000);
    }
}
