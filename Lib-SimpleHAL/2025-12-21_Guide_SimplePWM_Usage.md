# SimplePWM - คู่มือการใช้งานฉบับสมบูรณ์

**Version:** 1.0  
**Date:** 2025-12-21  
**Category:** Library Usage Guide

---

## สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [การติดตั้งและเริ่มต้นใช้งาน](#การติดตั้งและเริ่มต้นใช้งาน)
3. [ขั้นพื้นฐาน - Basic Usage](#ขั้นพื้นฐาน---basic-usage)
4. [ขั้นกลาง - Intermediate Usage](#ขั้นกลาง---intermediate-usage)
5. [ขั้นสูง - Advanced Usage](#ขั้นสูง---advanced-usage)
6. [เทคนิคพิเศษ](#เทคนิคพิเศษ)
7. [API Reference](#api-reference)
8. [Troubleshooting](#troubleshooting)

---

## ภาพรวม

SimplePWM เป็น library สำหรับควบคุม PWM output บน CH32V003 ที่ใช้งานง่าย รองรับ 8 channels จาก TIM1 และ TIM2

### คุณสมบัติหลัก
- ✅ 8 PWM channels (TIM1: 4, TIM2: 4)
- ✅ Duty cycle 0-100% หรือ raw value
- ✅ Frequency adjustment
- ✅ Arduino analogWrite() compatible
- ✅ Pin remapping support
- ✅ Polarity control

### PWM Channels และ Pins

#### TIM1 Channels (Default pins)
| Channel | Pin | Arduino Pin | Notes |
|---------|-----|-------------|-------|
| PWM1_CH1 | PD2 | A0 | Analog pin |
| PWM1_CH2 | PA1 | - | - |
| PWM1_CH3 | PC3 | D3 | Digital pin |
| PWM1_CH4 | PC4 | D4 | Digital pin |

#### TIM2 Channels (Default pins)
| Channel | Pin | Arduino Pin | Notes |
|---------|-----|-------------|-------|
| PWM2_CH1 | PD4 | A2 | Analog pin |
| PWM2_CH2 | PD3 | A1 | Analog pin |
| PWM2_CH3 | PC0 | D0 | Digital pin |
| PWM2_CH4 | PD7 | A7 | Analog pin |

> **⚠️ หมายเหตุ:** ห้ามใช้ SimpleTIM กับ timer เดียวกับ SimplePWM

---

## การติดตั้งและเริ่มต้นใช้งาน

### 1. Include Header
```c
#include "SimpleHAL.h"
// หรือ
#include "SimplePWM.h"
```

### 2. Setup พื้นฐาน
```c
#include "SimplePWM.h"
#include "timer.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // เริ่มต้น PWM ที่ 1kHz
    PWM_Init(PWM1_CH1, 1000);
    PWM_SetDutyCycle(PWM1_CH1, 50);  // 50%
    PWM_Start(PWM1_CH1);
    
    while(1);
}
```

---

## ขั้นพื้นฐาน - Basic Usage

### 1. LED Fade (ค่อยๆ สว่าง/มืด)

```c
#include "SimplePWM.h"
#include "timer.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // เริ่มต้น PWM ที่ 1kHz
    PWM_Init(PWM1_CH1, 1000);
    PWM_Start(PWM1_CH1);
    
    while(1) {
        // Fade in (0% → 100%)
        for (uint8_t i = 0; i <= 100; i++) {
            PWM_SetDutyCycle(PWM1_CH1, i);
            Delay_Ms(10);
        }
        
        // Fade out (100% → 0%)
        for (uint8_t i = 100; i > 0; i--) {
            PWM_SetDutyCycle(PWM1_CH1, i);
            Delay_Ms(10);
        }
    }
}
```

**อธิบาย:**
- `PWM_Init()` - เริ่มต้น PWM channel ด้วยความถี่
- `PWM_SetDutyCycle()` - ตั้งค่า duty cycle (0-100%)
- `PWM_Start()` - เริ่ม PWM output

---

### 2. Arduino analogWrite()

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    while(1) {
        // Fade in (0-255)
        for (uint16_t i = 0; i <= 255; i++) {
            PWM_Write(PWM1_CH1, i);  // Arduino-style
            Delay_Ms(5);
        }
        
        // Fade out (255-0)
        for (uint16_t i = 255; i > 0; i--) {
            PWM_Write(PWM1_CH1, i);
            Delay_Ms(5);
        }
    }
}
```

**ข้อดี:**
- ใช้งานเหมือน Arduino
- Auto-init ถ้ายังไม่ได้ init
- ค่า 0-255 (เหมือน Arduino)

---

### 3. Fixed Brightness

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    PWM_Init(PWM1_CH1, 1000);
    
    // ตั้งค่าความสว่างต่างๆ
    PWM_SetDutyCycle(PWM1_CH1, 25);   // 25% - มืด
    PWM_Start(PWM1_CH1);
    Delay_Ms(2000);
    
    PWM_SetDutyCycle(PWM1_CH1, 50);   // 50% - กลาง
    Delay_Ms(2000);
    
    PWM_SetDutyCycle(PWM1_CH1, 75);   // 75% - สว่าง
    Delay_Ms(2000);
    
    PWM_SetDutyCycle(PWM1_CH1, 100);  // 100% - สว่างสุด
    Delay_Ms(2000);
    
    PWM_Stop(PWM1_CH1);  // ปิด
    
    while(1);
}
```

---

### 4. Frequency Change

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    PWM_Init(PWM1_CH1, 1000);
    PWM_SetDutyCycle(PWM1_CH1, 50);
    PWM_Start(PWM1_CH1);
    
    uint32_t frequencies[] = {100, 500, 1000, 2000, 5000};
    
    while(1) {
        for (uint8_t i = 0; i < 5; i++) {
            printf("Frequency: %lu Hz\n", frequencies[i]);
            PWM_SetFrequency(PWM1_CH1, frequencies[i]);
            PWM_SetDutyCycle(PWM1_CH1, 50);
            PWM_Start(PWM1_CH1);
            Delay_Ms(2000);
        }
    }
}
```

---

## ขั้นกลาง - Intermediate Usage

### 1. Servo Motor Control

```c
#include "SimplePWM.h"
#include "timer.h"

// Servo ใช้ PWM 50Hz
// Pulse width: 1-2ms (5-10% duty @ 50Hz)
// 1.0ms = 0°, 1.5ms = 90°, 2.0ms = 180°

void setServoAngle(PWM_Channel channel, uint8_t angle) {
    // แปลงมุม (0-180°) เป็น duty cycle (5-10%)
    uint8_t duty = 5 + (angle * 5) / 180;
    PWM_SetDutyCycle(channel, duty);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Servo ต้องใช้ 50Hz
    PWM_Init(PWM1_CH1, 50);
    PWM_Start(PWM1_CH1);
    
    while(1) {
        // 0 degrees
        setServoAngle(PWM1_CH1, 0);
        printf("0°\n");
        Delay_Ms(1000);
        
        // 90 degrees
        setServoAngle(PWM1_CH1, 90);
        printf("90°\n");
        Delay_Ms(1000);
        
        // 180 degrees
        setServoAngle(PWM1_CH1, 180);
        printf("180°\n");
        Delay_Ms(1000);
    }
}
```

**Tips:**
- Servo ต้องใช้ 50Hz เสมอ
- Pulse width: 1-2ms
- ทดสอบหาค่าที่เหมาะสมกับ servo ของคุณ

---

### 2. RGB LED Control

```c
void setRGB(uint8_t red, uint8_t green, uint8_t blue) {
    PWM_SetDutyCycle(PWM1_CH1, red);
    PWM_SetDutyCycle(PWM1_CH2, green);
    PWM_SetDutyCycle(PWM1_CH3, blue);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Init RGB channels
    PWM_Init(PWM1_CH1, 1000);  // Red
    PWM_Init(PWM1_CH2, 1000);  // Green
    PWM_Init(PWM1_CH3, 1000);  // Blue
    
    PWM_Start(PWM1_CH1);
    PWM_Start(PWM1_CH2);
    PWM_Start(PWM1_CH3);
    
    while(1) {
        setRGB(100, 0, 0);    // Red
        Delay_Ms(1000);
        
        setRGB(0, 100, 0);    // Green
        Delay_Ms(1000);
        
        setRGB(0, 0, 100);    // Blue
        Delay_Ms(1000);
        
        setRGB(100, 100, 0);  // Yellow
        Delay_Ms(1000);
        
        setRGB(0, 100, 100);  // Cyan
        Delay_Ms(1000);
        
        setRGB(100, 0, 100);  // Magenta
        Delay_Ms(1000);
        
        setRGB(100, 100, 100); // White
        Delay_Ms(1000);
    }
}
```

---

### 3. Motor Speed Control

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    PWM_Init(PWM1_CH1, 1000);  // Motor PWM
    PWM_Start(PWM1_CH1);
    
    while(1) {
        // Accelerate
        printf("Accelerating...\n");
        for (uint8_t speed = 0; speed <= 100; speed += 5) {
            PWM_SetDutyCycle(PWM1_CH1, speed);
            printf("Speed: %u%%\n", speed);
            Delay_Ms(200);
        }
        
        Delay_Ms(2000);
        
        // Decelerate
        printf("Decelerating...\n");
        for (uint8_t speed = 100; speed > 0; speed -= 5) {
            PWM_SetDutyCycle(PWM1_CH1, speed);
            printf("Speed: %u%%\n", speed);
            Delay_Ms(200);
        }
        
        Delay_Ms(2000);
    }
}
```

---

### 4. Multiple PWM Channels

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Init multiple channels
    PWM_Init(PWM1_CH1, 1000);
    PWM_Init(PWM1_CH2, 1000);
    PWM_Init(PWM1_CH3, 1000);
    
    PWM_Start(PWM1_CH1);
    PWM_Start(PWM1_CH2);
    PWM_Start(PWM1_CH3);
    
    while(1) {
        // แต่ละ channel มี duty cycle ต่างกัน
        for (uint8_t i = 0; i <= 100; i++) {
            PWM_SetDutyCycle(PWM1_CH1, i);
            PWM_SetDutyCycle(PWM1_CH2, 100 - i);
            PWM_SetDutyCycle(PWM1_CH3, (i * 2) % 100);
            Delay_Ms(20);
        }
    }
}
```

---

## ขั้นสูง - Advanced Usage

### 1. Breathing LED Effect

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    PWM_Init(PWM1_CH1, 1000);
    PWM_Start(PWM1_CH1);
    
    while(1) {
        // Breathe in (exponential curve)
        for (uint8_t i = 0; i <= 100; i++) {
            // Quadratic curve สำหรับ smooth breathing
            uint8_t brightness = (i * i) / 100;
            PWM_SetDutyCycle(PWM1_CH1, brightness);
            Delay_Ms(15);
        }
        
        // Breathe out
        for (uint8_t i = 100; i > 0; i--) {
            uint8_t brightness = (i * i) / 100;
            PWM_SetDutyCycle(PWM1_CH1, brightness);
            Delay_Ms(15);
        }
        
        Delay_Ms(500);
    }
}
```

**เทคนิค:**
- ใช้ quadratic curve แทน linear
- ทำให้ breathing นุ่มนวลกว่า
- สามารถใช้ cubic curve ได้ด้วย

---

### 2. Advanced PWM Setup

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // ตั้งค่าแบบละเอียด
    // SystemCoreClock = 48MHz
    // PWM freq = 48MHz / ((prescaler+1) * (period+1))
    // Example: 1kHz = 48MHz / (48 * 1000)
    
    uint16_t prescaler = 47;
    uint16_t period = 999;
    uint16_t duty = 500;  // 50%
    
    PWM_AdvancedInit(PWM1_CH1, prescaler, period, duty);
    PWM_Start(PWM1_CH1);
    
    printf("PWM configured:\n");
    printf("  Prescaler: %u\n", prescaler);
    printf("  Period: %u\n", period);
    printf("  Duty: %u\n", duty);
    printf("  Frequency: %lu Hz\n", 
           SystemCoreClock / ((prescaler+1) * (period+1)));
    printf("  Duty cycle: %u%%\n", (duty * 100) / period);
    
    while(1);
}
```

---

### 3. PWM with Button Control

```c
volatile uint8_t brightness = 50;

void button_up_isr(void) {
    if (brightness < 100) {
        brightness += 10;
        PWM_SetDutyCycle(PWM1_CH1, brightness);
        printf("Brightness: %u%%\n", brightness);
    }
}

void button_down_isr(void) {
    if (brightness > 0) {
        brightness -= 10;
        PWM_SetDutyCycle(PWM1_CH1, brightness);
        printf("Brightness: %u%%\n", brightness);
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Setup PWM
    PWM_Init(PWM1_CH1, 1000);
    PWM_SetDutyCycle(PWM1_CH1, brightness);
    PWM_Start(PWM1_CH1);
    
    // Setup buttons
    pinMode(D1, PIN_MODE_INPUT_PULLUP);  // Up
    pinMode(D2, PIN_MODE_INPUT_PULLUP);  // Down
    
    attachInterrupt(D1, button_up_isr, FALLING);
    attachInterrupt(D2, button_down_isr, FALLING);
    
    printf("PWM Button Control\n");
    printf("D1: Increase, D2: Decrease\n");
    printf("Initial: %u%%\n", brightness);
    
    while(1) {
        Delay_Ms(100);
    }
}
```

---

### 4. Polarity Control

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    PWM_Init(PWM1_CH1, 1000);
    PWM_SetDutyCycle(PWM1_CH1, 30);
    PWM_Start(PWM1_CH1);
    
    while(1) {
        // Normal polarity (high = active)
        printf("Normal polarity\n");
        PWM_SetPolarity(PWM1_CH1, 0);
        Delay_Ms(2000);
        
        // Inverted polarity (low = active)
        printf("Inverted polarity\n");
        PWM_SetPolarity(PWM1_CH1, 1);
        Delay_Ms(2000);
    }
}
```

**เมื่อไหร่ใช้:**
- LED common anode
- Motor driver ที่ต้องการ inverted signal
- Logic level conversion

---

## เทคนิคพิเศษ

### 1. Smooth Color Transitions

```c
typedef struct {
    uint8_t r, g, b;
} RGB_t;

void smoothTransition(RGB_t from, RGB_t to, uint16_t duration_ms) {
    uint16_t steps = duration_ms / 10;
    
    for (uint16_t i = 0; i <= steps; i++) {
        uint8_t r = from.r + ((to.r - from.r) * i) / steps;
        uint8_t g = from.g + ((to.g - from.g) * i) / steps;
        uint8_t b = from.b + ((to.b - from.b) * i) / steps;
        
        PWM_SetDutyCycle(PWM1_CH1, r);
        PWM_SetDutyCycle(PWM1_CH2, g);
        PWM_SetDutyCycle(PWM1_CH3, b);
        
        Delay_Ms(10);
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    PWM_Init(PWM1_CH1, 1000);
    PWM_Init(PWM1_CH2, 1000);
    PWM_Init(PWM1_CH3, 1000);
    
    PWM_Start(PWM1_CH1);
    PWM_Start(PWM1_CH2);
    PWM_Start(PWM1_CH3);
    
    RGB_t red = {100, 0, 0};
    RGB_t green = {0, 100, 0};
    RGB_t blue = {0, 0, 100};
    
    while(1) {
        smoothTransition(red, green, 2000);
        smoothTransition(green, blue, 2000);
        smoothTransition(blue, red, 2000);
    }
}
```

---

### 2. PWM Frequency Sweep

```c
void frequencySweep(uint32_t start_freq, uint32_t end_freq, 
                   uint16_t duration_ms, uint8_t duty) {
    uint16_t steps = duration_ms / 100;
    
    for (uint16_t i = 0; i <= steps; i++) {
        uint32_t freq = start_freq + 
                       ((end_freq - start_freq) * i) / steps;
        
        PWM_SetFrequency(PWM1_CH1, freq);
        PWM_SetDutyCycle(PWM1_CH1, duty);
        PWM_Start(PWM1_CH1);
        
        printf("Frequency: %lu Hz\n", freq);
        Delay_Ms(100);
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    PWM_Init(PWM1_CH1, 100);
    
    while(1) {
        // Sweep up
        frequencySweep(100, 10000, 5000, 50);
        Delay_Ms(1000);
        
        // Sweep down
        frequencySweep(10000, 100, 5000, 50);
        Delay_Ms(1000);
    }
}
```

---

### 3. Duty Cycle Measurement

```c
uint8_t getCurrentDutyCycle(PWM_Channel channel) {
    uint16_t duty_raw = PWM_GetDutyCycleRaw(channel);
    uint16_t period = PWM_GetPeriod(channel);
    
    return (duty_raw * 100) / period;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    PWM_Init(PWM1_CH1, 1000);
    PWM_Start(PWM1_CH1);
    
    while(1) {
        for (uint8_t i = 0; i <= 100; i += 10) {
            PWM_SetDutyCycle(PWM1_CH1, i);
            Delay_Ms(500);
            
            uint8_t actual = getCurrentDutyCycle(PWM1_CH1);
            printf("Set: %u%%, Actual: %u%%\n", i, actual);
        }
    }
}
```

---

### 4. Multi-Channel Synchronization

```c
void setAllChannels(uint8_t duty) {
    // ตั้งค่าทุก channels พร้อมกัน
    PWM_SetDutyCycle(PWM1_CH1, duty);
    PWM_SetDutyCycle(PWM1_CH2, duty);
    PWM_SetDutyCycle(PWM1_CH3, duty);
    PWM_SetDutyCycle(PWM1_CH4, duty);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Init all channels with same frequency
    for (uint8_t ch = PWM1_CH1; ch <= PWM1_CH4; ch++) {
        PWM_Init(ch, 1000);
        PWM_Start(ch);
    }
    
    while(1) {
        for (uint8_t i = 0; i <= 100; i++) {
            setAllChannels(i);
            Delay_Ms(20);
        }
        
        for (uint8_t i = 100; i > 0; i--) {
            setAllChannels(i);
            Delay_Ms(20);
        }
    }
}
```

---

## API Reference

### PWM Channels
```c
// TIM1 Channels
PWM1_CH1  // PD2 (A0)
PWM1_CH2  // PA1
PWM1_CH3  // PC3 (D3)
PWM1_CH4  // PC4 (D4)

// TIM2 Channels
PWM2_CH1  // PD4 (A2)
PWM2_CH2  // PD3 (A1)
PWM2_CH3  // PC0 (D0)
PWM2_CH4  // PD7 (A7)
```

### Functions

#### PWM_Init()
```c
void PWM_Init(PWM_Channel channel, uint32_t frequency_hz);
```
เริ่มต้น PWM channel ด้วยความถี่

#### PWM_Start()
```c
void PWM_Start(PWM_Channel channel);
```
เริ่ม PWM output

#### PWM_Stop()
```c
void PWM_Stop(PWM_Channel channel);
```
หยุด PWM output

#### PWM_SetDutyCycle()
```c
void PWM_SetDutyCycle(PWM_Channel channel, uint8_t duty_percent);
```
ตั้งค่า duty cycle (0-100%)

#### PWM_SetDutyCycleRaw()
```c
void PWM_SetDutyCycleRaw(PWM_Channel channel, uint16_t duty_value);
```
ตั้งค่า duty cycle แบบ raw

#### PWM_SetFrequency()
```c
void PWM_SetFrequency(PWM_Channel channel, uint32_t frequency_hz);
```
เปลี่ยนความถี่ PWM

#### PWM_Write()
```c
void PWM_Write(PWM_Channel channel, uint8_t value);
```
Arduino-style analogWrite (0-255)

#### PWM_GetPeriod()
```c
uint16_t PWM_GetPeriod(PWM_Channel channel);
```
อ่านค่า period

#### PWM_GetDutyCycleRaw()
```c
uint16_t PWM_GetDutyCycleRaw(PWM_Channel channel);
```
อ่านค่า duty cycle (raw)

#### PWM_GetDutyCycle()
```c
uint8_t PWM_GetDutyCycle(PWM_Channel channel);
```
อ่านค่า duty cycle (%)

#### PWM_SetPolarity()
```c
void PWM_SetPolarity(PWM_Channel channel, uint8_t inverted);
```
ตั้งค่า polarity (0=normal, 1=inverted)

---

## Troubleshooting

### ปัญหา: PWM ไม่ทำงาน
**สาเหตุ:**
- ลืมเรียก `PWM_Start()`
- Pin ไม่ถูกต้อง

**วิธีแก้:**
```c
PWM_Init(PWM1_CH1, 1000);
PWM_SetDutyCycle(PWM1_CH1, 50);
PWM_Start(PWM1_CH1);  // ⚠️ ต้องมี!
```

---

### ปัญหา: LED ไม่สว่าง
**สาเหตุ:**
- Duty cycle = 0
- Polarity ผิด

**วิธีแก้:**
```c
// ตรวจสอบ duty cycle
PWM_SetDutyCycle(PWM1_CH1, 50);  // ต้อง > 0

// ลอง invert polarity
PWM_SetPolarity(PWM1_CH1, 1);
```

---

### ปัญหา: Servo ไม่เคลื่อนที่
**สาเหตุ:**
- ความถี่ไม่ใช่ 50Hz
- Duty cycle ไม่ถูกต้อง

**วิธีแก้:**
```c
// Servo ต้องใช้ 50Hz
PWM_Init(PWM1_CH1, 50);

// Duty cycle: 5-10%
PWM_SetDutyCycle(PWM1_CH1, 7);  // 90°
```

---

### ปัญหา: ความถี่ไม่ตรง
**สาเหตุ:**
- SystemCoreClock ไม่ถูกต้อง
- ความถี่สูงเกินไป

**วิธีแก้:**
```c
SystemCoreClockUpdate();
printf("Clock: %lu Hz\n", SystemCoreClock);

// ความถี่สูงสุด ≈ SystemCoreClock / 2
```

---

## สรุป

SimplePWM ให้ความสามารถ:
- ✅ 8 PWM channels
- ✅ Duty cycle และ frequency control
- ✅ Arduino analogWrite() compatible
- ✅ เหมาะสำหรับ LED, servo, motor

**Applications:**
- LED dimming และ RGB control
- Servo motor positioning
- DC motor speed control
- Audio tone generation
- Power supply regulation

**Next Steps:**
- ลองทำ RGB LED
- ควบคุม servo motor
- สร้าง motor controller!
