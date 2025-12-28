# SimpleOPAMP Library - คู่มือการใช้งาน

> **Simple OPAMP Library สำหรับ CH32V003**  
> ห่อหุ้ม OPAMP peripheral ให้ใช้งานง่ายแบบ Arduino-style

---

## 📑 สารบัญ

1. [OPAMP คืออะไร](#opamp-คืออะไร)
2. [โครงสร้าง OPAMP ใน CH32V003](#โครงสร้าง-opamp-ใน-ch32v003)
3. [การใช้งานพื้นฐาน](#การใช้งานพื้นฐาน)
4. [การใช้งานขั้นสูง](#การใช้งานขั้นสูง)
5. [เทคนิคขั้นสูง](#เทคนิคขั้นสูง)
6. [ตัวอย่างการใช้งาน](#ตัวอย่างการใช้งาน)
7. [การแก้ปัญหา](#การแก้ปัญหา)

---

## OPAMP คืออะไร

**Operational Amplifier (OPAMP)** คือวงจรขยายสัญญาณที่มีคุณสมบัติพิเศษ:

### คุณสมบัติหลัก
- **Gain สูงมาก** (>100,000 เท่า)
- **Input Impedance สูงมาก** (ไม่ดูดกระแส)
- **Output Impedance ต่ำมาก** (ขับโหลดได้ดี)
- **Bandwidth กว้าง** (ทำงานได้หลายความถี่)

### การทำงาน
```
Output = Gain × (V+ - V-)
```
- `V+` = แรงดันที่ positive input
- `V-` = แรงดันที่ negative input
- `Gain` = อัตราขยาย (กำหนดโดยวงจรภายนอก)

---

## โครงสร้าง OPAMP ใน CH32V003

### Pin Configuration

CH32V003 มี OPAMP 1 ตัว ที่สามารถเลือก input channels ได้:

```
Positive Inputs:
  ├─ CHP0 - Positive Channel 0
  └─ CHP1 - Positive Channel 1

Negative Inputs:
  ├─ CHN0 - Negative Channel 0
  └─ CHN1 - Negative Channel 1

Output:
  └─ สามารถอ่านผ่าน ADC หรือ route ไปยัง GPIO/TIM2
```

### การเชื่อมต่อ

OPAMP สามารถเชื่อมต่อกับ:
- **ADC** - อ่านค่า output
- **TIM2** - ใช้กับ timer
- **GPIO** - output เป็น digital signal

---

## การใช้งานพื้นฐาน

### 1. Voltage Follower (Buffer)

**วัตถุประสงค์:** Buffer สัญญาณ, แยก impedance

**วงจร:**
```
Input ──→ CHP0 (+)
            │
            ▼
         OPAMP
            │
            ▼
         Output ──┬──→ Load
                  │
                  └──→ CHN0 (-) [External wire]
```

**สูตร:**
```
Vout = Vin
Gain = 1
```

**โค้ด:**
```c
#include "SimpleHAL/SimpleOPAMP.h"

int main(void) {
    // เริ่มต้น OPAMP เป็น voltage follower
    OPAMP_SimpleInit(OPAMP_MODE_VOLTAGE_FOLLOWER);
    OPAMP_Enable();
    
    while(1) {
        // OPAMP ทำงานเป็น buffer
    }
}
```

**การใช้งาน:**
- Buffer สัญญาณจาก sensor
- ป้องกัน loading effect
- Impedance matching

---

### 2. Non-Inverting Amplifier

**วัตถุประสงค์:** ขยายสัญญาณโดยไม่กลับเฟส

**วงจร:**
```
Input ──→ CHP0 (+)
            │
            ▼
         OPAMP
            │
            ▼
         Output ──┬──→ Load
                  │
                 R2
                  │
                  ├──→ CHN0 (-)
                  │
                 R1
                  │
                 GND
```

**สูตร:**
```
Gain = 1 + (R2/R1)
Vout = Vin × Gain
```

**ตัวอย่างการคำนวณ:**

| Gain | R1 (kΩ) | R2 (kΩ) | สูตร |
|------|---------|---------|------|
| 2    | 10      | 10      | 1 + (10/10) = 2 |
| 5    | 10      | 40      | 1 + (40/10) = 5 |
| 10   | 10      | 90      | 1 + (90/10) = 10 |

**โค้ด:**
```c
#include "SimpleHAL/SimpleOPAMP.h"

// ต้องการ Gain = 5
#define R1  10000   // 10kΩ
#define R2  40000   // 40kΩ

int main(void) {
    // คำนวณ R2 สำหรับ gain ที่ต้องการ
    uint32_t r2 = OPAMP_CalculateR2NonInv(R1, 5.0f);
    printf("R2 required: %lu Ω\n", r2);
    
    // เริ่มต้น OPAMP
    OPAMP_ConfigNonInverting(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();
    
    while(1) {
        // OPAMP ขยายสัญญาณ 5 เท่า
    }
}
```

**การใช้งาน:**
- ขยายสัญญาณจาก sensor
- เพิ่มความละเอียดของ ADC
- Signal conditioning

---

### 3. Inverting Amplifier

**วัตถุประสงค์:** ขยายสัญญาณและกลับเฟส

**วงจร:**
```
Vref ──→ CHP0 (+)

Input ──[R1]──┬──→ CHN0 (-)
              │
              │
           Output ──┬──→ Load
                    │
                   [R2]
                    │
                    └──── (feedback)
```

**สูตร:**
```
Gain = -(R2/R1)
Vout = Vref - (Vin - Vref) × |Gain|
```

**ตัวอย่างการคำนวณ:**

| Gain | R1 (kΩ) | R2 (kΩ) | สูตร |
|------|---------|---------|------|
| -2   | 10      | 20      | -(20/10) = -2 |
| -5   | 10      | 50      | -(50/10) = -5 |
| -10  | 10      | 100     | -(100/10) = -10 |

**โค้ด:**
```c
#include "SimpleHAL/SimpleOPAMP.h"

// ต้องการ Gain = -3
#define R1  10000   // 10kΩ
#define R2  30000   // 30kΩ

int main(void) {
    // คำนวณ R2 สำหรับ gain ที่ต้องการ
    uint32_t r2 = OPAMP_CalculateR2Inv(R1, 3.0f);
    printf("R2 required: %lu Ω\n", r2);
    
    // เริ่มต้น OPAMP
    OPAMP_ConfigInverting(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();
    
    while(1) {
        // OPAMP ขยายและกลับเฟส
    }
}
```

**การใช้งาน:**
- ขยายและกลับเฟสสัญญาณ
- Summing amplifier
- Current-to-voltage converter

---

### 4. Comparator Mode

**วัตถุประสงค์:** เปรียบเทียบแรงดัน 2 ค่า

**วงจร:**
```
Signal ──→ CHP0 (+)

Threshold ──→ CHN0 (-)

Output ──→ Digital (HIGH/LOW)
```

**การทำงาน:**
```
ถ้า V+ > V-  →  Output = HIGH (≈ Vcc)
ถ้า V+ < V-  →  Output = LOW (≈ GND)
```

**โค้ด:**
```c
#include "SimpleHAL/SimpleOPAMP.h"
#include "SimpleHAL/SimpleGPIO.h"

int main(void) {
    // เริ่มต้น OPAMP เป็น comparator
    OPAMP_ConfigComparator(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();
    
    // LED indicator
    pinMode(PC0, PIN_MODE_OUTPUT);
    
    while(1) {
        // อ่านค่า comparator output
        uint16_t output = ADC_Read(ADC_CH_A2);
        
        // ควบคุม LED
        if(output > 512) {  // Signal > Threshold
            digitalWrite(PC0, HIGH);
        } else {
            digitalWrite(PC0, LOW);
        }
        
        Delay_Ms(100);
    }
}
```

**การใช้งาน:**
- Zero-crossing detector
- Threshold detection
- Over-voltage/Under-voltage protection
- Window comparator

---

## การใช้งานขั้นสูง

### Signal Conditioning สำหรับ Sensor

**ปัญหา:** Sensor หลายตัวให้สัญญาณเล็กมาก (mV) ทำให้ ADC อ่านได้ไม่ละเอียด

**วิธีแก้:** ใช้ OPAMP ขยายสัญญาณก่อนส่งเข้า ADC

**ตัวอย่าง: LM35 Temperature Sensor**

LM35 ให้สัญญาณ 10mV/°C
- ที่ 25°C → 250mV
- ADC 10-bit (3.3V) → resolution = 3.2mV
- Temperature resolution = 0.32°C (ไม่ละเอียดพอ!)

**วิธีแก้:**
```
LM35 → OPAMP (Gain=10) → ADC
```
- Output = 100mV/°C
- Temperature resolution = 0.032°C (ละเอียดขึ้น 10 เท่า!)

**โค้ด:**
```c
#define SENSOR_MV_PER_UNIT  10.0f   // 10mV/°C
#define AMPLIFIER_GAIN      10.0f   // ขยาย 10 เท่า
#define R1  10000   // 10kΩ
#define R2  90000   // 90kΩ → Gain = 10

int main(void) {
    // เริ่มต้น OPAMP
    OPAMP_ConfigNonInverting(OPAMP_CHP0, OPAMP_CHN0);
    OPAMP_Enable();
    
    ADC_SimpleInit();
    
    while(1) {
        // อ่านค่า ADC (หลังขยาย)
        uint16_t adc = ADC_Read(ADC_CH_A0);
        float amplified_v = ADC_ToVoltage(adc, 3.3);
        
        // คำนวณกลับเป็นค่าจาก sensor
        float sensor_mv = (amplified_v / AMPLIFIER_GAIN) * 1000.0f;
        float temperature = sensor_mv / SENSOR_MV_PER_UNIT;
        
        printf("Temperature: %.2f°C\n", temperature);
        Delay_Ms(1000);
    }
}
```

---

### Offset Compensation

**ปัญหา:** สัญญาณมี DC offset ที่ไม่ต้องการ

**วิธีแก้:** ใช้ voltage divider สร้าง reference voltage

**วงจร:**
```
         Vcc
          │
         R3
          │
Vref ─────┴──→ CHP0 (+)

Input ──[R1]──┬──→ CHN0 (-)
              │
           Output
              │
             [R2]
              │
             (feedback)
```

**โค้ด:**
```c
// สร้าง Vref = Vcc/2 = 1.65V
// ใช้ voltage divider R3=R4=10kΩ

OPAMP_ConfigInverting(OPAMP_CHP0, OPAMP_CHN0);
OPAMP_Enable();
```

---

### Multi-Stage Amplification

**เมื่อไร:** ต้องการ gain สูงมาก (>100x)

**วิธี:** ใช้ OPAMP หลายขั้น

**ตัวอย่าง:**
```
Stage 1: Gain = 10
Stage 2: Gain = 10
Total Gain = 10 × 10 = 100
```

**ข้อควรระวัง:**
- Bandwidth ลดลงตาม gain
- Noise สะสมทุกขั้น
- ต้องมี decoupling capacitor

---

## เทคนิคขั้นสูง

### 1. Auto-Ranging Amplifier

**วัตถุประสงค์:** สลับ gain อัตโนมัติตามขนาดสัญญาณ

**หลักการ:**
```
ถ้า Output < 80% full scale  →  เพิ่ม Gain
ถ้า Output > 95% full scale  →  ลด Gain
```

**ประโยชน์:**
- ป้องกัน saturation
- ใช้ช่วงของ ADC ได้เต็มที่
- วัดได้หลายช่วง

**โค้ด:** ดูใน `06_Advanced_Techniques.c`

---

### 2. Noise Reduction

**เทคนิค:**

1. **Hardware:**
   - ใช้ low-pass filter (RC filter)
   - Shielded cable สำหรับสัญญาณ analog
   - Separate analog/digital ground

2. **Software:**
   - Averaging หลายครั้ง
   - Median filter
   - Moving average

**ตัวอย่าง:**
```c
// อ่านค่า ADC แบบ average
uint16_t adc = ADC_ReadAverage(ADC_CH_A0, 10);
```

---

### 3. Bandwidth Considerations

**ข้อจำกัด:**
- Gain × Bandwidth = constant (Gain-Bandwidth Product)
- Gain สูง → Bandwidth แคบ

**ตัวอย่าง:**
```
GBW = 1 MHz
Gain = 10  →  Bandwidth = 100 kHz
Gain = 100 →  Bandwidth = 10 kHz
```

**วิธีแก้:**
- ใช้ gain ที่พอเหมาะ
- Multi-stage amplifier
- เลือก OPAMP ที่มี GBW สูง

---

## ตัวอย่างการใช้งาน

### 📁 ตัวอย่างที่มีให้

| ไฟล์ | คำอธิบาย | ความยาก |
|------|----------|---------|
| `01_Basic_VoltageFollower.c` | Voltage follower พื้นฐาน | ⭐ |
| `02_NonInverting_Amplifier.c` | Non-inverting amplifier | ⭐⭐ |
| `03_Inverting_Amplifier.c` | Inverting amplifier | ⭐⭐ |
| `04_Comparator_Mode.c` | Comparator กับ LED | ⭐⭐ |
| `05_Signal_Conditioning.c` | LM35 sensor interface | ⭐⭐⭐ |
| `06_Advanced_Techniques.c` | Auto-ranging amplifier | ⭐⭐⭐⭐ |

---

## การแก้ปัญหา

### ปัญหา: Output ไม่ตาม Input

**สาเหตุที่เป็นไปได้:**
1. ไม่ได้ต่อ feedback (negative input)
2. ความต้านทานผิดค่า
3. OPAMP ไม่ได้เปิดใช้งาน

**วิธีแก้:**
```c
// ตรวจสอบว่า OPAMP เปิดอยู่
if(OPAMP_IsEnabled()) {
    printf("OPAMP is running\n");
} else {
    printf("OPAMP is disabled!\n");
    OPAMP_Enable();
}
```

---

### ปัญหา: Output Saturated (ติด Vcc หรือ GND)

**สาเหตุ:**
1. Input signal ใหญ่เกินไป
2. Gain สูงเกินไป
3. Offset ไม่ถูกต้อง

**วิธีแก้:**
1. ลด input signal
2. ลด gain (เปลี่ยน R2)
3. ปรับ offset/reference voltage

---

### ปัญหา: Gain ไม่ตรงตามคำนวณ

**สาเหตุ:**
1. ความต้านทานไม่ตรงค่า (tolerance)
2. Loading effect
3. Frequency response

**วิธีแก้:**
1. วัดค่าความต้านทานจริง
2. ใช้ buffer สำหรับ output
3. ตรวจสอบ bandwidth

---

### ปัญหา: Noise มาก

**วิธีแก้:**
1. เพิ่ม decoupling capacitor (0.1µF)
2. ใช้ shielded cable
3. Separate analog/digital ground
4. Software averaging

```c
// Averaging เพื่อลด noise
uint16_t adc = ADC_ReadAverage(ADC_CH_A0, 10);
```

---

## 📊 ตารางสรุป

### เปรียบเทียบโหมดต่างๆ

| โหมด | Gain | Phase | Input Z | การใช้งาน |
|------|------|-------|---------|-----------|
| Voltage Follower | 1 | 0° | สูงมาก | Buffer, Impedance matching |
| Non-Inverting | >1 | 0° | สูงมาก | ขยายสัญญาณ |
| Inverting | <0 | 180° | = R1 | ขยาย+กลับเฟส |
| Comparator | ∞ | - | สูงมาก | Threshold detection |

---

## 🔗 API Reference

### Basic Functions
- `OPAMP_SimpleInit(mode)` - เริ่มต้นแบบง่าย
- `OPAMP_Enable()` - เปิดใช้งาน
- `OPAMP_Disable()` - ปิดการใช้งาน
- `OPAMP_SetMode(mode)` - เปลี่ยนโหมด

### Advanced Functions
- `OPAMP_ConfigVoltageFollower(pos)` - ตั้งค่า voltage follower
- `OPAMP_ConfigNonInverting(pos, neg)` - ตั้งค่า non-inverting
- `OPAMP_ConfigInverting(pos, neg)` - ตั้งค่า inverting
- `OPAMP_ConfigComparator(pos, neg)` - ตั้งค่า comparator

### Utility Functions
- `OPAMP_CalculateGainNonInv(r1, r2)` - คำนวณ gain (non-inv)
- `OPAMP_CalculateGainInv(r1, r2)` - คำนวณ gain (inv)
- `OPAMP_CalculateR2NonInv(r1, gain)` - คำนวณ R2 (non-inv)
- `OPAMP_CalculateR2Inv(r1, gain)` - คำนวณ R2 (inv)

---

## 📚 เอกสารอ้างอิง

- CH32V003 Datasheet
- CH32V003 Reference Manual
- Operational Amplifier Theory
- Analog Circuit Design

---

## 📝 License

MIT License

---

## 👨‍💻 Version

- **Version:** 1.0.0
- **Date:** 2025-12-21
- **Author:** SimpleHAL Team
