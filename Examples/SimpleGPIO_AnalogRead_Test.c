/**
 * @file SimpleGPIO_AnalogRead_Test.c
 * @brief ทดสอบ analogRead() function
 * @date 2025-12-21
 * 
 * @details
 * ทดสอบการอ่านค่า ADC จากทุก pins ที่รองรับ
 * 
 * **Hardware Setup:**
 * - ต่อ potentiometer ที่ PD2 (ADC input)
 * - ต่อ USB-UART สำหรับดู output
 * 
 * **Expected Output:**
 * - แสดงค่า ADC (0-1023) และ voltage ผ่าน USART
 * - ค่าเปลี่ยนตามการหมุน potentiometer
 */

#include "SimpleHAL.h"
#include <stdio.h>

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Init USART for debug output
    USART_Printf_Init(115200);
    
    printf("\n=== SimpleGPIO analogRead() Test ===\n");
    printf("Testing all ADC pins (PD2-PD7)\n\n");
    
    // ไม่ต้อง init ADC - analogRead() จะ init อัตโนมัติ
    
    while(1) {
        printf("ADC Readings:\n");
        
        // Test PD2 (ADC Channel 3)
        uint16_t pd2 = analogRead(PD2);
        float v_pd2 = (pd2 / 1023.0) * 3.3;
        printf("  PD2: %4u (%.2fV)\n", pd2, v_pd2);
        
        // Test PD3 (ADC Channel 4)
        uint16_t pd3 = analogRead(PD3);
        float v_pd3 = (pd3 / 1023.0) * 3.3;
        printf("  PD3: %4u (%.2fV)\n", pd3, v_pd3);
        
        // Test PD4 (ADC Channel 7)
        uint16_t pd4 = analogRead(PD4);
        float v_pd4 = (pd4 / 1023.0) * 3.3;
        printf("  PD4: %4u (%.2fV)\n", pd4, v_pd4);
        
        // Test PD5 (ADC Channel 5)
        uint16_t pd5 = analogRead(PD5);
        float v_pd5 = (pd5 / 1023.0) * 3.3;
        printf("  PD5: %4u (%.2fV)\n", pd5, v_pd5);
        
        // Test PD6 (ADC Channel 6)
        uint16_t pd6 = analogRead(PD6);
        float v_pd6 = (pd6 / 1023.0) * 3.3;
        printf("  PD6: %4u (%.2fV)\n", pd6, v_pd6);
        
        // Test PD7 (ADC Channel 0)
        uint16_t pd7 = analogRead(PD7);
        float v_pd7 = (pd7 / 1023.0) * 3.3;
        printf("  PD7: %4u (%.2fV)\n", pd7, v_pd7);
        
        printf("\n");
        
        // Test error handling - PC0 ไม่รองรับ ADC
        uint16_t pc0 = analogRead(PC0);
        printf("PC0 (should be 0): %u\n\n", pc0);
        
        Delay_Ms(1000);
    }
}
