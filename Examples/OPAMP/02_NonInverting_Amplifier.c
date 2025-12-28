/**
 * @file 02_NonInverting_Amplifier.c
 * @brief ตัวอย่างการใช้งาน OPAMP แบบ Non-Inverting Amplifier
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * Non-Inverting Amplifier ขยายสัญญาณโดยไม่กลับเฟส
 * 
 * **สูตรคำนวณ Gain:**
 * ```
 * Gain = 1 + (R2/R1)
 * ```
 * 
 * **วงจร:**
 * ```
 * Input Signal -> CHP0 (OPAMP+)
 *                    |
 *                    v
 *                 OPAMP
 *                    |
 *                    v
 *                 Output ----+---> ADC (อ่านค่า)
 *                            |
 *                           R2
 *                            |
 *                            +---> CHN0 (OPAMP-)
 *                            |
 *                           R1
 *                            |
 *                           GND
 * ```
 * 
 * **ตัวอย่างการคำนวณ:**
 * - ต้องการ Gain = 2
 * - เลือก R1 = 10kΩ
 * - R2 = R1 * (Gain - 1) = 10k * (2-1) = 10kΩ
 * 
 * - ต้องการ Gain = 5
 * - เลือก R1 = 10kΩ
 * - R2 = R1 * (Gain - 1) = 10k * (5-1) = 40kΩ
 */

#include "debug.h"
#include "SimpleHAL/SimpleOPAMP.h"
#include "SimpleHAL/SimpleADC.h"

// กำหนดค่าความต้านทาน (Ω)
#define R1_VALUE    10000   // 10kΩ (ต่อ GND)
#define R2_VALUE    10000   // 10kΩ (feedback) -> Gain = 2

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART
    USART_Printf_Init(115200);
    printf("\r\n=== OPAMP Non-Inverting Amplifier Example ===\r\n");
    printf("Amplify signal without phase inversion\r\n\r\n");
    
    // คำนวณ gain ที่คาดหวัง
    float expected_gain = OPAMP_CalculateGainNonInv(R1_VALUE, R2_VALUE);
    printf("Circuit Configuration:\r\n");
    printf("  R1 (to GND): %lu Ω\r\n", R1_VALUE);
    printf("  R2 (feedback): %lu Ω\r\n", R2_VALUE);
    printf("  Expected Gain: %.2f\r\n\r\n", expected_gain);
    
    // เริ่มต้น ADC
    ADC_SimpleInit();
    
    // เริ่มต้น OPAMP เป็น Non-Inverting Amplifier
    printf("Initializing OPAMP as Non-Inverting Amplifier...\r\n");
    OPAMP_ConfigNonInverting(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();
    printf("OPAMP Enabled\r\n\r\n");
    
    printf("Instructions:\r\n");
    printf("1. Connect input signal to CHP0 (positive input)\r\n");
    printf("2. Connect R2 (%lukΩ) from output to CHN0\r\n", R2_VALUE/1000);
    printf("3. Connect R1 (%lukΩ) from CHN0 to GND\r\n", R1_VALUE/1000);
    printf("4. Output should be %.1fx of input\r\n\r\n", expected_gain);
    
    printf("Monitoring amplifier performance...\r\n");
    printf("Time(s) | Input(V) | Output(V) | Actual Gain | Error(%%)\r\n");
    printf("--------|----------|-----------|-------------|----------\r\n");
    
    uint32_t count = 0;
    
    while(1) {
        // อ่านค่า input (ADC_CH_A0)
        uint16_t input_adc = ADC_Read(ADC_CH_A0);
        float input_voltage = ADC_ToVoltage(input_adc, 3.3);
        
        // อ่านค่า output (ADC_CH_A1)
        uint16_t output_adc = ADC_Read(ADC_CH_A1);
        float output_voltage = ADC_ToVoltage(output_adc, 3.3);
        
        // คำนวณ gain จริง
        float actual_gain = 0.0f;
        if(input_voltage > 0.01f) {  // ป้องกันการหารด้วยศูนย์
            actual_gain = output_voltage / input_voltage;
        }
        
        // คำนวณ error
        float error_percent = 0.0f;
        if(expected_gain > 0.0f) {
            error_percent = ((actual_gain - expected_gain) / expected_gain) * 100.0f;
        }
        
        // แสดงผล
        printf("%7lu | %8.3f | %9.3f | %11.2f | %8.1f\r\n",
               count / 2,
               input_voltage,
               output_voltage,
               actual_gain,
               error_percent);
        
        // ตรวจสอบความถูกต้อง
        if(error_percent > 10.0f || error_percent < -10.0f) {
            printf("WARNING: Gain error > 10%%. Check resistor values!\r\n");
        }
        
        // ตรวจสอบ saturation
        if(output_voltage > 3.2f) {
            printf("WARNING: Output near saturation! Reduce input.\r\n");
        }
        
        count++;
        Delay_Ms(500);
    }
}
