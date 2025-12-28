# SimpleIWDG - Independent Watchdog Library

> 📚 เอกสารฉบับสมบูรณ์สำหรับ SimpleIWDG Library

## 📖 สารบัญ

- [แนวคิดพื้นฐาน](#แนวคิดพื้นฐาน)
- [การติดตั้งและใช้งาน](#การติดตั้งและใช้งาน)
- [API Reference](#api-reference)
- [ตัวอย่างการใช้งาน](#ตัวอย่างการใช้งาน)
- [การคำนวณ Timeout](#การคำนวณ-timeout)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)
- [เทคนิคขั้นสูง](#เทคนิคขั้นสูง)

---

## แนวคิดพื้นฐาน

### IWDG คืออะไร?

**IWDG (Independent Watchdog)** คือ watchdog timer ที่ทำงานอิสระจาก system clock โดยใช้ LSI (Low Speed Internal) oscillator ที่มีความถี่ประมาณ 40kHz

### การทำงาน

```
┌─────────────────────────────────────────────────┐
│  IWDG Counter (นับถอยหลัง)                      │
│                                                 │
│  Start: 0xFFF (4095) ──────> Countdown ──> 0   │
│                                            │    │
│                                            ↓    │
│                                        RESET!   │
│                                                 │
│  ป้องกัน: เรียก IWDG_Feed() ก่อนถึง 0          │
└─────────────────────────────────────────────────┘
```

### ทำไมต้องใช้ Watchdog?

1. **ป้องกันระบบค้าง** - ถ้าโปรแกรมค้างหรือเข้า infinite loop
2. **Auto Recovery** - ระบบจะ reset และกลับมาทำงานปกติ
3. **Reliability** - เพิ่มความน่าเชื่อถือของระบบ
4. **Safety** - สำคัญในระบบ critical applications

---

## การติดตั้งและใช้งาน

### 1. Include Header

```c
#include "SimpleHAL/SimpleIWDG.h"
```

### 2. การใช้งานพื้นฐาน

```c
int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    
    // เริ่มต้น IWDG ด้วย timeout 1000ms (1 วินาที)
    IWDG_SimpleInit(1000);
    
    while(1)
    {
        // ต้อง feed watchdog ก่อนหมดเวลา
        IWDG_Feed();
        
        // Your code here
        Delay_Ms(500);
    }
}
```

### 3. ตรวจสอบสาเหตุการ Reset

```c
if(IWDG_WasResetCause())
{
    printf("System recovered from watchdog reset!\n");
    IWDG_ClearResetFlag();
}
```

---

## API Reference

### Basic API

#### `IWDG_SimpleInit(timeout_ms)`

เริ่มต้น IWDG ด้วย timeout ที่กำหนด (มิลลิวินาที)

**Parameters:**
- `timeout_ms` - เวลา timeout (1 - 32768 ms)

**Example:**
```c
IWDG_SimpleInit(1000);  // 1 second timeout
IWDG_SimpleInit(5000);  // 5 seconds timeout
```

**Note:** ฟังก์ชันนี้จะเลือก prescaler ที่เหมาะสมโดยอัตโนมัติ

---

#### `IWDG_Feed()`

Feed watchdog (reload counter)

**Example:**
```c
IWDG_Feed();  // Reset watchdog counter
```

**Note:** ต้องเรียกก่อนหมดเวลา มิฉะนั้นระบบจะ reset

---

### Advanced API

#### `IWDG_Init(prescaler, reload)`

เริ่มต้น IWDG ด้วยค่า prescaler และ reload ที่กำหนดเอง

**Parameters:**
- `prescaler` - ค่า prescaler (ใช้ constants: `IWDG_PRESCALER_4` ถึง `IWDG_PRESCALER_256`)
- `reload` - ค่า reload (0x0000 - 0x0FFF)

**Example:**
```c
// 500ms timeout with prescaler 32
uint16_t reload = IWDG_CALC_RELOAD(32, 500);
IWDG_Init(IWDG_PRESCALER_32, reload);
```

---

#### `IWDG_IsBusy()`

ตรวจสอบว่า IWDG กำลัง update registers อยู่หรือไม่

**Returns:**
- `1` - IWDG busy
- `0` - IWDG ready

**Example:**
```c
while(IWDG_IsBusy());  // Wait for IWDG ready
```

---

#### `IWDG_GetTimeout(prescaler, reload)`

คำนวณค่า timeout จาก prescaler และ reload

**Returns:** Timeout ในหน่วยมิลลิวินาที

**Example:**
```c
uint32_t timeout = IWDG_GetTimeout(32, 625);  // Returns 500ms
```

---

### Utility Functions

#### `IWDG_WasResetCause()`

ตรวจสอบว่า reset ล่าสุดเกิดจาก IWDG หรือไม่

**Returns:**
- `1` - Reset caused by IWDG
- `0` - Reset not caused by IWDG

**Example:**
```c
if(IWDG_WasResetCause())
{
    printf("Watchdog reset detected!\n");
}
```

---

#### `IWDG_ClearResetFlag()`

ล้าง reset flag

**Example:**
```c
IWDG_ClearResetFlag();
```

---

## ตัวอย่างการใช้งาน

### ระดับพื้นฐาน: LED Blink

```c
#include "SimpleHAL/SimpleGPIO.h"
#include "SimpleHAL/SimpleIWDG.h"

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    
    pinMode(PC0, PIN_MODE_OUTPUT);
    
    // เริ่มต้น IWDG - timeout 1 วินาที
    IWDG_SimpleInit(1000);
    
    while(1)
    {
        digitalToggle(PC0);
        IWDG_Feed();        // Feed watchdog
        Delay_Ms(500);      // ต้องน้อยกว่า timeout
    }
}
```

**ผลลัพธ์:** LED กระพริบทุก 500ms, watchdog ถูก feed ทุก 500ms

---

### ระดับกลาง: System Recovery

```c
void PrintResetCause(void)
{
    if(IWDG_WasResetCause())
    {
        printf("[!] IWDG Reset - System recovered!\n");
        IWDG_ClearResetFlag();
    }
    else
    {
        printf("[*] Normal startup\n");
    }
}

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    USART_Printf_Init(115200);
    
    PrintResetCause();
    
    IWDG_SimpleInit(2000);  // 2 second timeout
    
    while(1)
    {
        // Normal operation
        IWDG_Feed();
        Delay_Ms(1000);
    }
}
```

**ผลลัพธ์:** แสดงสาเหตุการ reset และกู้คืนระบบอัตโนมัติ

---

### ระดับสูง: Multi-Task Monitoring

```c
#define TASK_FLAG_1  (1 << 0)
#define TASK_FLAG_2  (1 << 1)
#define TASK_ALL     (TASK_FLAG_1 | TASK_FLAG_2)

volatile uint8_t task_flags = 0;

void Task1(void)
{
    // Do task 1 work
    task_flags |= TASK_FLAG_1;
}

void Task2(void)
{
    // Do task 2 work
    task_flags |= TASK_FLAG_2;
}

int main(void)
{
    SystemCoreClockUpdate();
    Timer_Init();
    
    IWDG_SimpleInit(3000);  // 3 second timeout
    
    while(1)
    {
        Task1();
        Task2();
        
        // Feed watchdog only if all tasks completed
        if(task_flags == TASK_ALL)
        {
            IWDG_Feed();
            task_flags = 0;
        }
        
        Delay_Ms(100);
    }
}
```

**ผลลัพธ์:** Watchdog จะ feed เฉพาะเมื่อทุก task ทำงานสำเร็จ

---

## การคำนวณ Timeout

### ตาราง Prescaler และ Timeout

| Prescaler | ค่าจริง | Timeout ต่ำสุด | Timeout สูงสุด (reload=0xFFF) |
|-----------|---------|----------------|-------------------------------|
| 4         | 4       | 0.1 ms         | 512 ms                        |
| 8         | 8       | 0.2 ms         | 1024 ms (1.0 s)               |
| 16        | 16      | 0.4 ms         | 2048 ms (2.0 s)               |
| 32        | 32      | 0.8 ms         | 4096 ms (4.1 s)               |
| 64        | 64      | 1.6 ms         | 8192 ms (8.2 s)               |
| 128       | 128     | 3.2 ms         | 16384 ms (16.4 s)             |
| 256       | 256     | 6.4 ms         | 32768 ms (32.8 s)             |

**Note:** ค่าจริงอาจแตกต่างกัน ±25% เนื่องจาก LSI tolerance

### สูตรการคำนวณ

```c
// คำนวณ timeout (ms) จาก prescaler และ reload
timeout_ms = (prescaler * reload * 1000) / 40000

// คำนวณ reload จาก prescaler และ timeout ที่ต้องการ
reload = (timeout_ms * 40000) / (prescaler * 1000)
```

### ตัวอย่างการคำนวณ

**ต้องการ timeout 500ms:**

```c
// วิธีที่ 1: ใช้ macro
uint16_t reload = IWDG_CALC_RELOAD(32, 500);
// reload = (500 * 40000) / (32 * 1000) = 625

IWDG_Init(IWDG_PRESCALER_32, reload);

// วิธีที่ 2: ใช้ SimpleInit (แนะนำ)
IWDG_SimpleInit(500);  // เลือก prescaler อัตโนมัติ
```

---

## Best Practices

### ✅ DO

1. **Feed watchdog เป็นประจำ**
   ```c
   while(1)
   {
       IWDG_Feed();
       // Your code
   }
   ```

2. **ตรวจสอบ reset cause**
   ```c
   if(IWDG_WasResetCause())
   {
       // Handle recovery
       IWDG_ClearResetFlag();
   }
   ```

3. **ใช้ timeout ที่เหมาะสม**
   - ไม่สั้นเกินไป (ระบบ reset บ่อย)
   - ไม่ยาวเกินไป (ตรวจจับปัญหาช้า)
   - แนะนำ: 1-5 วินาที

4. **ใช้ task flags สำหรับ multi-task**
   ```c
   if(all_tasks_completed)
   {
       IWDG_Feed();
   }
   ```

### ❌ DON'T

1. **อย่า feed watchdog ใน interrupt**
   ```c
   // ❌ ไม่ดี
   void TIM1_IRQHandler(void)
   {
       IWDG_Feed();  // อย่าทำ!
   }
   ```

2. **อย่าใช้ timeout สั้นเกินไป**
   ```c
   // ❌ อันตราย
   IWDG_SimpleInit(50);  // 50ms - สั้นเกินไป!
   ```

3. **อย่าลืม feed watchdog**
   ```c
   // ❌ ระบบจะ reset
   while(1)
   {
       // IWDG_Feed();  // ลืม feed!
       Delay_Ms(500);
   }
   ```

4. **อย่า disable watchdog**
   - เมื่อเปิด IWDG แล้วจะปิดไม่ได้จนกว่าจะ reset

---

## Troubleshooting

### ปัญหา: ระบบ reset บ่อยเกินไป

**สาเหตุ:**
- Timeout สั้นเกินไป
- โค้ดทำงานช้ากว่า timeout
- ลืม feed watchdog

**แก้ไข:**
```c
// เพิ่ม timeout
IWDG_SimpleInit(2000);  // เพิ่มเป็น 2 วินาที

// ตรวจสอบว่า feed ทุกครั้ง
while(1)
{
    IWDG_Feed();  // ✅ ต้องมี
    // Your code
}
```

---

### ปัญหา: ระบบไม่ reset แม้โค้ดค้าง

**สาเหตุ:**
- Timeout ยาวเกินไป
- Feed watchdog ใน interrupt

**แก้ไข:**
```c
// ลด timeout
IWDG_SimpleInit(1000);  // 1 วินาที

// อย่า feed ใน interrupt
void TIM1_IRQHandler(void)
{
    // ❌ IWDG_Feed();  // ลบออก
}
```

---

### ปัญหา: ไม่แน่ใจว่า IWDG ทำงานหรือไม่

**ทดสอบ:**
```c
IWDG_SimpleInit(1000);

while(1)
{
    // Comment บรรทัดนี้เพื่อทดสอบ
    // IWDG_Feed();
    
    Delay_Ms(500);
}

// ถ้า IWDG ทำงาน: ระบบจะ reset ทุก 1 วินาที
```

---

## เทคนิคขั้นสูง

### 1. นับจำนวนครั้งที่ Reset

```c
#include "SimpleHAL/SimpleFlash.h"

#define RESET_COUNT_ADDR  0x1FFFF000

void IncrementResetCount(void)
{
    uint32_t count;
    FLASH_Read(RESET_COUNT_ADDR, (uint8_t*)&count, 4);
    count++;
    FLASH_Write(RESET_COUNT_ADDR, (uint8_t*)&count, 4);
    
    printf("Reset count: %lu\n", count);
}

int main(void)
{
    if(IWDG_WasResetCause())
    {
        IncrementResetCount();
        IWDG_ClearResetFlag();
    }
    
    // ...
}
```

---

### 2. Safe Mode หลัง Reset บ่อย

```c
#define MAX_RESET_COUNT  5

int main(void)
{
    uint32_t reset_count = GetResetCount();
    
    if(reset_count > MAX_RESET_COUNT)
    {
        // เข้าสู่ safe mode
        EnterSafeMode();
        
        // ไม่เปิด IWDG ใน safe mode
        while(1)
        {
            // Safe mode operation
        }
    }
    else
    {
        // Normal mode
        IWDG_SimpleInit(2000);
        // ...
    }
}
```

---

### 3. Task Monitoring แบบละเอียด

```c
typedef struct {
    uint32_t last_run;
    uint32_t timeout;
    uint8_t  is_alive;
} Task_t;

Task_t tasks[3];

void CheckTasks(void)
{
    uint32_t now = millis();
    uint8_t all_alive = 1;
    
    for(int i = 0; i < 3; i++)
    {
        if(now - tasks[i].last_run > tasks[i].timeout)
        {
            tasks[i].is_alive = 0;
            all_alive = 0;
            printf("Task %d timeout!\n", i);
        }
    }
    
    if(all_alive)
    {
        IWDG_Feed();
    }
    else
    {
        // ไม่ feed - ให้ watchdog reset
        printf("System will reset...\n");
    }
}
```

---

### 4. Watchdog + Error Logging

```c
void LogError(const char* error)
{
    // บันทึก error ใน Flash
    FLASH_WriteString(ERROR_LOG_ADDR, error);
    
    // ส่งผ่าน USART
    printf("[ERROR] %s\n", error);
}

int main(void)
{
    if(IWDG_WasResetCause())
    {
        LogError("IWDG Reset");
        
        // อ่าน error log
        char error[64];
        FLASH_ReadString(ERROR_LOG_ADDR, error, 64);
        printf("Last error: %s\n", error);
        
        IWDG_ClearResetFlag();
    }
    
    IWDG_SimpleInit(2000);
    // ...
}
```

---

## สรุป

### เมื่อไหร่ควรใช้ IWDG?

✅ **ใช้เมื่อ:**
- ระบบต้องทำงานต่อเนื่อง 24/7
- ต้องการ auto recovery
- ระบบอาจค้างได้
- ไม่มีคนดูแลระบบตลอดเวลา

❌ **ไม่ควรใช้เมื่อ:**
- ระบบทดสอบ/พัฒนา (ใช้หลังเสร็จแล้ว)
- ต้องการ debug (watchdog จะรบกวน)
- Timing ไม่แน่นอนมาก

### IWDG vs WWDG

| คุณสมบัติ | IWDG | WWDG |
|----------|------|------|
| Clock | LSI (อิสระ) | PCLK1 |
| Timeout | 0.1ms - 32s | 0.17ms - 87ms |
| Window | ไม่มี | มี |
| Interrupt | ไม่มี | มี (EWI) |
| การใช้งาน | ป้องกันค้างทั่วไป | ตรวจสอบ timing เข้มงวด |

---

## ไฟล์ตัวอย่าง

- `01_Basic_IWDG.c` - การใช้งานพื้นฐาน
- `02_System_Recovery.c` - การกู้คืนระบบ
- `03_MultiTask_Monitor.c` - ตรวจสอบหลาย tasks

---

**Version:** 1.0.0  
**Last Updated:** 2025-12-21  
**Author:** SimpleHAL Team
