/**
 * @file SimpleGPIO_AnalogWrite_Test.c
 * @brief ทดสอบ analogWrite() function
 * @date 2025-12-21
 * 
 * @details
 * ทดสอบการสร้าง PWM จากทุก pins ที่รองรับ
 * 
 * **Hardware Setup:**
 * - ต่อ LED ที่ PC3 (PWM output)
 * - ต่อ USB-UART สำหรับดู output
 * 
 * **Expected Output:**
 * - LED fade in/out ค่อยๆ สว่าง/มืด
 * - แสดง duty cycle ผ่าน USART
 */

#include "SimpleHAL.h"
#include <stdio.h>

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Init USART for debug output
    USART_Printf_Init(115200);
    
    printf("\n=== SimpleGPIO analogWrite() Test ===\n");
    printf("Testing PWM on PC3 (LED fade)\n\n");
    
    // ไม่ต้อง init PWM - analogWrite() จะ init อัตโนมัติ
    
    uint8_t brightness = 0;
    int8_t direction = 1;
    
    while(1) {
        // Fade in/out
        analogWrite(PC3, brightness);
        
        printf("Brightness: %3u/255 (%2u%%)\n", 
               brightness, (brightness * 100) / 255);
        
        // Update brightness
        brightness += direction * 5;
        
        // Reverse direction at limits
        if (brightness >= 250) {
            direction = -1;
        } else if (brightness <= 5) {
            direction = 1;
        }
        
        Delay_Ms(50);
    }
}
