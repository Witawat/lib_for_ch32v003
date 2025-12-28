/**
 * @file 06_Advanced_Techniques.c
 * @brief เทคนิคขั้นสูงในการใช้งาน OPAMP
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * ตัวอย่างนี้แสดงเทคนิคขั้นสูงต่างๆ:
 * 
 * 1. **Dynamic Gain Switching**
 *    - สลับ gain ตามช่วงสัญญาณ
 *    - Auto-ranging amplifier
 * 
 * 2. **Offset Compensation**
 *    - แก้ไข DC offset
 *    - ปรับ zero point
 * 
 * 3. **Multi-stage Amplification**
 *    - ใช้ OPAMP หลายขั้น
 *    - เพิ่ม gain รวม
 * 
 * 4. **Noise Reduction**
 *    - เทคนิคการลด noise
 *    - Averaging และ filtering
 * 
 * **ตัวอย่างนี้:**
 * - Auto-ranging amplifier สำหรับวัดแรงดันหลายช่วง
 * - สลับ gain อัตโนมัติตามขนาดสัญญาณ
 * - ป้องกัน saturation
 */

#include "debug.h"
#include "SimpleHAL/SimpleOPAMP.h"
#include "SimpleHAL/SimpleADC.h"
#include "SimpleHAL/SimpleGPIO.h"

// Gain levels
typedef enum {
    GAIN_LOW = 0,    // Gain = 2  (สำหรับสัญญาณใหญ่)
    GAIN_MEDIUM,     // Gain = 5  (สำหรับสัญญาณปานกลาง)
    GAIN_HIGH        // Gain = 10 (สำหรับสัญญาณเล็ก)
} GainLevel;

// Gain configurations
typedef struct {
    GainLevel level;
    float gain;
    uint32_t r1;
    uint32_t r2;
    const char* name;
} GainConfig;

const GainConfig gain_configs[] = {
    {GAIN_LOW,    2.0f,  10000, 10000, "Low (2x)"},
    {GAIN_MEDIUM, 5.0f,  10000, 40000, "Medium (5x)"},
    {GAIN_HIGH,   10.0f, 10000, 90000, "High (10x)"}
};

// Thresholds for auto-ranging (% of full scale)
#define THRESHOLD_SWITCH_UP     80  // สลับไป gain สูงขึ้นเมื่อ < 80%
#define THRESHOLD_SWITCH_DOWN   95  // สลับไป gain ต่ำลงเมื่อ > 95%

static GainLevel current_gain_level = GAIN_MEDIUM;

void setGain(GainLevel level) {
    if(level != current_gain_level) {
        printf("\r\n>>> Switching gain to %s <<<\r\n", gain_configs[level].name);
        
        // Disable OPAMP
        OPAMP_Disable();
        Delay_Ms(10);
        
        // Reconfigure (ในการใช้งานจริง ต้องสลับความต้านทานด้วย relay หรือ analog switch)
        OPAMP_ConfigNonInverting(OPAMP_CHP0, OPAMP_CHN0);
        
        // Re-enable
        OPAMP_Enable();
        Delay_Ms(10);
        
        current_gain_level = level;
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART
    USART_Printf_Init(115200);
    printf("\r\n=== OPAMP Advanced Techniques Example ===\r\n");
    printf("Auto-Ranging Amplifier with Dynamic Gain Switching\r\n\r\n");
    
    // แสดง gain configurations
    printf("Available Gain Levels:\r\n");
    for(int i = 0; i < 3; i++) {
        printf("  %d. %s - R1=%lukΩ, R2=%lukΩ\r\n",
               i+1,
               gain_configs[i].name,
               gain_configs[i].r1/1000,
               gain_configs[i].r2/1000);
    }
    printf("\r\n");
    
    // เริ่มต้น ADC
    ADC_SimpleInit();
    
    // เริ่มต้น OPAMP
    printf("Initializing OPAMP...\r\n");
    OPAMP_ConfigNonInverting(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();
    printf("OPAMP Enabled\r\n\r\n");
    
    printf("Auto-Ranging Algorithm:\r\n");
    printf("  - If output < %d%% of full scale -> Increase gain\r\n", THRESHOLD_SWITCH_UP);
    printf("  - If output > %d%% of full scale -> Decrease gain\r\n", THRESHOLD_SWITCH_DOWN);
    printf("  - Prevents saturation while maximizing resolution\r\n\r\n");
    
    printf("Monitoring with auto-ranging...\r\n");
    printf("Time(s) | Input(V) | Output(V) | Gain | ADC%% | Status\r\n");
    printf("--------|----------|-----------|------|------|--------\r\n");
    
    uint32_t count = 0;
    
    while(1) {
        // อ่านค่า input (ADC_CH_A0) - แบบ average เพื่อลด noise
        uint16_t input_adc = ADC_ReadAverage(ADC_CH_A0, 5);
        float input_voltage = ADC_ToVoltage(input_adc, 3.3);
        
        // อ่านค่า output (ADC_CH_A1)
        uint16_t output_adc = ADC_ReadAverage(ADC_CH_A1, 5);
        float output_voltage = ADC_ToVoltage(output_adc, 3.3);
        
        // คำนวณ % ของ full scale
        float output_percent = (output_voltage / 3.3f) * 100.0f;
        
        // Auto-ranging logic
        const char* status = "OK";
        
        if(output_percent > THRESHOLD_SWITCH_DOWN && current_gain_level > GAIN_LOW) {
            // Output ใกล้ saturate -> ลด gain
            setGain(current_gain_level - 1);
            status = "Gain↓";
        }
        else if(output_percent < THRESHOLD_SWITCH_UP && current_gain_level < GAIN_HIGH) {
            // Output ต่ำเกินไป -> เพิ่ม gain
            setGain(current_gain_level + 1);
            status = "Gain↑";
        }
        
        // แสดงผล
        printf("%7lu | %8.3f | %9.3f | %4.0fx | %4.0f%% | %s\r\n",
               count / 2,
               input_voltage,
               output_voltage,
               gain_configs[current_gain_level].gain,
               output_percent,
               status);
        
        // คำนวณ actual gain
        float actual_gain = 0.0f;
        if(input_voltage > 0.01f) {
            actual_gain = output_voltage / input_voltage;
        }
        
        // ตรวจสอบความถูกต้อง
        float expected_gain = gain_configs[current_gain_level].gain;
        float gain_error = ((actual_gain - expected_gain) / expected_gain) * 100.0f;
        
        if(gain_error > 15.0f || gain_error < -15.0f) {
            printf("WARNING: Gain error = %.1f%%. Check resistors!\r\n", gain_error);
        }
        
        count++;
        Delay_Ms(500);
    }
}
