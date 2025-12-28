# SimpleWWDG - Window Watchdog Library

> 📚 เอกสารฉบับสมบูรณ์สำหรับ SimpleWWDG Library

## 📖 สารบัญ

- [แนวคิดพื้นฐาน](#แนวคิดพื้นฐาน)
- [ความแตกต่างจาก IWDG](#ความแตกต่างจาก-iwdg)
- [การติดตั้งและใช้งาน](#การติดตั้งและใช้งาน)
- [API Reference](#api-reference)
- [ตัวอย่างการใช้งาน](#ตัวอย่างการใช้งาน)
- [การคำนวณ Timeout](#การคำนวณ-timeout)
- [Early Wakeup Interrupt](#early-wakeup-interrupt)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)
- [เทคนิคขั้นสูง](#เทคนิคขั้นสูง)

---

## แนวคิดพื้นฐาน

### WWDG คืออะไร?

**WWDG (Window Watchdog)** คือ watchdog timer ที่มี "window" หรือช่วงเวลาที่กำหนด การ refresh watchdog ต้องทำในช่วงเวลาที่ถูกต้องเท่านั้น

### การทำงาน

```
┌─────────────────────────────────────────────────┐
│  WWDG Counter (นับถอยหลัง)                      │
│                                                 │
│  0x7F ────> 0x50 ────> 0x40 ────> 0x3F         │
│   (127)      (80)       (64)       (63)        │
│                │         │          │          │
│                │         │          └─> RESET! │
│                │         └─> EWI (Interrupt)   │
│                └─> Window (ต้อง refresh ที่นี่)│
│                                                 │
│  ❌ Refresh เร็วเกินไป (counter > window)      │
│  ✅ Refresh ในช่วง window                      │
│  ❌ Refresh ช้าเกินไป (counter < 0x40)         │
└─────────────────────────────────────────────────┘
```

### ทำไมต้องใช้ WWDG?

1. **ตรวจสอบ Timing เข้มงวด** - ต้อง refresh ในช่วงเวลาที่กำหนด
2. **Detect Timing Violations** - ตรวจจับการทำงานที่เร็วหรือช้าเกินไป
3. **Early Warning** - มี interrupt แจ้งเตือนก่อน reset
4. **Critical Applications** - เหมาะสำหรับระบบที่ timing สำคัญมาก

---

## ความแตกต่างจาก IWDG

| คุณสมบัติ | IWDG | WWDG |
|----------|------|------|
| **Clock Source** | LSI (40kHz, อิสระ) | PCLK1 (24MHz) |
| **Timeout Range** | 0.1ms - 32.8s | 0.17ms - 87.4ms |
| **Window** | ❌ ไม่มี | ✅ มี |
| **Interrupt** | ❌ ไม่มี | ✅ มี (EWI) |
| **Refresh Timing** | ก่อนหมดเวลา | ในช่วง window เท่านั้น |
| **การใช้งาน** | ป้องกันค้างทั่วไป | ตรวจสอบ timing เข้มงวด |
| **ความซับซ้อน** | ⭐ ง่าย | ⭐⭐⭐ ซับซ้อน |

### เมื่อไหร่ควรใช้ WWDG?

✅ **ใช้ WWDG เมื่อ:**
- ต้องการตรวจสอบ timing ที่เข้มงวด
- Task ต้องทำงานในช่วงเวลาที่กำหนด
- ต้องการ early warning interrupt
- ระบบ safety-critical

✅ **ใช้ IWDG เมื่อ:**
- ต้องการป้องกันระบบค้างทั่วไป
- Timing ไม่เข้มงวดมาก
- ต้องการ timeout ยาว (> 100ms)
- ใช้งานง่ายกว่า

---

## การติดตั้งและใช้งาน

### 1. Include Header

```c
#include "SimpleHAL/SimpleWWDG.h"
```

### 2. การใช้งานพื้นฐาน

```c
int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    
    // เริ่มต้น WWDG
    // Counter = 0x7F (127), Window = 0x50 (80)
    WWDG_SimpleInit(0x7F, 0x50);
    
    while(1)
    {
        // ต้อง refresh เมื่อ: 0x50 > counter > 0x40
        WWDG_Refresh(0x7F);
        
        // Delay ต้องอยู่ในช่วงที่ถูกต้อง
        Delay_Ms(40);  // Safe for this config
    }
}
```

### 3. การใช้งานกับ Interrupt

```c
void WWDG_EarlyWarning(void)
{
    printf("Warning: About to reset!\n");
    WWDG_Refresh(0x7F);  // Refresh to prevent reset
}

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    
    // ตั้ง callback
    WWDG_SetCallback(WWDG_EarlyWarning);
    
    // เริ่มต้น WWDG พร้อม interrupt
    WWDG_InitWithInterrupt(0x7F, 0x50, WWDG_PRESCALER_8);
    
    while(1)
    {
        // Your code
        Delay_Ms(100);
    }
}
```

**Note:** ต้องเพิ่ม interrupt handler ใน `ch32v00x_it.c` (ดูรายละเอียดด้านล่าง)

---

## API Reference

### Basic API

#### `WWDG_SimpleInit(counter, window)`

เริ่มต้น WWDG ด้วยค่า counter และ window

**Parameters:**
- `counter` - ค่า counter เริ่มต้น (0x40 - 0x7F)
- `window` - ค่า window (0x40 - 0x7F)

**Example:**
```c
WWDG_SimpleInit(0x7F, 0x50);
// Counter = 127, Window = 80
// Valid refresh: 80 > counter > 64
```

**Note:** ใช้ prescaler = 8 เป็นค่าเริ่มต้น

---

#### `WWDG_Refresh(counter)`

Refresh WWDG counter

**Parameters:**
- `counter` - ค่า counter ใหม่ (0x40 - 0x7F)

**Example:**
```c
WWDG_Refresh(0x7F);  // Reset counter to 127
```

**Warning:** ต้อง refresh เมื่อ counter อยู่ในช่วง window เท่านั้น!

---

### Advanced API

#### `WWDG_Init(counter, window, prescaler)`

เริ่มต้น WWDG ด้วยค่า prescaler ที่กำหนดเอง

**Parameters:**
- `counter` - ค่า counter เริ่มต้น (0x40 - 0x7F)
- `window` - ค่า window (0x40 - 0x7F)
- `prescaler` - ค่า prescaler (ใช้ constants: `WWDG_PRESCALER_1/2/4/8`)

**Example:**
```c
WWDG_Init(0x7F, 0x50, WWDG_PRESCALER_4);
```

---

#### `WWDG_InitWithInterrupt(counter, window, prescaler)`

เริ่มต้น WWDG พร้อม Early Wakeup Interrupt

**Parameters:**
- `counter` - ค่า counter เริ่มต้น (0x40 - 0x7F)
- `window` - ค่า window (0x40 - 0x7F)
- `prescaler` - ค่า prescaler

**Example:**
```c
WWDG_SetCallback(MyCallback);
WWDG_InitWithInterrupt(0x7F, 0x50, WWDG_PRESCALER_8);
```

**Note:** Interrupt จะถูกเรียกเมื่อ counter = 0x40

---

#### `WWDG_SetCallback(callback)`

ตั้งค่า callback function สำหรับ Early Wakeup Interrupt

**Parameters:**
- `callback` - pointer to callback function

**Example:**
```c
void MyCallback(void)
{
    printf("Early wakeup!\n");
}

WWDG_SetCallback(MyCallback);
```

---

### Utility Functions

#### `WWDG_CalcTimeout(prescaler, counter)`

คำนวณ timeout ในหน่วยมิลลิวินาที

**Returns:** Timeout (ms)

**Example:**
```c
uint32_t timeout = WWDG_CalcTimeout(8, 0x7F);
printf("Timeout: %lu ms\n", timeout);  // ~87ms
```

---

#### `WWDG_GetInterruptFlag()`

ตรวจสอบ Early Wakeup Interrupt flag

**Returns:**
- `1` - Interrupt flag set
- `0` - Interrupt flag not set

---

#### `WWDG_ClearInterruptFlag()`

ล้าง Early Wakeup Interrupt flag

---

#### `WWDG_Disable()`

ปิด WWDG (reset peripheral)

**Example:**
```c
WWDG_Disable();
```

---

## ตัวอย่างการใช้งาน

### ระดับพื้นฐาน: Window Watchdog

```c
int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    
    pinMode(PC0, PIN_MODE_OUTPUT);
    
    // Counter = 0x7F, Window = 0x50, Prescaler = 8
    WWDG_SimpleInit(0x7F, 0x50);
    
    // Timeout = ~87ms, Window = ~23ms
    // Valid refresh: 23ms - 87ms
    
    while(1)
    {
        digitalToggle(PC0);
        WWDG_Refresh(0x7F);
        Delay_Ms(40);  // ✅ อยู่ในช่วง 23-87ms
    }
}
```

---

### ระดับกลาง: Early Wakeup Interrupt

```c
volatile uint32_t interrupt_count = 0;

void WWDG_EarlyWakeup(void)
{
    interrupt_count++;
    printf("Early wakeup #%lu\n", interrupt_count);
    
    // Refresh to prevent reset
    WWDG_Refresh(0x7F);
}

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_Printf_Init(115200);
    
    WWDG_SetCallback(WWDG_EarlyWakeup);
    WWDG_InitWithInterrupt(0x7F, 0x50, WWDG_PRESCALER_8);
    
    while(1)
    {
        // Main loop
        Delay_Ms(100);
    }
}
```

**Note:** ต้องเพิ่ม interrupt handler (ดูด้านล่าง)

---

### ระดับสูง: Critical Timing Protection

```c
#define TASK_INTERVAL_MS    50
#define TOLERANCE_MS        10

void CriticalTask(void)
{
    static uint32_t last_time = 0;
    uint32_t elapsed = millis() - last_time;
    
    if(elapsed < (TASK_INTERVAL_MS - TOLERANCE_MS) ||
       elapsed > (TASK_INTERVAL_MS + TOLERANCE_MS))
    {
        printf("Timing violation: %lu ms\n", elapsed);
        // ไม่ refresh - ให้ WWDG reset
        return;
    }
    
    // Timing OK
    WWDG_Refresh(0x7F);
    last_time = millis();
}

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    
    WWDG_SimpleInit(0x7F, 0x60);
    
    while(1)
    {
        CriticalTask();
        Delay_Ms(TASK_INTERVAL_MS);
    }
}
```

---

## การคำนวณ Timeout

### ตาราง Prescaler และ Timeout (PCLK1 = 24MHz)

| Prescaler | Counter Range | Min Timeout | Max Timeout |
|-----------|---------------|-------------|-------------|
| 1         | 0x40 - 0x7F   | 170 µs      | 10.9 ms     |
| 2         | 0x40 - 0x7F   | 341 µs      | 21.8 ms     |
| 4         | 0x40 - 0x7F   | 683 µs      | 43.7 ms     |
| 8         | 0x40 - 0x7F   | 1.37 ms     | 87.4 ms     |

### สูตรการคำนวณ

```c
// Timeout (µs) = (4096 * prescaler * (counter - 0x3F)) / PCLK1
timeout_us = (4096 * prescaler * (counter - 63)) / 24000000

// Timeout (ms)
timeout_ms = timeout_us / 1000
```

### ตัวอย่างการคำนวณ

**Configuration: Prescaler=8, Counter=0x7F, Window=0x50**

```c
// Max timeout (counter = 0x7F = 127)
timeout_max = (4096 * 8 * (127 - 63)) / 24000000
            = (4096 * 8 * 64) / 24000000
            = 87.4 ms

// Window timeout (counter = 0x50 = 80)
timeout_window = (4096 * 8 * (80 - 63)) / 24000000
               = (4096 * 8 * 17) / 24000000
               = 23.2 ms

// Valid refresh range: 23.2ms - 87.4ms
```

---

## Early Wakeup Interrupt

### การเพิ่ม Interrupt Handler

**ใน `ch32v00x_it.c`:**

```c
#include "SimpleHAL/SimpleWWDG.h"

// เพิ่ม interrupt handler
void WWDG_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void WWDG_IRQHandler(void)
{
    WWDG_IRQHandler_Callback();
}
```

### การใช้งาน Interrupt

```c
void MyEarlyWakeup(void)
{
    // ⚠️ ทำงานให้เร็วที่สุด!
    
    // Option 1: Refresh เพื่อป้องกัน reset
    WWDG_Refresh(0x7F);
    
    // Option 2: ไม่ refresh - ให้ reset (emergency shutdown)
    // SaveCriticalData();
    // (ไม่ refresh)
}

int main(void)
{
    WWDG_SetCallback(MyEarlyWakeup);
    WWDG_InitWithInterrupt(0x7F, 0x50, WWDG_PRESCALER_8);
    // ...
}
```

### ประโยชน์ของ Early Wakeup Interrupt

1. **แจ้งเตือนก่อน Reset** - มีเวลาประมาณ 1-2ms ก่อน reset
2. **บันทึก Log** - บันทึกสถานะก่อน reset
3. **Emergency Shutdown** - ปิดระบบอย่างปลอดภัย
4. **ส่งการแจ้งเตือน** - แจ้งผ่าน USART/Network

---

## Best Practices

### ✅ DO

1. **คำนวณ timing อย่างระมัดระวัง**
   ```c
   uint32_t timeout = WWDG_CalcTimeout(8, 0x7F);
   uint32_t window = WWDG_CalcTimeout(8, 0x50);
   printf("Valid range: %lu - %lu ms\n", window, timeout);
   ```

2. **ทดสอบ timing ก่อนใช้งานจริง**
   ```c
   uint32_t start = millis();
   // Your code
   uint32_t elapsed = millis() - start;
   printf("Elapsed: %lu ms\n", elapsed);
   ```

3. **ใช้ interrupt สำหรับ early warning**
   ```c
   WWDG_SetCallback(EarlyWarning);
   WWDG_InitWithInterrupt(0x7F, 0x50, WWDG_PRESCALER_8);
   ```

4. **ใช้ร่วมกับ IWDG**
   ```c
   // WWDG สำหรับ critical task
   WWDG_SimpleInit(0x7F, 0x50);
   
   // IWDG สำหรับ overall protection
   IWDG_SimpleInit(2000);
   ```

### ❌ DON'T

1. **อย่าใช้ delay ที่นอกช่วง window**
   ```c
   // ❌ อันตราย
   WWDG_SimpleInit(0x7F, 0x50);  // Valid: 23-87ms
   while(1)
   {
       WWDG_Refresh(0x7F);
       Delay_Ms(100);  // ❌ เกิน 87ms!
   }
   ```

2. **อย่า refresh เร็วเกินไป**
   ```c
   // ❌ ผิด
   while(1)
   {
       WWDG_Refresh(0x7F);
       Delay_Ms(10);  // ❌ เร็วกว่า window (23ms)!
   }
   ```

3. **อย่าใช้ printf() ใน interrupt**
   ```c
   // ❌ ไม่ดี
   void WWDG_Callback(void)
   {
       printf("Interrupt!\n");  // ❌ ช้าเกินไป!
       WWDG_Refresh(0x7F);
   }
   ```

4. **อย่าใช้ WWDG ถ้า timing ไม่แน่นอน**
   ```c
   // ❌ ไม่เหมาะ
   while(1)
   {
       ProcessData();  // เวลาไม่แน่นอน
       WWDG_Refresh(0x7F);  // อาจผิด window!
   }
   ```

---

## Troubleshooting

### ปัญหา: ระบบ reset ทันที

**สาเหตุ:**
- Refresh เร็วเกินไป (counter > window)
- Window configuration ผิด

**แก้ไข:**
```c
// ตรวจสอบ timing
uint32_t window_time = WWDG_CalcTimeout(8, 0x50);
printf("Window: %lu ms\n", window_time);

// เพิ่ม delay ให้มากกว่า window
Delay_Ms(window_time + 10);
WWDG_Refresh(0x7F);
```

---

### ปัญหา: ระบบ reset หลัง delay

**สาเหตุ:**
- Refresh ช้าเกินไป (counter < 0x40)
- Delay มากกว่า timeout

**แก้ไข:**
```c
// ตรวจสอบ max timeout
uint32_t max_timeout = WWDG_CalcTimeout(8, 0x7F);
printf("Max timeout: %lu ms\n", max_timeout);

// Delay ต้องน้อยกว่า max timeout
Delay_Ms(max_timeout - 10);
WWDG_Refresh(0x7F);
```

---

### ปัญหา: Interrupt ไม่ทำงาน

**สาเหตุ:**
- ลืมเพิ่ม interrupt handler ใน ch32v00x_it.c
- ไม่ได้เรียก WWDG_SetCallback()

**แก้ไข:**
```c
// 1. เพิ่มใน ch32v00x_it.c
void WWDG_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void WWDG_IRQHandler(void)
{
    WWDG_IRQHandler_Callback();
}

// 2. ตั้ง callback
WWDG_SetCallback(MyCallback);
WWDG_InitWithInterrupt(0x7F, 0x50, WWDG_PRESCALER_8);
```

---

## เทคนิคขั้นสูง

### 1. Adaptive Window

```c
void AdaptiveWWDG(uint32_t task_time_ms)
{
    // ปรับ window ตาม task execution time
    uint8_t window = 0x7F - (task_time_ms / 2);
    
    if(window < 0x40) window = 0x40;
    
    WWDG_Init(0x7F, window, WWDG_PRESCALER_8);
}
```

---

### 2. Timing Violation Logger

```c
typedef struct {
    uint32_t timestamp;
    uint32_t elapsed;
    uint8_t  violation_type;  // 0=too early, 1=too late
} TimingViolation_t;

TimingViolation_t violations[10];
uint8_t violation_index = 0;

void LogTimingViolation(uint32_t elapsed, uint8_t type)
{
    violations[violation_index].timestamp = millis();
    violations[violation_index].elapsed = elapsed;
    violations[violation_index].violation_type = type;
    
    violation_index = (violation_index + 1) % 10;
}
```

---

### 3. WWDG + IWDG Combined

```c
int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    
    // WWDG สำหรับ critical task (timing เข้มงวด)
    WWDG_SimpleInit(0x7F, 0x50);
    
    // IWDG สำหรับ overall protection
    IWDG_SimpleInit(2000);
    
    while(1)
    {
        // Critical task - ต้องทำใน timing ที่กำหนด
        CriticalTask();
        WWDG_Refresh(0x7F);
        
        // Normal tasks
        NormalTask1();
        NormalTask2();
        
        // Feed IWDG
        IWDG_Feed();
    }
}
```

---

### 4. Emergency Shutdown

```c
void WWDG_EmergencyShutdown(void)
{
    // บันทึกข้อมูลสำคัญ
    SaveCriticalData();
    
    // ปิด peripherals
    TurnOffMotors();
    CloseValves();
    
    // ส่งการแจ้งเตือน
    SendAlert("Emergency shutdown!");
    
    // ไม่ refresh WWDG - ให้ระบบ reset
}

int main(void)
{
    WWDG_SetCallback(WWDG_EmergencyShutdown);
    WWDG_InitWithInterrupt(0x7F, 0x50, WWDG_PRESCALER_8);
    // ...
}
```

---

## สรุป

### WWDG vs IWDG - เลือกใช้อย่างไร?

| สถานการณ์ | แนะนำ |
|----------|-------|
| ป้องกันระบบค้างทั่วไป | IWDG |
| ตรวจสอบ timing เข้มงวด | WWDG |
| Timeout ยาว (> 100ms) | IWDG |
| Timeout สั้น (< 100ms) | WWDG |
| ต้องการ early warning | WWDG |
| ใช้งานง่าย | IWDG |
| Critical timing | WWDG |
| ระบบ safety-critical | WWDG + IWDG |

### ข้อควรระวัง

⚠️ **WWDG ซับซ้อนกว่า IWDG มาก!**
- ต้องคำนวณ timing อย่างระมัดระวัง
- ต้องทดสอบให้ดีก่อนใช้งานจริง
- ไม่เหมาะสำหรับ timing ที่ไม่แน่นอน
- ควรใช้ร่วมกับ IWDG สำหรับความปลอดภัย

---

## ไฟล์ตัวอย่าง

- `01_Basic_WWDG.c` - การใช้งานพื้นฐาน
- `02_WWDG_Interrupt.c` - Early Wakeup Interrupt
- `03_Critical_Timing.c` - Critical timing protection

---

**Version:** 1.0.0  
**Last Updated:** 2025-12-21  
**Author:** SimpleHAL Team
