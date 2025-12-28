/**
 * @file 01_Basic_VoltageFollower.c
 * @brief ตัวอย่างการใช้งาน OPAMP แบบ Voltage Follower (Buffer)
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * Voltage Follower เป็นวงจร buffer ที่มี gain = 1
 * ใช้สำหรับ:
 * - แยก impedance ระหว่าง source และ load
 * - ป้องกัน loading effect
 * - Buffer สัญญาณก่อนส่งเข้า ADC
 * 
 * **วงจร:**
 * ```
 * Input (Potentiometer) -> CHP0 (OPAMP+)
 *                          |
 *                          v
 *                       OPAMP Output -> CHN0 (OPAMP-) [External wire]
 *                          |
 *                          v
 *                       ADC (อ่านค่า output)
 * ```
 * 
 * **การทำงาน:**
 * - Input voltage จาก potentiometer ป้อนเข้า positive input
 * - Output ต่อกลับไปที่ negative input (negative feedback)
 * - Output voltage = Input voltage (Gain = 1)
 * - Input impedance สูงมาก, Output impedance ต่ำมาก
 */

#include "debug.h"
#include "SimpleHAL/SimpleOPAMP.h"
#include "SimpleHAL/SimpleADC.h"
#include "SimpleHAL/SimpleGPIO.h"

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART สำหรับแสดงผล
    USART_Printf_Init(115200);
    printf("\r\n=== OPAMP Voltage Follower Example ===\r\n");
    printf("Voltage Follower (Buffer) - Gain = 1\r\n\r\n");
    
    // เริ่มต้น ADC สำหรับอ่านค่า input และ output
    ADC_SimpleInit();
    
    // เริ่มต้น OPAMP เป็น Voltage Follower
    printf("Initializing OPAMP as Voltage Follower...\r\n");
    OPAMP_SimpleInit(OPAMP_MODE_VOLTAGE_FOLLOWER);
    
    // เปิดใช้งาน OPAMP
    OPAMP_Enable();
    printf("OPAMP Enabled\r\n\r\n");
    
    // ตรวจสอบสถานะ
    if(OPAMP_IsEnabled()) {
        printf("OPAMP Status: Running\r\n");
    }
    
    printf("\r\nInstructions:\r\n");
    printf("1. Connect potentiometer to OPAMP positive input (CHP0)\r\n");
    printf("2. Connect OPAMP output to negative input (CHN0) externally\r\n");
    printf("3. Adjust potentiometer and observe input/output voltages\r\n");
    printf("4. Output should follow input (Vout = Vin)\r\n\r\n");
    
    printf("Reading values every 500ms...\r\n");
    printf("Time(s) | Input(V) | Output(V) | Difference(mV)\r\n");
    printf("--------|----------|-----------|---------------\r\n");
    
    uint32_t count = 0;
    
    while(1) {
        // อ่านค่า input voltage (ผ่าน ADC channel อื่น)
        // สมมติว่า potentiometer ต่อกับ ADC_CH_A0
        uint16_t input_adc = ADC_Read(ADC_CH_A0);
        float input_voltage = ADC_ToVoltage(input_adc, 3.3);
        
        // อ่านค่า output voltage (ผ่าน ADC)
        // สมมติว่า OPAMP output ต่อกับ ADC_CH_A1
        uint16_t output_adc = ADC_Read(ADC_CH_A1);
        float output_voltage = ADC_ToVoltage(output_adc, 3.3);
        
        // คำนวณความแตกต่าง
        float difference_mv = (output_voltage - input_voltage) * 1000.0f;
        
        // แสดงผล
        printf("%7lu | %8.3f | %9.3f | %13.1f\r\n", 
               count / 2,  // แสดงเป็นวินาที (500ms interval)
               input_voltage, 
               output_voltage, 
               difference_mv);
        
        // ตรวจสอบความถูกต้อง
        if(difference_mv > 50.0f || difference_mv < -50.0f) {
            printf("WARNING: Large difference detected! Check connections.\r\n");
        }
        
        count++;
        Delay_Ms(500);
    }
}
