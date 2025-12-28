# SimpleHAL Library

Simple Hardware Abstraction Layer สำหรับ CH32V003 - ใช้งานง่ายแบบ Arduino

## 📁 โครงสร้างไฟล์

```
SimpleHAL/
├── SimpleGPIO.h/.c         # GPIO และ Interrupt control
├── SimpleADC.h/.c          # ADC (Analog to Digital Converter)
├── SimplePWM.h/.c          # PWM output
├── SimpleTIM.h/.c          # Timer peripherals
├── SimpleUSART.h/.c        # Serial communication
├── SimpleI2C.h/.c          # I2C communication
├── SimpleSPI.h/.c          # SPI communication
├── SimpleFlash.h/.c        # Flash memory storage
├── SimpleIWDG.h/.c         # Independent Watchdog
├── SimpleWWDG.h/.c         # Window Watchdog
├── SimpleHAL.h             # Header รวมทั้งหมด
└── Examples/               # ตัวอย่างการใช้งาน
    ├── SimpleGPIO_Examples.c
    ├── SimpleADC_Examples.c
    ├── SimpleADC_FlexibleInit_Examples.c
    ├── SimplePWM_Examples.c
    ├── SimpleTIM_Examples.c
    ├── SimpleHAL_Examples.c
    ├── IWDG/
    │   ├── 01_Basic_IWDG.c
    │   ├── 02_System_Recovery.c
    │   ├── 03_MultiTask_Monitor.c
    │   └── README.md
    └── WWDG/
        ├── 01_Basic_WWDG.c
        ├── 02_WWDG_Interrupt.c
        ├── 03_Critical_Timing.c
        └── README.md
```

## 🚀 การใช้งาน

### วิธีที่ 1: Include เฉพาะที่ต้องการ

```c
#include "SimpleHAL/SimpleGPIO.h"
#include "SimpleHAL/timer.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    pinMode(PC0, PIN_MODE_OUTPUT);
    digitalWrite(PC0, HIGH);
    
    while(1) {
        digitalToggle(PC0);
        Delay_Ms(500);
    }
}
```

### วิธีที่ 2: Include ทั้งหมด

```c
#include "SimpleHAL/SimpleHAL.h"

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // ใช้งาน GPIO
    pinMode(PC0, PIN_MODE_OUTPUT);
    
    // ใช้งาน USART
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    USART_Print("Hello World!\r\n");
    
    // ใช้งาน I2C
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
    
    while(1) {
        // Your code here
    }
}
```

## 📚 Peripherals ที่รองรับ

| Peripheral | Header | คำอธิบาย |
|-----------|--------|----------|
| **GPIO** | `SimpleGPIO.h` | Digital I/O, Interrupts |
| **ADC** | `SimpleADC.h` | อ่านค่า Analog |
| **PWM** | `SimplePWM.h` | PWM output control (8 channels) |
| **OPAMP** | `SimpleOPAMP.h` | Operational Amplifier (ขยายสัญญาณ, buffer) |
| **Flash** | `SimpleFlash.h` | Flash memory storage (config/data) |
| **TIM** | `SimpleTIM.h` | Timer interrupts |
| **TIM Ext** | `SimpleTIM_Ext.h` | Stopwatch และ Countdown timers |
| **USART** | `SimpleUSART.h` | Serial communication |
| **I2C** | `SimpleI2C.h` | I2C สำหรับ sensors, EEPROM |
| **SPI** | `SimpleSPI.h` | SPI communication |
| **IWDG** | `SimpleIWDG.h` | Independent Watchdog (ป้องกันระบบค้าง) |
| **WWDG** | `SimpleWWDG.h` | Window Watchdog (ตรวจสอบ timing) |
| **Timer** | `timer.h` | Delay และ timing functions |

## 📖 ตัวอย่างการใช้งาน

ตัวอย่างทั้งหมดอยู่ในโฟลเดอร์ `Examples/`:

### GPIO Examples
- LED Blink (พื้นฐาน)
- Button Reading
- Interrupts
- Multiple LEDs
- Port Write

### ADC Examples
- อ่านค่า ADC ช่องเดียว/หลายช่อง
- Voltage Monitoring
- Potentiometer Reader
- Temperature Sensor (LM35)
- Light Sensor (LDR)

### PWM Examples
- LED Fade
- Servo Control
- RGB LED
- Motor Speed Control
- Breathing LED Effect

### Timer Examples
- Basic Timer Interrupt
- LED Blink with Timer
- Multiple Timers
- Stopwatch
- Task Scheduler

### Communication Examples
- USART: Serial communication
- I2C: EEPROM, I2C Scanner
- SPI: Basic transfer, Buffer transfer

## 🔧 คุณสมบัติหลัก

- ✅ **SimpleGPIO**: Digital I/O และ Interrupts
- ✅ **SimpleADC**: อ่านค่า Analog
- ✅ **SimplePWM**: PWM output control
- ✅ **SimpleOPAMP**: Operational Amplifier (ขยายสัญญาณ, buffer, comparator)
- ✅ **SimpleFlash**: Flash memory storage (configuration และข้อมูล)
- ✅ **SimpleTIM**: Timer peripherals
- ✅ **SimpleTIM_Ext**: Stopwatch และ Countdown
- ✅ **SimpleUSART**: Serial communication
- ✅ **SimpleI2C**: I2C communication
- ✅ **SimpleSPI**: SPI communication
- ✅ **SimpleIWDG**: Independent Watchdog (ป้องกันระบบค้าง)
- ✅ **SimpleWWDG**: Window Watchdog (ตรวจสอบ timing เข้มงวด)

## 📌 Pin Mapping

SimpleHAL ใช้ชื่อ pin แบบ CH32V003 native:

### GPIO Pins
```
GPIOA: PA1, PA2
GPIOC: PC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7
GPIOD: PD2, PD3, PD4, PD5, PD6, PD7
```

### PWM Support
- **PA1**: PWM1_CH2
- **PC0**: PWM2_CH3
- **PC3**: PWM1_CH3
- **PC4**: PWM1_CH4
- **PD2**: PWM1_CH1
- **PD3**: PWM2_CH2
- **PD4**: PWM2_CH1
- **PD7**: PWM2_CH4

### ADC Support (PD pins only)
- **PD2-PD7**: ADC channels (PA และ PC ไม่รองรับ ADC)

## 🔗 Dependencies

SimpleHAL ต้องการ:
- `User/Lib/Timer/` - สำหรับ delay และ timing functions
- CH32V003 Peripheral Library

## 📝 License

MIT License

## 👨‍💻 Version

- **Version:** 1.8.0
- **Last Updated:** 2025-12-21
- **New in 1.8.0:** เพิ่มฟังก์ชัน Advanced GPIO: digitalWriteMultiple(), pulseIn(), shiftOut(), shiftIn() สำหรับควบคุม GPIO ขั้นสูง (ultrasonic sensor, shift register, software SPI)
