/**
 * @file 04_Comparator_Mode.c
 * @brief ตัวอย่างการใช้งาน OPAMP เป็น Comparator
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * Comparator เปรียบเทียบแรงดัน 2 ค่า และให้ output เป็น digital (HIGH/LOW)
 * 
 * **การทำงาน:**
 * - ถ้า V+ > V- : Output = HIGH (≈ Vcc)
 * - ถ้า V+ < V- : Output = LOW (≈ GND)
 * 
 * **วงจร:**
 * ```
 * Signal Input -> CHP0 (OPAMP+)
 * 
 * Threshold (Potentiometer) -> CHN0 (OPAMP-)
 * 
 * Output -> LED/Relay/Digital Input
 * ```
 * 
 * **การใช้งาน:**
 * - ตรวจจับ zero-crossing
 * - Window comparator
 * - Over-voltage/Under-voltage protection
 * - Light sensor threshold detection
 * - Temperature alarm
 */

#include "debug.h"
#include "SimpleHAL/SimpleOPAMP.h"
#include "SimpleHAL/SimpleADC.h"
#include "SimpleHAL/SimpleGPIO.h"

// กำหนด threshold voltage (V)
#define THRESHOLD_VOLTAGE   1.65f   // Vcc/2 = 1.65V

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART
    USART_Printf_Init(115200);
    printf("\r\n=== OPAMP Comparator Mode Example ===\r\n");
    printf("Compare input signal with threshold\r\n\r\n");
    
    // เริ่มต้น GPIO สำหรับ LED indicator
    pinMode(PC0, PIN_MODE_OUTPUT);
    digitalWrite(PC0, LOW);
    
    // เริ่มต้น ADC
    ADC_SimpleInit();
    
    // เริ่มต้น OPAMP เป็น Comparator
    printf("Initializing OPAMP as Comparator...\r\n");
    OPAMP_ConfigComparator(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();
    printf("OPAMP Enabled\r\n\r\n");
    
    printf("Configuration:\r\n");
    printf("  Threshold: %.2fV\r\n", THRESHOLD_VOLTAGE);
    printf("  Signal Input: CHP0 (positive input)\r\n");
    printf("  Threshold Input: CHN0 (negative input)\r\n");
    printf("  LED Output: PC0\r\n\r\n");
    
    printf("Instructions:\r\n");
    printf("1. Connect signal to CHP0 (positive input)\r\n");
    printf("2. Connect threshold voltage to CHN0 (negative input)\r\n");
    printf("3. LED will turn ON when signal > threshold\r\n");
    printf("4. LED will turn OFF when signal < threshold\r\n\r\n");
    
    printf("Monitoring comparator...\r\n");
    printf("Time(s) | Signal(V) | Threshold(V) | Output | LED\r\n");
    printf("--------|-----------|--------------|--------|-----\r\n");
    
    uint32_t count = 0;
    uint8_t prev_state = 0;
    
    while(1) {
        // อ่านค่า signal input (ADC_CH_A0)
        uint16_t signal_adc = ADC_Read(ADC_CH_A0);
        float signal_voltage = ADC_ToVoltage(signal_adc, 3.3);
        
        // อ่านค่า threshold (ADC_CH_A1)
        uint16_t threshold_adc = ADC_Read(ADC_CH_A1);
        float threshold_voltage = ADC_ToVoltage(threshold_adc, 3.3);
        
        // อ่านค่า comparator output (ADC_CH_A2)
        uint16_t output_adc = ADC_Read(ADC_CH_A2);
        float output_voltage = ADC_ToVoltage(output_adc, 3.3);
        
        // กำหนด logic level
        uint8_t output_state = (output_voltage > 1.5f) ? 1 : 0;
        
        // ควบคุม LED ตาม comparator output
        digitalWrite(PC0, output_state);
        
        // แสดงผลเมื่อมีการเปลี่ยนสถานะ
        if(output_state != prev_state || count % 4 == 0) {
            printf("%7lu | %9.3f | %12.3f | %6s | %s\r\n",
                   count / 2,
                   signal_voltage,
                   threshold_voltage,
                   output_state ? "HIGH" : "LOW",
                   output_state ? "ON" : "OFF");
            
            // แจ้งเตือนเมื่อเปลี่ยนสถานะ
            if(output_state != prev_state) {
                if(output_state) {
                    printf(">>> Signal exceeded threshold! <<<\r\n");
                } else {
                    printf(">>> Signal below threshold <<<\r\n");
                }
            }
        }
        
        prev_state = output_state;
        count++;
        Delay_Ms(500);
    }
}
