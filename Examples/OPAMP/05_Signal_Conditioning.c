/**
 * @file 05_Signal_Conditioning.c
 * @brief ตัวอย่างการใช้ OPAMP สำหรับปรับสัญญาณก่อนส่งเข้า ADC
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * Signal Conditioning คือการปรับสัญญาณให้เหมาะสมก่อนส่งเข้า ADC
 * 
 * **วัตถุประสงค์:**
 * - ขยายสัญญาณเล็กๆ ให้ใช้ full range ของ ADC
 * - Buffer สัญญาณเพื่อป้องกัน loading effect
 * - กรองสัญญาณ noise
 * - ปรับ offset ให้อยู่ในช่วงที่ ADC อ่านได้
 * 
 * **ตัวอย่างการใช้งาน:**
 * 1. Thermocouple Amplifier (ขยายสัญญาณ mV)
 * 2. Strain Gauge Amplifier (ขยายสัญญาณ differential)
 * 3. Photodiode Amplifier (แปลงกระแส -> แรงดัน)
 * 4. Sensor Interface (ปรับช่วงแรงดัน)
 * 
 * **ตัวอย่างนี้:**
 * - รับสัญญาณจาก LM35 temperature sensor (10mV/°C)
 * - ขยาย 10 เท่า เพื่อให้ ADC อ่านได้ละเอียดขึ้น
 * - แปลงเป็นอุณหภูมิ (°C)
 */

#include "debug.h"
#include "SimpleHAL/SimpleOPAMP.h"
#include "SimpleHAL/SimpleADC.h"

// LM35 Temperature Sensor Configuration
#define LM35_MV_PER_CELSIUS     10.0f   // 10mV/°C
#define AMPLIFIER_GAIN          10.0f   // ขยาย 10 เท่า
#define R1_VALUE                10000   // 10kΩ
#define R2_VALUE                90000   // 90kΩ -> Gain = 1 + (90k/10k) = 10

// Temperature range
#define TEMP_MIN                0.0f    // °C
#define TEMP_MAX                100.0f  // °C

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART
    USART_Printf_Init(115200);
    printf("\r\n=== OPAMP Signal Conditioning Example ===\r\n");
    printf("LM35 Temperature Sensor Interface\r\n\r\n");
    
    // คำนวณและแสดงค่า gain
    float calculated_gain = OPAMP_CalculateGainNonInv(R1_VALUE, R2_VALUE);
    printf("Amplifier Configuration:\r\n");
    printf("  Sensor: LM35 (10mV/°C)\r\n");
    printf("  R1: %lu Ω\r\n", R1_VALUE);
    printf("  R2: %lu Ω\r\n", R2_VALUE);
    printf("  Gain: %.1f\r\n", calculated_gain);
    printf("  Output: %.0f mV/°C\r\n\r\n", LM35_MV_PER_CELSIUS * calculated_gain);
    
    // เริ่มต้น ADC
    ADC_SimpleInit();
    
    // เริ่มต้น OPAMP เป็น Non-Inverting Amplifier
    printf("Initializing OPAMP for signal conditioning...\r\n");
    OPAMP_ConfigNonInverting(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();
    printf("OPAMP Enabled\r\n\r\n");
    
    printf("Instructions:\r\n");
    printf("1. Connect LM35 output to CHP0 (OPAMP input)\r\n");
    printf("2. Connect feedback resistors (R1=%lukΩ, R2=%lukΩ)\r\n", 
           R1_VALUE/1000, R2_VALUE/1000);
    printf("3. OPAMP output connects to ADC\r\n\r\n");
    
    printf("Benefits of signal conditioning:\r\n");
    printf("  - Without amplifier: 10mV/°C -> ADC resolution = 3.3V/1024 = 3.2mV\r\n");
    printf("  - With 10x amplifier: 100mV/°C -> Better resolution!\r\n");
    printf("  - Temperature resolution improved from 0.32°C to 0.032°C\r\n\r\n");
    
    printf("Reading temperature...\r\n");
    printf("Time(s) | Raw ADC | Amplified(V) | Sensor(mV) | Temp(°C)\r\n");
    printf("--------|---------|--------------|------------|----------\r\n");
    
    uint32_t count = 0;
    
    // สำหรับ averaging (ลด noise)
    #define AVG_SAMPLES 10
    
    while(1) {
        // อ่านค่า ADC แบบ average เพื่อลด noise
        uint16_t adc_avg = ADC_ReadAverage(ADC_CH_A0, AVG_SAMPLES);
        float amplified_voltage = ADC_ToVoltage(adc_avg, 3.3);
        
        // คำนวณแรงดันจาก sensor (ก่อนขยาย)
        float sensor_voltage_mv = (amplified_voltage / AMPLIFIER_GAIN) * 1000.0f;
        
        // แปลงเป็นอุณหภูมิ (LM35: 10mV/°C)
        float temperature = sensor_voltage_mv / LM35_MV_PER_CELSIUS;
        
        // แสดงผล
        printf("%7lu | %7u | %12.3f | %10.1f | %8.2f\r\n",
               count,
               adc_avg,
               amplified_voltage,
               sensor_voltage_mv,
               temperature);
        
        // ตรวจสอบช่วงอุณหภูมิ
        if(temperature < TEMP_MIN || temperature > TEMP_MAX) {
            printf("WARNING: Temperature out of range!\r\n");
        }
        
        // ตรวจสอบ saturation
        if(amplified_voltage > 3.2f) {
            printf("WARNING: Amplifier saturated! Temperature too high.\r\n");
        }
        
        // แจ้งเตือนอุณหภูมิสูง
        if(temperature > 50.0f) {
            printf("ALERT: High temperature detected! (%.1f°C)\r\n", temperature);
        }
        
        count++;
        Delay_Ms(1000);  // อ่านทุก 1 วินาที
    }
}
