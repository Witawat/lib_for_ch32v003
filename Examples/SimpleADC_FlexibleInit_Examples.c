/**
 * @file SimpleADC_FlexibleInit_Examples.c
 * @brief ตัวอย่างการใช้งาน ADC แบบยืดหยุ่น
 * @version 1.0
 * @date 2025-12-12
 */

#include "../SimpleHAL.h"

/* ========== วิธีที่ 1: เปิดทุก Channels (แบบเดิม) ========== */

/**
 * @brief เปิดใช้งาน ADC ทุกช่อง
 * 
 * @note ใช้เมื่อต้องการใช้งานหลายช่อง หรือไม่แน่ใจว่าจะใช้ช่องไหนบ้าง
 */
void Example_ADC_AllChannels(void) {
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // เปิดทุก channels (A0-A7)
    ADC_SimpleInit();
    
    USART_Print("=== All Channels Enabled ===\r\n");
    
    while(1) {
        // อ่านได้ทุกช่อง
        uint16_t a0 = ADC_Read(ADC_CH_A0);
        uint16_t a1 = ADC_Read(ADC_CH_A1);
        uint16_t a2 = ADC_Read(ADC_CH_A2);
        
        USART_Print("A0: ");
        USART_PrintNum(a0);
        USART_Print(", A1: ");
        USART_PrintNum(a1);
        USART_Print(", A2: ");
        USART_PrintNum(a2);
        USART_Print("\r\n");
        
        Delay_Ms(1000);
    }
}

/* ========== วิธีที่ 2: เปิดเฉพาะช่องที่ใช้ (แนะนำ) ========== */

/**
 * @brief เปิดเฉพาะช่องที่ต้องการใช้
 * 
 * @note ประหยัดกว่า เพราะเปิดเฉพาะ GPIO pins ที่จำเป็น
 */
void Example_ADC_SelectedChannels(void) {
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // กำหนดช่องที่ต้องการใช้
    ADC_Channel my_channels[] = {ADC_CH_A0, ADC_CH_A1};
    
    // เปิดเฉพาะช่องที่ระบุ
    ADC_SimpleInitChannels(my_channels, 2);
    
    USART_Print("=== Only A0 and A1 Enabled ===\r\n");
    
    while(1) {
        // อ่านได้เฉพาะช่องที่เปิด
        uint16_t a0 = ADC_Read(ADC_CH_A0);
        uint16_t a1 = ADC_Read(ADC_CH_A1);
        
        USART_Print("A0: ");
        USART_PrintNum(a0);
        USART_Print(", A1: ");
        USART_PrintNum(a1);
        USART_Print("\r\n");
        
        Delay_Ms(1000);
    }
}

/* ========== วิธีที่ 3: เปิดช่องเดียว ========== */

/**
 * @brief เปิดเฉพาะช่องเดียว
 * 
 * @note เหมาะกับการใช้งาน sensor เดียว
 */
void Example_ADC_SingleChannel(void) {
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // เปิดเฉพาะ A0
    ADC_Channel ch[] = {ADC_CH_A0};
    ADC_SimpleInitChannels(ch, 1);
    
    USART_Print("=== Only A0 Enabled ===\r\n");
    
    while(1) {
        uint16_t value = ADC_Read(ADC_CH_A0);
        float voltage = ADC_ToVoltage(value, 3.3);
        
        USART_Print("Voltage: ");
        USART_PrintNum((int)(voltage * 1000));
        USART_Print(" mV\r\n");
        
        Delay_Ms(500);
    }
}

/* ========== วิธีที่ 4: เปิดเพิ่มทีหลัง ========== */

/**
 * @brief เปิดช่องเพิ่มเติมภายหลัง
 * 
 * @note ใช้เมื่อต้องการเพิ่มช่องระหว่างการทำงาน
 */
void Example_ADC_AddChannelLater(void) {
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // เริ่มต้นด้วย A0 อย่างเดียว
    ADC_Channel ch[] = {ADC_CH_A0};
    ADC_SimpleInitChannels(ch, 1);
    
    USART_Print("=== Starting with A0 only ===\r\n");
    
    // อ่าน A0 สักพัก
    for(int i = 0; i < 5; i++) {
        uint16_t a0 = ADC_Read(ADC_CH_A0);
        USART_Print("A0: ");
        USART_PrintNum(a0);
        USART_Print("\r\n");
        Delay_Ms(500);
    }
    
    // เปิด A1 เพิ่ม
    USART_Print("\r\n=== Adding A1 ===\r\n");
    ADC_EnableChannel(ADC_CH_A1);
    
    // ตอนนี้อ่านได้ทั้ง A0 และ A1
    while(1) {
        uint16_t a0 = ADC_Read(ADC_CH_A0);
        uint16_t a1 = ADC_Read(ADC_CH_A1);
        
        USART_Print("A0: ");
        USART_PrintNum(a0);
        USART_Print(", A1: ");
        USART_PrintNum(a1);
        USART_Print("\r\n");
        
        Delay_Ms(1000);
    }
}

/* ========== ตัวอย่างการใช้งานจริง ========== */

/**
 * @brief ตัวอย่าง: อ่านค่า Potentiometer 2 ตัว
 */
void Example_ADC_TwoPotentiometers(void) {
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // เปิดเฉพาะ A0 และ A1 สำหรับ potentiometers
    ADC_Channel pots[] = {ADC_CH_A0, ADC_CH_A1};
    ADC_SimpleInitChannels(pots, 2);
    
    USART_Print("=== Two Potentiometers ===\r\n");
    
    while(1) {
        // อ่านค่า potentiometers
        uint16_t pot1 = ADC_ReadAverage(ADC_CH_A0, 5);
        uint16_t pot2 = ADC_ReadAverage(ADC_CH_A1, 5);
        
        // แปลงเป็นเปอร์เซ็นต์
        float percent1 = ADC_ToPercent(pot1);
        float percent2 = ADC_ToPercent(pot2);
        
        // แสดงผล
        USART_Print("Pot1: ");
        USART_PrintNum((int)percent1);
        USART_Print("%, Pot2: ");
        USART_PrintNum((int)percent2);
        USART_Print("%\r\n");
        
        Delay_Ms(200);
    }
}

/**
 * @brief ตัวอย่าง: ระบบตรวจสอบแบตเตอรี่
 */
void Example_ADC_BatteryMonitor(void) {
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // เปิดเฉพาะ A0 สำหรับวัดแรงดันแบตเตอรี่
    ADC_Channel battery_ch[] = {ADC_CH_A0};
    ADC_SimpleInitChannels(battery_ch, 1);
    
    USART_Print("=== Battery Monitor ===\r\n");
    
    const float BATTERY_MAX = 4.2;  // Li-ion full charge
    const float BATTERY_MIN = 3.0;  // Li-ion empty
    
    while(1) {
        // อ่านแรงดันแบตเตอรี่ (ผ่าน voltage divider)
        float voltage = ADC_ReadVoltage(ADC_CH_A0, 3.3);
        
        // คำนวณเปอร์เซ็นต์แบตเตอรี่
        float battery_percent = ((voltage - BATTERY_MIN) / (BATTERY_MAX - BATTERY_MIN)) * 100.0;
        if(battery_percent > 100) battery_percent = 100;
        if(battery_percent < 0) battery_percent = 0;
        
        // แสดงผล
        USART_Print("Battery: ");
        USART_PrintNum((int)(voltage * 100));
        USART_Print(" V (");
        USART_PrintNum((int)battery_percent);
        USART_Print("%)\r\n");
        
        // เตือนถ้าแบตต่ำ
        if(battery_percent < 20) {
            USART_Print("WARNING: Low battery!\r\n");
        }
        
        Delay_Ms(5000);
    }
}

/**
 * @brief ตัวอย่าง: อ่านค่า Temperature + Light sensor
 */
void Example_ADC_MultiSensor(void) {
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // เปิด A0 สำหรับ LM35, A1 สำหรับ LDR
    ADC_Channel sensors[] = {ADC_CH_A0, ADC_CH_A1};
    ADC_SimpleInitChannels(sensors, 2);
    
    USART_Print("=== Multi-Sensor System ===\r\n");
    
    while(1) {
        // อ่านอุณหภูมิจาก LM35 (A0)
        float temp_voltage = ADC_ReadVoltage(ADC_CH_A0, 3.3);
        float temperature = temp_voltage * 100.0;  // 10mV/°C
        
        // อ่านความสว่างจาก LDR (A1)
        uint16_t light = ADC_ReadAverage(ADC_CH_A1, 10);
        float light_percent = ADC_ToPercent(light);
        
        // แสดงผล
        USART_Print("Temp: ");
        USART_PrintNum((int)temperature);
        USART_Print(" C, Light: ");
        USART_PrintNum((int)light_percent);
        USART_Print("%\r\n");
        
        Delay_Ms(1000);
    }
}

/* ========== Main Function ========== */

void ADC_FlexibleInit_Examples_Main(void) {
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เลือกตัวอย่างที่ต้องการทดสอบ
    
    // Example_ADC_AllChannels();
    // Example_ADC_SelectedChannels();
    // Example_ADC_SingleChannel();
    // Example_ADC_AddChannelLater();
    
    // ตัวอย่างการใช้งานจริง
    // Example_ADC_TwoPotentiometers();
    // Example_ADC_BatteryMonitor();
    Example_ADC_MultiSensor();
}
