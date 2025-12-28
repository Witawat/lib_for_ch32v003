/**
 * @file 05_USART_DMA_RX.c
 * @brief ตัวอย่างการใช้ DMA สำหรับรับข้อมูลผ่าน USART (Circular Buffer)
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * แสดงการรับข้อมูลผ่าน USART โดยใช้ DMA circular buffer mode
 * เหมาะสำหรับรับข้อมูลต่อเนื่อง
 */

#include "SimpleHAL/SimpleHAL.h"
#include <stdio.h>
#include <string.h>

#define RX_BUFFER_SIZE 128

uint8_t rx_buffer[RX_BUFFER_SIZE];
uint16_t last_pos = 0;

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    printf("\r\n=== USART DMA RX (Circular Buffer) ===\r\n");
    printf("Send data via USART to see it echoed back\r\n\r\n");
    
    // ตั้งค่า DMA circular buffer สำหรับ RX
    DMA_USART_InitRx(DMA_CH3, rx_buffer, RX_BUFFER_SIZE, 1);
    DMA_Start(DMA_CH3);
    
    while (1) {
        // ตรวจสอบข้อมูลใหม่
        uint16_t current_pos = DMA_USART_GetReceivedCount(DMA_CH3, RX_BUFFER_SIZE);
        
        if (current_pos != last_pos) {
            // มีข้อมูลใหม่
            if (current_pos > last_pos) {
                // ข้อมูลไม่ wrap around
                for (uint16_t i = last_pos; i < current_pos; i++) {
                    printf("%c", rx_buffer[i]);
                }
            } else {
                // ข้อมูล wrap around
                for (uint16_t i = last_pos; i < RX_BUFFER_SIZE; i++) {
                    printf("%c", rx_buffer[i]);
                }
                for (uint16_t i = 0; i < current_pos; i++) {
                    printf("%c", rx_buffer[i]);
                }
            }
            
            last_pos = current_pos;
        }
        
        Delay_Ms(10);
    }
}
