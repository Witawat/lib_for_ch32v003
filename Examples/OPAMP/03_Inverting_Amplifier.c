/**
 * @file 03_Inverting_Amplifier.c
 * @brief ตัวอย่างการใช้งาน OPAMP แบบ Inverting Amplifier
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * Inverting Amplifier ขยายสัญญาณและกลับเฟส (180 องศา)
 * 
 * **สูตรคำนวณ Gain:**
 * ```
 * Gain = -(R2/R1)
 * ```
 * 
 * **วงจร:**
 * ```
 * Vref (GND หรือ Vcc/2) -> CHP0 (OPAMP+)
 * 
 * Input Signal --[R1]--+---> CHN0 (OPAMP-)
 *                      |
 *                      |
 *                   Output ----+---> ADC
 *                              |
 *                            [R2]
 *                              |
 *                              +---- (feedback)
 * ```
 * 
 * **ตัวอย่างการคำนวณ:**
 * - ต้องการ Gain = -2
 * - เลือก R1 = 10kΩ
 * - R2 = R1 * |Gain| = 10k * 2 = 20kΩ
 * 
 * - ต้องการ Gain = -5
 * - เลือก R1 = 10kΩ
 * - R2 = R1 * |Gain| = 10k * 5 = 50kΩ
 * 
 * **หมายเหตุ:**
 * - Positive input ต้องต่อกับ reference voltage (มักเป็น GND หรือ Vcc/2)
 * - สัญญาณ input ป้อนผ่าน R1 เข้า negative input
 * - Output จะกลับเฟสจาก input
 */

#include "debug.h"
#include "SimpleHAL/SimpleOPAMP.h"
#include "SimpleHAL/SimpleADC.h"

// กำหนดค่าความต้านทาน (Ω)
#define R1_VALUE    10000   // 10kΩ (input resistor)
#define R2_VALUE    20000   // 20kΩ (feedback) -> Gain = -2

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART
    USART_Printf_Init(115200);
    printf("\r\n=== OPAMP Inverting Amplifier Example ===\r\n");
    printf("Amplify signal with phase inversion\r\n\r\n");
    
    // คำนวณ gain ที่คาดหวัง
    float expected_gain = OPAMP_CalculateGainInv(R1_VALUE, R2_VALUE);
    printf("Circuit Configuration:\r\n");
    printf("  R1 (input): %lu Ω\r\n", R1_VALUE);
    printf("  R2 (feedback): %lu Ω\r\n", R2_VALUE);
    printf("  Expected Gain: %.2f\r\n", expected_gain);
    printf("  (Negative gain = phase inversion)\r\n\r\n");
    
    // เริ่มต้น ADC
    ADC_SimpleInit();
    
    // เริ่มต้น OPAMP เป็น Inverting Amplifier
    printf("Initializing OPAMP as Inverting Amplifier...\r\n");
    OPAMP_ConfigInverting(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();
    printf("OPAMP Enabled\r\n\r\n");
    
    printf("Instructions:\r\n");
    printf("1. Connect reference voltage (GND or Vcc/2) to CHP0\r\n");
    printf("2. Connect input signal through R1 (%lukΩ) to CHN0\r\n", R1_VALUE/1000);
    printf("3. Connect R2 (%lukΩ) from output to CHN0 (feedback)\r\n", R2_VALUE/1000);
    printf("4. Output = -%.1f × Input (inverted)\r\n\r\n", -expected_gain);
    
    printf("Monitoring amplifier performance...\r\n");
    printf("Note: For AC signals, observe phase inversion on oscilloscope\r\n\r\n");
    printf("Time(s) | Input(V) | Output(V) | Gain | Expected\r\n");
    printf("--------|----------|-----------|------|----------\r\n");
    
    uint32_t count = 0;
    float vref = 1.65f;  // สมมติว่าใช้ Vcc/2 = 1.65V เป็น reference
    
    while(1) {
        // อ่านค่า input (ADC_CH_A0)
        uint16_t input_adc = ADC_Read(ADC_CH_A0);
        float input_voltage = ADC_ToVoltage(input_adc, 3.3);
        
        // อ่านค่า output (ADC_CH_A1)
        uint16_t output_adc = ADC_Read(ADC_CH_A1);
        float output_voltage = ADC_ToVoltage(output_adc, 3.3);
        
        // คำนวณ gain จริง (relative to Vref)
        float input_relative = input_voltage - vref;
        float output_relative = output_voltage - vref;
        
        float actual_gain = 0.0f;
        if(input_relative > 0.01f || input_relative < -0.01f) {
            actual_gain = output_relative / input_relative;
        }
        
        // แสดงผล
        printf("%7lu | %8.3f | %9.3f | %4.1f | %8.1f\r\n",
               count / 2,
               input_voltage,
               output_voltage,
               actual_gain,
               expected_gain);
        
        // ตรวจสอบการกลับเฟส
        if(input_relative > 0.1f && output_relative > 0.0f) {
            printf("WARNING: Phase not inverted! Check connections.\r\n");
        }
        if(input_relative < -0.1f && output_relative < 0.0f) {
            printf("WARNING: Phase not inverted! Check connections.\r\n");
        }
        
        // ตรวจสอบ saturation
        if(output_voltage > 3.2f || output_voltage < 0.1f) {
            printf("WARNING: Output saturated! Reduce input amplitude.\r\n");
        }
        
        count++;
        Delay_Ms(500);
    }
}
