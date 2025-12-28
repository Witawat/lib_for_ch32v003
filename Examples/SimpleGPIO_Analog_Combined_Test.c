/**
 * @file SimpleGPIO_Analog_Combined_Test.c
 * @brief ทดสอบ analogRead() และ analogWrite() ร่วมกัน
 * @date 2025-12-21
 * 
 * @details
 * ทดสอบการใช้งาน ADC และ PWM ร่วมกัน
 * ควบคุม LED brightness ด้วย potentiometer
 * 
 * **Hardware Setup:**
 * - ต่อ potentiometer ที่ PD2 (ADC input)
 * - ต่อ LED ที่ PC3 (PWM output)
 * - ต่อ USB-UART สำหรับดู output
 * 
 * **Expected Output:**
 * - LED brightness เปลี่ยนตาม potentiometer
 * - แสดงค่า ADC และ PWM ผ่าน USART
 */

#include "SimpleHAL.h"
#include <stdio.h>

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Init USART for debug output
    USART_Printf_Init(115200);
    
    printf("\n=== SimpleGPIO Analog Combined Test ===\n");
    printf("Potentiometer (PD2) -> LED (PC3)\n\n");
    
    while(1) {
        // อ่านค่า potentiometer
        uint16_t pot_value = analogRead(PD2);
        
        // แปลงเป็น PWM value (0-255)
        uint8_t brightness = (pot_value * 255) / 1023;
        
        // ควบคุม LED
        analogWrite(PC3, brightness);
        
        // แสดงผล
        float voltage = (pot_value / 1023.0) * 3.3;
        printf("POT: %4u (%.2fV) -> LED: %3u/255 (%2u%%)\n",
               pot_value, voltage, brightness, (brightness * 100) / 255);
        
        Delay_Ms(100);
    }
}
