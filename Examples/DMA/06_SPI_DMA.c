/**
 * @file 06_SPI_DMA.c
 * @brief ตัวอย่างการใช้ DMA สำหรับ SPI Transfer
 * @version 1.0
 * @date 2025-12-22
 */

#include "SimpleHAL/SimpleHAL.h"
#include <stdio.h>

#define TRANSFER_SIZE 64

uint8_t tx_data[TRANSFER_SIZE];
uint8_t rx_data[TRANSFER_SIZE];

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
    
    printf("\r\n=== SPI DMA Transfer ===\r\n\r\n");
    
    // เตรียมข้อมูล
    for (uint8_t i = 0; i < TRANSFER_SIZE; i++) {
        tx_data[i] = i;
    }
    
    // ตั้งค่า DMA สำหรับ SPI
    DMA_SPI_Init(DMA_CH4, DMA_CH5);
    
    printf("Transferring %d bytes via SPI+DMA...\r\n", TRANSFER_SIZE);
    
    // ส่งและรับข้อมูล
    DMA_SPI_TransferBuffer(DMA_CH4, DMA_CH5, tx_data, rx_data, TRANSFER_SIZE);
    
    printf("Transfer complete!\r\n");
    printf("Received data: ");
    for (uint8_t i = 0; i < 16; i++) {
        printf("%02X ", rx_data[i]);
    }
    printf("...\r\n");
    
    while (1) {
        Delay_Ms(1000);
    }
}
