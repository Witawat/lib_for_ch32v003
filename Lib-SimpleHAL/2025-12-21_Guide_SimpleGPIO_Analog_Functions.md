# SimpleGPIO Analog Functions - คู่มือการใช้งาน

**Version:** 1.0  
**Date:** 2025-12-21  
**Category:** Library Usage Guide

---

## สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [Pin Compatibility](#pin-compatibility)
3. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
4. [การใช้งานขั้นสูง](#การใช้งานขั้นสูง)
5. [API Reference](#api-reference)
6. [Best Practices](#best-practices)
7. [Troubleshooting](#troubleshooting)

---

## ภาพรวม

SimpleGPIO เวอร์ชัน 1.2 เพิ่มฟังก์ชัน analog I/O แบบ Arduino-style:

- **`analogRead(pin)`** - อ่านค่า ADC (0-1023)
- **`analogWrite(pin, value)`** - สร้าง PWM (0-255)

### คุณสมบัติหลัก

✅ **Arduino-Compatible** - ใช้งานเหมือน Arduino  
✅ **Auto-Initialization** - ไม่ต้อง init ADC/PWM เอง  
✅ **Pin Mapping** - แมป GPIO pin → ADC/PWM อัตโนมัติ  
✅ **Error Handling** - ตรวจสอบ pin ที่ไม่รองรับ  
✅ **Single Header** - include `SimpleHAL.h` เพียงอย่างเดียว

---

## Pin Compatibility

### ADC Pins (analogRead)

| GPIO Pin | ADC Channel | Voltage Range | Notes |
|----------|-------------|---------------|-------|
| **PD2** | CH3 | 0-3.3V | ✅ รองรับ PWM ด้วย |
| **PD3** | CH4 | 0-3.3V | ✅ รองรับ PWM ด้วย |
| **PD4** | CH7 | 0-3.3V | ✅ รองรับ PWM ด้วย |
| **PD5** | CH5 | 0-3.3V | ADC only |
| **PD6** | CH6 | 0-3.3V | ADC only |
| **PD7** | CH0 | 0-3.3V | ✅ รองรับ PWM ด้วย |

### PWM Pins (analogWrite)

| GPIO Pin | PWM Channel | Timer | Default Freq | Notes |
|----------|-------------|-------|--------------|-------|
| **PC0** | PWM2_CH3 | TIM2 | 1kHz | PWM only |
| **PC3** | PWM1_CH3 | TIM1 | 1kHz | PWM only |
| **PC4** | PWM1_CH4 | TIM1 | 1kHz | PWM only |
| **PD2** | PWM1_CH1 | TIM1 | 1kHz | ✅ รองรับ ADC ด้วย |
| **PD3** | PWM2_CH2 | TIM2 | 1kHz | ✅ รองรับ ADC ด้วย |
| **PD4** | PWM2_CH1 | TIM2 | 1kHz | ✅ รองรับ ADC ด้วย |
| **PD7** | PWM2_CH4 | TIM2 | 1kHz | ✅ รองรับ ADC ด้วย |

> [!WARNING]
> Pin เดียวกันไม่สามารถใช้ ADC และ PWM **พร้อมกัน** ได้

---

## การใช้งานพื้นฐาน

### 1. อ่านค่า ADC (analogRead)

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    while(1) {
        // อ่านค่า ADC จาก PD2 (0-1023)
        uint16_t value = analogRead(PD2);
        
        // แปลงเป็น voltage
        float voltage = (value / 1023.0) * 3.3;
        
        printf("ADC: %u, Voltage: %.2fV\n", value, voltage);
        Delay_Ms(100);
    }
}
```

**อธิบาย:**
- ไม่ต้อง init ADC - `analogRead()` จะ init อัตโนมัติ
- Return ค่า 0-1023 (10-bit ADC)
- ถ้า pin ไม่รองรับ ADC จะ return 0

---

### 2. สร้าง PWM (analogWrite)

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // LED fade in/out
    uint8_t brightness = 0;
    int8_t direction = 1;
    
    while(1) {
        // เขียนค่า PWM (0-255)
        analogWrite(PC3, brightness);
        
        brightness += direction * 5;
        
        if (brightness >= 250) direction = -1;
        if (brightness <= 5) direction = 1;
        
        Delay_Ms(50);
    }
}
```

**อธิบาย:**
- ไม่ต้อง init PWM - `analogWrite()` จะ init อัตโนมัติที่ 1kHz
- ค่า 0 = 0% duty cycle (off)
- ค่า 255 = 100% duty cycle (full on)
- ถ้า pin ไม่รองรับ PWM จะไม่ทำอะไร

---

### 3. Potentiometer Control LED

```c
#include "SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    while(1) {
        // อ่านค่า potentiometer (PD2)
        uint16_t pot = analogRead(PD2);
        
        // แปลงเป็น PWM value (0-255)
        uint8_t brightness = (pot * 255) / 1023;
        
        // ควบคุม LED (PC3)
        analogWrite(PC3, brightness);
        
        Delay_Ms(50);
    }
}
```

---

## การใช้งานขั้นสูง

### 1. Multi-Channel ADC Reading

```c
#include "SimpleHAL.h"

typedef struct {
    uint8_t pin;
    uint16_t value;
    float voltage;
} ADC_Reading_t;

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    ADC_Reading_t sensors[] = {
        {PD2, 0, 0.0},
        {PD3, 0, 0.0},
        {PD4, 0, 0.0}
    };
    
    while(1) {
        // อ่านค่าจากทุก sensors
        for (uint8_t i = 0; i < 3; i++) {
            sensors[i].value = analogRead(sensors[i].pin);
            sensors[i].voltage = (sensors[i].value / 1023.0) * 3.3;
            
            printf("Sensor %u: %u (%.2fV)\n", 
                   i, sensors[i].value, sensors[i].voltage);
        }
        
        printf("\n");
        Delay_Ms(500);
    }
}
```

---

### 2. RGB LED Control

```c
#include "SimpleHAL.h"

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(PC3, r);  // Red
    analogWrite(PC4, g);  // Green
    analogWrite(PD4, b);  // Blue
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    while(1) {
        // Rainbow effect
        for (uint16_t hue = 0; hue < 360; hue += 5) {
            // Simple HSV to RGB conversion
            uint8_t r, g, b;
            
            if (hue < 120) {
                r = 255 - (hue * 255 / 120);
                g = hue * 255 / 120;
                b = 0;
            } else if (hue < 240) {
                r = 0;
                g = 255 - ((hue - 120) * 255 / 120);
                b = (hue - 120) * 255 / 120;
            } else {
                r = (hue - 240) * 255 / 120;
                g = 0;
                b = 255 - ((hue - 240) * 255 / 120);
            }
            
            setRGB(r, g, b);
            Delay_Ms(20);
        }
    }
}
```

---

### 3. Temperature Sensor with Fan Control

```c
#include "SimpleHAL.h"

#define TEMP_SENSOR_PIN  PD2
#define FAN_PWM_PIN      PD4

float readTemperature(void) {
    uint16_t adc = analogRead(TEMP_SENSOR_PIN);
    float voltage = (adc / 1023.0) * 3.3;
    // LM35: 10mV/°C
    return voltage * 100.0;
}

uint8_t calculateFanSpeed(float temp) {
    if (temp < 25.0) return 0;
    if (temp < 30.0) return 64;   // 25%
    if (temp < 35.0) return 128;  // 50%
    if (temp < 40.0) return 192;  // 75%
    return 255;  // 100%
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    while(1) {
        float temp = readTemperature();
        uint8_t fan_speed = calculateFanSpeed(temp);
        
        analogWrite(FAN_PWM_PIN, fan_speed);
        
        printf("Temp: %.1f°C, Fan: %u%%\n", 
               temp, (fan_speed * 100) / 255);
        
        Delay_Ms(1000);
    }
}
```

---

### 4. Servo Control with Position Feedback

```c
#include "SimpleHAL.h"

#define SERVO_PIN     PD4
#define POT_PIN       PD2

void setServoAngle(uint8_t angle) {
    // Servo: 0° = 5%, 180° = 10%
    // PWM value = (5 + angle * 5 / 180) * 255 / 100
    uint8_t pwm = (5 + (angle * 5) / 180) * 255 / 100;
    analogWrite(SERVO_PIN, pwm);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Note: Servo ต้องการ 50Hz PWM
    // ต้องใช้ SimplePWM โดยตรงสำหรับความถี่ที่แม่นยำ
    PWM_Init(PWM2_CH1, 50);  // 50Hz for servo
    PWM_Start(PWM2_CH1);
    
    while(1) {
        // อ่านตำแหน่งที่ต้องการจาก potentiometer
        uint16_t pot = analogRead(POT_PIN);
        uint8_t angle = (pot * 180) / 1023;
        
        // ควบคุม servo
        uint8_t duty = 5 + (angle * 5) / 180;
        PWM_SetDutyCycle(PWM2_CH1, duty);
        
        printf("Angle: %u°\n", angle);
        Delay_Ms(20);
    }
}
```

---

## API Reference

### analogRead()

```c
uint16_t analogRead(uint8_t pin);
```

**Parameters:**
- `pin` - GPIO pin (PD2, PD3, PD4, PD5, PD6, PD7)

**Returns:**
- ADC value (0-1023)
- 0 ถ้า pin ไม่รองรับ ADC

**Notes:**
- ADC จะถูก init อัตโนมัติในครั้งแรก
- 10-bit resolution (0-1023)
- Voltage range: 0-3.3V

**Example:**
```c
uint16_t value = analogRead(PD2);
float voltage = (value / 1023.0) * 3.3;
```

---

### analogWrite()

```c
void analogWrite(uint8_t pin, uint8_t value);
```

**Parameters:**
- `pin` - GPIO pin (PC0, PC3, PC4, PD2, PD3, PD4, PD7)
- `value` - PWM value (0-255)

**Returns:**
- None

**Notes:**
- PWM จะถูก init อัตโนมัติที่ 1kHz
- 0 = 0% duty cycle (off)
- 255 = 100% duty cycle (full on)
- ถ้า pin ไม่รองรับ PWM จะไม่ทำอะไร

**Example:**
```c
analogWrite(PC3, 128);  // 50% duty cycle
analogWrite(PD2, 255);  // 100% duty cycle
```

---

### การปรับความถี่ PWM

> [!IMPORTANT]
> `analogWrite()` ใช้ความถี่เริ่มต้น **1000 Hz (1 kHz)** เมื่อ auto-init

#### วิธีที่ 1: Init ด้วยความถี่ที่ต้องการก่อน (แนะนำ)

```c
// Init PWM ด้วยความถี่ที่ต้องการก่อนใช้ analogWrite()
PWM_Init(PWM1_CH3, 25000);  // 25 kHz สำหรับ motor
PWM_Start(PWM1_CH3);

// จากนั้นใช้ analogWrite() ได้เลย (จะใช้ความถี่ที่ init ไว้)
analogWrite(PC3, 128);  // 50% duty @ 25kHz
```

#### วิธีที่ 2: เปลี่ยนความถี่หลัง auto-init

```c
// ใช้ analogWrite() ครั้งแรก (auto-init ที่ 1kHz)
analogWrite(PC3, 0);

// เปลี่ยนความถี่ทีหลัง
PWM_SetFrequency(PWM1_CH3, 25000);  // เปลี่ยนเป็น 25 kHz
PWM_Start(PWM1_CH3);

// ใช้งานต่อ
analogWrite(PC3, 128);  // 50% duty @ 25kHz
```

#### ตัวอย่างความถี่ที่แนะนำ

| Application | Frequency | Example |
|-------------|-----------|---------|
| LED Dimming | 500-2000 Hz | `PWM_Init(PWM1_CH3, 1000)` |
| Servo Motor | 50 Hz | `PWM_Init(PWM2_CH1, 50)` |
| DC Motor | 20-25 kHz | `PWM_Init(PWM2_CH1, 25000)` |
| Buzzer/Tone | 100-10000 Hz | `PWM_Init(PWM1_CH1, 440)` |

> [!WARNING]
> Channels บน timer เดียวกันต้องใช้ความถี่เดียวกัน
> - PWM1_CH1-4 ใช้ TIM1 (ต้องความถี่เดียวกัน)
> - PWM2_CH1-4 ใช้ TIM2 (ต้องความถี่เดียวกัน)


---

## Best Practices

### ✅ 1. Pin Planning

```c
// วางแผนการใช้ pin ก่อนเขียนโค้ด
/*
 * Pin Assignment:
 * PD2 - Temperature Sensor (ADC)
 * PD4 - Fan Control (PWM)
 * PD5 - Light Sensor (ADC)
 * PC3 - Status LED (PWM)
 */
```

### ✅ 2. Use #define for Pin Names

```c
#define TEMP_SENSOR   PD2
#define FAN_CONTROL   PD4
#define LIGHT_SENSOR  PD5
#define STATUS_LED    PC3

// ใช้งาน
uint16_t temp = analogRead(TEMP_SENSOR);
analogWrite(FAN_CONTROL, 128);
```

### ✅ 3. Voltage Conversion Helper

```c
float adcToVoltage(uint16_t adc) {
    return (adc / 1023.0) * 3.3;
}

float voltage = adcToVoltage(analogRead(PD2));
```

### ✅ 4. PWM Percentage Helper

```c
void setPWMPercent(uint8_t pin, uint8_t percent) {
    if (percent > 100) percent = 100;
    uint8_t pwm = (percent * 255) / 100;
    analogWrite(pin, pwm);
}

setPWMPercent(PC3, 75);  // 75% brightness
```

---

## Troubleshooting

### ปัญหา: analogRead() return 0 เสมอ

**สาเหตุ:**
- ใช้ pin ที่ไม่รองรับ ADC
- Pin ถูกใช้เป็น PWM อยู่

**วิธีแก้:**
```c
// ✅ ใช้ pin ที่รองรับ ADC
uint16_t value = analogRead(PD2);  // PD2-PD7 เท่านั้น

// ❌ ผิด - PC0 ไม่รองรับ ADC
uint16_t value = analogRead(PC0);  // return 0
```

---

### ปัญหา: analogWrite() ไม่ทำงาน

**สาเหตุ:**
- ใช้ pin ที่ไม่รองรับ PWM
- Pin ถูกใช้เป็น ADC อยู่

**วิธีแก้:**
```c
// ✅ ใช้ pin ที่รองรับ PWM
analogWrite(PC3, 128);  // PC0, PC3-4, PD2-4, PD7

// ❌ ผิด - PC1 ไม่รองรับ PWM
analogWrite(PC1, 128);  // ไม่ทำอะไร
```

---

### ปัญหา: ใช้ ADC และ PWM บน pin เดียวกัน

**สาเหตุ:**
- Pin เดียวกันไม่สามารถใช้ทั้ง ADC และ PWM พร้อมกัน

**วิธีแก้:**
```c
// ❌ ผิด - PD2 ใช้ทั้ง ADC และ PWM
uint16_t adc = analogRead(PD2);
analogWrite(PD2, 128);  // Conflict!

// ✅ ถูก - ใช้คนละ pin
uint16_t adc = analogRead(PD2);  // ADC
analogWrite(PC3, 128);           // PWM
```

---

### ปัญหา: ADC อ่านค่าไม่เสถียร

**สาเหตุ:**
- Noise จาก circuit
- ไม่มี filtering

**วิธีแก้:**
```c
// ใช้ averaging
uint16_t readADCAverage(uint8_t pin, uint8_t samples) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples; i++) {
        sum += analogRead(pin);
        Delay_Us(100);
    }
    return sum / samples;
}

uint16_t stable_value = readADCAverage(PD2, 10);
```

---

### ปัญหา: ต้องการเปลี่ยนความถี่ PWM

**สาเหตุ:**
- `analogWrite()` ใช้ความถี่เริ่มต้น 1kHz
- ต้องการความถี่อื่น (เช่น 25kHz สำหรับ motor, 50Hz สำหรับ servo)

**วิธีแก้:**
```c
// วิธีที่ 1: Init ก่อน (แนะนำ)
PWM_Init(PWM1_CH3, 25000);  // 25 kHz
PWM_Start(PWM1_CH3);
analogWrite(PC3, 128);      // ใช้ความถี่ที่ init ไว้

// วิธีที่ 2: เปลี่ยนทีหลัง
analogWrite(PC3, 0);                    // Auto-init ที่ 1kHz
PWM_SetFrequency(PWM1_CH3, 25000);     // เปลี่ยนเป็น 25kHz
PWM_Start(PWM1_CH3);
analogWrite(PC3, 128);
```

**ความถี่ที่แนะนำ:**
- LED: 500-2000 Hz
- Servo: 50 Hz
- DC Motor: 20-25 kHz
- Buzzer: 100-10000 Hz


---

## สรุป

SimpleGPIO analog functions ให้ความสามารถ:

✅ **Arduino-Compatible** - ใช้งานง่ายเหมือน Arduino  
✅ **Auto-Init** - ไม่ต้อง init ADC/PWM เอง  
✅ **6 ADC Channels** - PD2-PD7  
✅ **7 PWM Channels** - PC0, PC3-4, PD2-4, PD7  
✅ **Error Handling** - ตรวจสอบ pin อัตโนมัติ

**ข้อควรระวัง:**
- Pin เดียวกันไม่สามารถใช้ ADC และ PWM พร้อมกัน
- PWM default ที่ 1kHz (ใช้ SimplePWM โดยตรงสำหรับความถี่อื่น)

**ดูเพิ่มเติม:**
- [SimpleGPIO Usage Guide](file:///c:/Users/XSoFTz-PC/mounriver-studio-projects/CH32V003-main/readme/2025-12-21_Guide_SimpleGPIO_Usage.md)
- [SimpleADC Usage Guide](file:///c:/Users/XSoFTz-PC/mounriver-studio-projects/CH32V003-main/readme/2025-12-21_Guide_SimpleADC_Usage.md)
- [SimplePWM Usage Guide](file:///c:/Users/XSoFTz-PC/mounriver-studio-projects/CH32V003-main/readme/2025-12-21_Guide_SimplePWM_Usage.md)
- [Integration Guide](file:///c:/Users/XSoFTz-PC/mounriver-studio-projects/CH32V003-main/readme/2025-12-21_Guide_Integration_GPIO_PWM_ADC.md)
