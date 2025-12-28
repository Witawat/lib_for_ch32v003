/**
 * @file 04_USART_DMA_TX.c
 * @brief ตัวอย่างการใช้ DMA สำหรับส่งข้อมูลผ่าน USART
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * แสดงการส่งข้อมูลขนาดใหญ่ผ่าน USART โดยใช้ DMA
 * CPU สามารถทำงานอื่นได้ระหว่างการส่งข้อมูล
 */

#include "SimpleHAL/SimpleHAL.h"
#include <stdio.h>
#include <string.h>

#define TX_BUFFER_SIZE 256

uint8_t tx_buffer[TX_BUFFER_SIZE];
volatile uint8_t tx_complete = 0;

void on_tx_complete(DMA_Channel channel) {
    tx_complete = 1;
}

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    printf("\r\n=== USART DMA TX Example ===\r\n\r\n");
    
    // เตรียมข้อมูลที่จะส่ง
    const char* message = "This is a long message sent via DMA! ";
    uint16_t msg_len = strlen(message);
    
    // เติมข้อมูลใน buffer (ส่งซ้ำหลายครั้ง)
    uint16_t pos = 0;
    while (pos < TX_BUFFER_SIZE - msg_len) {
        memcpy(&tx_buffer[pos], message, msg_len);
        pos += msg_len;
    }
    
    // ตั้งค่า DMA สำหรับ USART TX
    DMA_USART_InitTx(DMA_CH2, tx_buffer, TX_BUFFER_SIZE);
    DMA_SetTransferCompleteCallback(DMA_CH2, on_tx_complete);
    
    printf("Sending %d bytes via DMA...\r\n", TX_BUFFER_SIZE);
    printf("CPU can do other work during transmission!\r\n\r\n");
    
    // เริ่มส่งข้อมูล
    DMA_USART_Transmit(DMA_CH2, tx_buffer, TX_BUFFER_SIZE);
    
    // CPU ทำงานอื่นได้
    uint32_t work_count = 0;
    while (!tx_complete) {
        work_count++;
    }
    
    printf("\r\n\r\nTransmission complete!\r\n");
    printf("CPU did %lu iterations during transmission\r\n", work_count);
    
    while (1) {
        Delay_Ms(1000);
    }
}
