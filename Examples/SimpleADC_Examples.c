/**
 * @file SimpleADC_Examples.c
 * @brief ตัวอย่างการใช้งาน SimpleADC Library
 * @version 1.1
 * @date 2025-12-22
 * 
 * @note อัปเดตให้ใช้ ADC channels ที่ถูกต้อง: PA1, PA2, PC4, PD2-PD6
 */

#include "../SimpleHAL.h"

/* ========== ตัวอย่างที่ 1: อ่าน ADC ช่องเดียว ========== */

/**
 * @brief ตัวอย่างการอ่าน ADC ช่องเดียว
 */
void Example_ADC_Single(void) {
    // เริ่มต้น ADC และ USART
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    USART_Print("=== ADC Single Channel Example ===\r\n");
    
    while(1) {
        // อ่านค่า ADC จาก PD2 (Channel 3)
        uint16_t adc_value = ADC_Read(ADC_CH_PD2);
        
        // แปลงเป็น voltage
        float voltage = ADC_ToVoltage(adc_value, 3.3);
        
        // แปลงเป็นเปอร์เซ็นต์
        float percent = ADC_ToPercent(adc_value);
        
        // แสดงผล
        USART_Print("PD2: ");
        USART_PrintNum(adc_value);
        USART_Print(" (");
        USART_PrintNum((int)(voltage * 1000));  // แสดงเป็น mV
        USART_Print(" mV, ");
        USART_PrintNum((int)percent);
        USART_Print("%)\r\n");
        
        Delay_Ms(500);
    }
}

/* ========== ตัวอย่างที่ 2: อ่าน ADC หลายช่อง ========== */

/**
 * @brief ตัวอย่างการอ่าน ADC หลายช่องพร้อมกัน
 */
void Example_ADC_Multiple(void) {
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    USART_Print("=== ADC Multiple Channels Example ===\r\n");
    
    // กำหนด channels ที่ต้องการอ่าน (PD2, PD3, PD4)
    ADC_Channel channels[] = {ADC_CH_PD2, ADC_CH_PD3, ADC_CH_PD4};
    uint16_t values[3];
    
    while(1) {
        // อ่านทุก channels พร้อมกัน
        ADC_ReadMultiple(channels, values, 3);
        
        // แสดงผล
        USART_Print("PD2: ");
        USART_PrintNum(values[0]);
        USART_Print(", PD3: ");
        USART_PrintNum(values[1]);
        USART_Print(", PD4: ");
        USART_PrintNum(values[2]);
        USART_Print("\r\n");
        
        Delay_Ms(1000);
    }
}

/* ========== ตัวอย่างที่ 3: อ่าน ADC แบบ Average ========== */

/**
 * @brief ตัวอย่างการอ่าน ADC แบบ average เพื่อลด noise
 */
void Example_ADC_Average(void) {
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    USART_Print("=== ADC Average Example ===\r\n");
    
    while(1) {
        // อ่านแบบปกติ
        uint16_t single = ADC_Read(ADC_CH_PD2);
        
        // อ่านแบบ average 10 ครั้ง
        uint16_t average = ADC_ReadAverage(ADC_CH_PD2, 10);
        
        // แสดงผล
        USART_Print("Single: ");
        USART_PrintNum(single);
        USART_Print(", Average(10): ");
        USART_PrintNum(average);
        USART_Print("\r\n");
        
        Delay_Ms(1000);
    }
}

/* ========== ตัวอย่างที่ 4: Voltage Monitoring ========== */

/**
 * @brief ตัวอย่างการตรวจสอบแรงดัน
 */
void Example_ADC_VoltageMonitor(void) {
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    USART_Print("=== Voltage Monitor Example ===\r\n");
    
    const float VREF = 3.3;  // แรงดันอ้างอิง
    const float THRESHOLD_LOW = 1.0;   // แรงดันต่ำสุด
    const float THRESHOLD_HIGH = 3.0;  // แรงดันสูงสุด
    
    while(1) {
        // อ่านแรงดันจาก PD2
        float voltage = ADC_ReadVoltage(ADC_CH_PD2, VREF);
        
        // ตรวจสอบแรงดัน
        if(voltage < THRESHOLD_LOW) {
            USART_Print("WARNING: Voltage too low! ");
        } else if(voltage > THRESHOLD_HIGH) {
            USART_Print("WARNING: Voltage too high! ");
        } else {
            USART_Print("OK: ");
        }
        
        // แสดงแรงดัน
        USART_PrintNum((int)(voltage * 1000));
        USART_Print(" mV\r\n");
        
        Delay_Ms(500);
    }
}

/* ========== ตัวอย่างที่ 5: Potentiometer Reader ========== */

/**
 * @brief ตัวอย่างการอ่านค่า Potentiometer
 */
void Example_ADC_Potentiometer(void) {
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    USART_Print("=== Potentiometer Reader ===\r\n");
    
    while(1) {
        // อ่านค่า potentiometer
        uint16_t adc = ADC_ReadAverage(ADC_CH_PD2, 5);
        float percent = ADC_ToPercent(adc);
        
        // แสดง progress bar
        USART_Print("[");
        int bars = (int)(percent / 5);  // 20 bars สำหรับ 100%
        for(int i = 0; i < 20; i++) {
            if(i < bars) {
                USART_Print("=");
            } else {
                USART_Print(" ");
            }
        }
        USART_Print("] ");
        USART_PrintNum((int)percent);
        USART_Print("%\r\n");
        
        Delay_Ms(200);
    }
}

/* ========== ตัวอย่างที่ 6: Temperature Sensor (LM35) ========== */

/**
 * @brief ตัวอย่างการอ่านอุณหภูมิจาก LM35
 * @note LM35: 10mV/°C
 */
void Example_ADC_Temperature(void) {
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    USART_Print("=== Temperature Sensor (LM35) ===\r\n");
    
    while(1) {
        // อ่านแรงดันจาก LM35
        float voltage = ADC_ReadVoltage(ADC_CH_PD2, 3.3);
        
        // แปลงเป็นอุณหภูมิ (10mV/°C)
        float temperature = voltage * 100.0;  // V * 100 = °C
        
        // แสดงผล
        USART_Print("Temperature: ");
        USART_PrintNum((int)(temperature * 10));  // แสดง 1 ทศนิยม
        USART_Print(" C\r\n");
        
        Delay_Ms(1000);
    }
}

/* ========== ตัวอย่างที่ 7: Light Sensor (LDR) ========== */

/**
 * @brief ตัวอย่างการอ่านความสว่างจาก LDR
 */
void Example_ADC_LightSensor(void) {
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    USART_Print("=== Light Sensor (LDR) ===\r\n");
    
    while(1) {
        // อ่านค่า LDR
        uint16_t light = ADC_ReadAverage(ADC_CH_PD2, 10);
        float percent = ADC_ToPercent(light);
        
        // กำหนดระดับความสว่าง
        const char* level;
        if(percent < 20) {
            level = "Dark";
        } else if(percent < 50) {
            level = "Dim";
        } else if(percent < 80) {
            level = "Bright";
        } else {
            level = "Very Bright";
        }
        
        // แสดงผล
        USART_Print("Light: ");
        USART_PrintNum((int)percent);
        USART_Print("% (");
        USART_Print(level);
        USART_Print(")\r\n");
        
        Delay_Ms(500);
    }
}

/* ========== ตัวอย่างที่ 8: Internal Vref Reading ========== */

/**
 * @brief ตัวอย่างการอ่าน Internal Vref และคำนวณ VDD จริง
 */
void Example_ADC_InternalVref(void) {
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    USART_Print("=== Internal Vref Reading ===\r\n");
    USART_Print("Reading internal Vref to calculate actual VDD\r\n\r\n");
    
    while(1) {
        // อ่านค่า Vrefint (raw ADC value)
        uint16_t vrefint = ADC_ReadVrefInt();
        
        // คำนวณ VDD จริง
        float vdd = ADC_GetVDD();
        
        // แสดงผล
        USART_Print("Vrefint ADC: ");
        USART_PrintNum(vrefint);
        USART_Print(" | VDD: ");
        USART_PrintNum((int)(vdd * 1000));
        USART_Print(" mV");
        
        // เปรียบเทียบกับ VDD ที่คาดหวัง (3.3V)
        float diff = vdd - 3.3f;
        if (diff > 0.1f) {
            USART_Print(" [HIGH]");
        } else if (diff < -0.1f) {
            USART_Print(" [LOW]");
        } else {
            USART_Print(" [OK]");
        }
        USART_Print("\r\n");
        
        Delay_Ms(1000);
    }
}

/* ========== ตัวอย่างที่ 9: Battery Monitor ========== */

/**
 * @brief ตัวอย่างการตรวจสอบแบตเตอรี่พร้อมแสดงเปอร์เซ็นต์
 */
void Example_ADC_BatteryMonitor(void) {
    ADC_SimpleInit();
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    USART_Print("=== Battery Monitor ===\r\n");
    USART_Print("Monitoring battery voltage and percentage\r\n\r\n");
    
    // กำหนดช่วงแรงดันสำหรับ Li-ion
    // เปลี่ยนค่าตามประเภทแบตที่ใช้:
    // - Li-ion/Li-Po: 4.2V (full) -> 3.0V (empty)
    // - Alkaline 2xAA: 3.2V (full) -> 2.0V (empty)
    // - CR2032: 3.0V (full) -> 2.0V (empty)
    const float V_MAX = 4.2;  // แบตเต็ม
    const float V_MIN = 3.0;  // แบตหมด
    
    while(1) {
        // วัด VDD จริง (แรงดันแบตเตอรี่)
        float vdd = ADC_GetVDD();
        
        // คำนวณเปอร์เซ็นต์
        float percent = ADC_GetBatteryPercent(vdd, V_MIN, V_MAX);
        
        // แสดงผล
        USART_Print("Battery: ");
        USART_PrintNum((int)(vdd * 1000));
        USART_Print(" mV (");
        USART_PrintNum((int)percent);
        USART_Print("%)");
        
        // แสดง progress bar
        USART_Print(" [");
        int bars = (int)(percent / 5);  // 20 bars สำหรับ 100%
        for(int i = 0; i < 20; i++) {
            if(i < bars) {
                USART_Print("=");
            } else {
                USART_Print(" ");
            }
        }
        USART_Print("]");
        
        // เตือนเมื่อแบตต่ำ
        if(percent < 20) {
            USART_Print(" [LOW BATTERY!]");
            // สามารถเพิ่มการกระทำอื่นๆ เช่น:
            // - กระพริบ LED เตือน
            // - ส่งข้อความแจ้งเตือน
            // - ลด clock speed เพื่อประหยัดพลังงาน
        } else if(percent < 50) {
            USART_Print(" [Medium]");
        } else {
            USART_Print(" [Good]");
        }
        USART_Print("\r\n");
        
        Delay_Ms(1000);
    }
}

/* ========== Main Function ========== */

/**
 * @brief ฟังก์ชัน main สำหรับทดสอบตัวอย่าง
 */
void SimpleADC_Examples_Main(void) {
    // เริ่มต้นระบบ
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เลือกตัวอย่างที่ต้องการทดสอบ
    
    // Example_ADC_Single();
    // Example_ADC_Multiple();
    // Example_ADC_Average();
    // Example_ADC_VoltageMonitor();
    Example_ADC_Potentiometer();
    // Example_ADC_Temperature();
    // Example_ADC_LightSensor();
    
    // ========== New: Battery Monitoring Examples ==========
    // Example_ADC_InternalVref();      // Example 8: อ่าน Vref และคำนวณ VDD
    // Example_ADC_BatteryMonitor();    // Example 9: ตรวจสอบแบตเตอรี่
}
