# SimpleTIM - คู่มือการใช้งานฉบับสมบูรณ์

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

SimpleTIM เป็น library สำหรับควบคุม Timer peripherals บน CH32V003 ที่ใช้งานง่าย โดยตั้งค่าด้วยความถี่ (Hz) โดยตรง

### คุณสมบัติหลัก
- ✅ Frequency-based initialization
- ✅ Auto-calculation ของ prescaler/period
- ✅ Interrupt callbacks
- ✅ Dynamic frequency adjustment
- ✅ Counter access
- ✅ Advanced manual configuration

### Timer Resources

| Timer | Type | Channels | Features |
|-------|------|----------|----------|
| TIM1 | Advanced | 4 | PWM, Input Capture, Complementary outputs |
| TIM2 | General Purpose | 4 | PWM, Input Capture |

> **⚠️ หมายเหตุ:** ห้ามใช้ TIM1 ร่วมกับ SimplePWM ที่ใช้ TIM1

---

## การติดตั้งและเริ่มต้นใช้งาน

### 1. Include Header
```c
#include "SimpleHAL.h"
// หรือ
#include "SimpleTIM.h"
```

### 2. Setup พื้นฐาน
```c
#include "SimpleTIM.h"
#include <stdio.h>

void timer_callback(void) {
    printf("Tick!\n");
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // ตั้งค่า timer ที่ 1 Hz
    TIM_SimpleInit(TIM_1, 1);
    TIM_AttachInterrupt(TIM_1, timer_callback);
    TIM_Start(TIM_1);
    
    while(1);
}
```

---

## ขั้นพื้นฐาน - Basic Usage

### 1. Timer Interrupt (พื้นฐาน)

```c
#include "SimpleTIM.h"
#include <stdio.h>

volatile uint32_t tick_count = 0;

void timer1_callback(void) {
    tick_count++;
    printf("Tick: %lu\n", tick_count);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // ตั้งค่า timer ให้ทำงานที่ 1 Hz (1 ครั้งต่อวินาที)
    TIM_SimpleInit(TIM_1, 1);
    TIM_AttachInterrupt(TIM_1, timer1_callback);
    TIM_Start(TIM_1);
    
    while(1) {
        // Main loop ว่าง - ทุกอย่างทำงานใน interrupt
        Delay_Ms(100);
    }
}
```

**อธิบาย:**
- `TIM_SimpleInit()` - ตั้งค่า timer ด้วยความถี่ (Hz)
- `TIM_AttachInterrupt()` - ตั้งค่า callback function
- `TIM_Start()` - เริ่มการนับ

---

### 2. LED Blink ด้วย Timer

```c
#include "SimpleTIM.h"
#include "SimpleGPIO.h"

volatile uint8_t led_state = 0;

void led_blink_callback(void) {
    led_state = !led_state;
    digitalWrite(D0, led_state);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    pinMode(D0, PIN_MODE_OUTPUT);
    
    // ตั้งค่า timer ที่ 2 Hz (กระพริบ 1 Hz)
    TIM_SimpleInit(TIM_2, 2);
    TIM_AttachInterrupt(TIM_2, led_blink_callback);
    TIM_Start(TIM_2);
    
    while(1) {
        Delay_Ms(100);
    }
}
```

**ข้อดี:**
- ไม่ต้องใช้ delay ใน main loop
- Timing แม่นยำกว่า
- Main loop ว่างสำหรับงานอื่น

---

### 3. Timer Control (Start/Stop)

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    pinMode(D0, PIN_MODE_OUTPUT);
    
    TIM_SimpleInit(TIM_1, 2);
    TIM_AttachInterrupt(TIM_1, led_blink_callback);
    
    while(1) {
        printf("Starting timer...\n");
        TIM_Start(TIM_1);
        Delay_Ms(5000);  // Run for 5 seconds
        
        printf("Stopping timer...\n");
        TIM_Stop(TIM_1);
        digitalWrite(D0, LOW);
        Delay_Ms(2000);  // Pause for 2 seconds
    }
}
```

---

### 4. Frequency Change

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    pinMode(D0, PIN_MODE_OUTPUT);
    
    TIM_SimpleInit(TIM_1, 1);
    TIM_AttachInterrupt(TIM_1, led_blink_callback);
    TIM_Start(TIM_1);
    
    uint32_t frequencies[] = {1, 2, 4, 8};  // Hz
    
    while(1) {
        for (uint8_t i = 0; i < 4; i++) {
            printf("Frequency: %lu Hz\n", frequencies[i]);
            TIM_SetFrequency(TIM_1, frequencies[i]);
            TIM_Start(TIM_1);
            Delay_Ms(5000);
        }
    }
}
```

---

## ขั้นกลาง - Intermediate Usage

### 1. Multiple Timers

```c
volatile uint32_t fast_count = 0;
volatile uint32_t slow_count = 0;

void fast_timer_callback(void) {
    fast_count++;
}

void slow_timer_callback(void) {
    slow_count++;
    printf("Fast: %lu, Slow: %lu\n", fast_count, slow_count);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Timer 1: 10 Hz (fast)
    TIM_SimpleInit(TIM_1, 10);
    TIM_AttachInterrupt(TIM_1, fast_timer_callback);
    TIM_Start(TIM_1);
    
    // Timer 2: 1 Hz (slow)
    TIM_SimpleInit(TIM_2, 1);
    TIM_AttachInterrupt(TIM_2, slow_timer_callback);
    TIM_Start(TIM_2);
    
    while(1) {
        Delay_Ms(100);
    }
}
```

**ผลลัพธ์:**
```
Fast: 10, Slow: 1
Fast: 20, Slow: 2
Fast: 30, Slow: 3
```

---

### 2. Stopwatch (นาฬิกาจับเวลา)

```c
volatile uint32_t milliseconds = 0;
volatile uint8_t running = 0;

void stopwatch_callback(void) {
    if (running) {
        milliseconds++;
    }
}

void button_start_stop_isr(void) {
    running = !running;
    if (running) {
        milliseconds = 0;
        printf("Started\n");
    } else {
        uint32_t sec = milliseconds / 1000;
        uint32_t ms = milliseconds % 1000;
        printf("Stopped: %lu.%03lu seconds\n", sec, ms);
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Timer ที่ 1000 Hz (1ms resolution)
    TIM_SimpleInit(TIM_1, 1000);
    TIM_AttachInterrupt(TIM_1, stopwatch_callback);
    TIM_Start(TIM_1);
    
    // Button control
    pinMode(D1, PIN_MODE_INPUT_PULLUP);
    attachInterrupt(D1, button_start_stop_isr, FALLING);
    
    while(1) {
        if (running) {
            uint32_t sec = milliseconds / 1000;
            uint32_t ms = milliseconds % 1000;
            printf("\r%lu.%03lu", sec, ms);
            Delay_Ms(100);
        }
    }
}
```

---

### 3. Counter Reading

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // ตั้งค่า timer ที่ 1000 Hz
    TIM_SimpleInit(TIM_1, 1000);
    TIM_Start(TIM_1);
    
    while(1) {
        uint16_t counter = TIM_GetCounter(TIM_1);
        uint16_t period = TIM_GetPeriod(TIM_1);
        
        // แสดงเป็นเปอร์เซ็นต์
        uint8_t percent = (counter * 100) / period;
        
        printf("Counter: %u/%u (%u%%)\n", counter, period, percent);
        Delay_Ms(100);
    }
}
```

---

### 4. Periodic Task Scheduler

```c
volatile uint8_t task_flags = 0;

#define TASK_1HZ   (1 << 0)
#define TASK_2HZ   (1 << 1)
#define TASK_10HZ  (1 << 2)

volatile uint8_t counter_10hz = 0;

void scheduler_callback(void) {
    counter_10hz++;
    
    // 10 Hz task
    task_flags |= TASK_10HZ;
    
    // 2 Hz task (every 5 ticks)
    if (counter_10hz % 5 == 0) {
        task_flags |= TASK_2HZ;
    }
    
    // 1 Hz task (every 10 ticks)
    if (counter_10hz >= 10) {
        task_flags |= TASK_1HZ;
        counter_10hz = 0;
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Base timer ที่ 10 Hz
    TIM_SimpleInit(TIM_1, 10);
    TIM_AttachInterrupt(TIM_1, scheduler_callback);
    TIM_Start(TIM_1);
    
    while(1) {
        // Task 10Hz - Fast task
        if (task_flags & TASK_10HZ) {
            task_flags &= ~TASK_10HZ;
            // Do fast work
        }
        
        // Task 2Hz - Medium task
        if (task_flags & TASK_2HZ) {
            task_flags &= ~TASK_2HZ;
            printf("2Hz task\n");
        }
        
        // Task 1Hz - Slow task
        if (task_flags & TASK_1HZ) {
            task_flags &= ~TASK_1HZ;
            printf("1Hz task\n");
        }
        
        Delay_Ms(1);
    }
}
```

---

## ขั้นสูง - Advanced Usage

### 1. Advanced Timer Setup (Manual Configuration)

```c
int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // ตั้งค่าแบบละเอียด
    // SystemCoreClock = 48MHz
    // Frequency = 48MHz / ((prescaler+1) * (period+1))
    // Example: 1kHz = 48MHz / (48 * 1000)
    //          prescaler = 47, period = 999
    
    uint16_t prescaler = 47;
    uint16_t period = 999;
    
    TIM_AdvancedInit(TIM_1, prescaler, period, TIM_MODE_UP);
    TIM_AttachInterrupt(TIM_1, timer1_callback);
    TIM_Start(TIM_1);
    
    printf("Timer configured:\n");
    printf("  Prescaler: %u\n", prescaler);
    printf("  Period: %u\n", period);
    printf("  Frequency: %lu Hz\n", 
           SystemCoreClock / ((prescaler+1) * (period+1)));
    
    while(1) {
        Delay_Ms(100);
    }
}
```

**เมื่อไหร่ใช้:**
- ต้องการความถี่ที่แม่นยำมาก
- ต้องการ period เฉพาะ
- ต้องการ down counting mode

---

### 2. Precise Timing Measurement

```c
uint32_t measureExecutionTime(void (*func)(void)) {
    uint16_t start = TIM_GetCounter(TIM_1);
    
    func();  // Execute function
    
    uint16_t end = TIM_GetCounter(TIM_1);
    
    // Handle overflow
    if (end < start) {
        return (TIM_GetPeriod(TIM_1) - start) + end;
    }
    return end - start;
}

void testFunction(void) {
    for (volatile int i = 0; i < 1000; i++);
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    // Timer ที่ 1 MHz (1us resolution)
    TIM_SimpleInit(TIM_1, 1000000);
    TIM_Start(TIM_1);
    
    while(1) {
        uint32_t time_us = measureExecutionTime(testFunction);
        printf("Execution time: %lu us\n", time_us);
        Delay_Ms(1000);
    }
}
```

---

### 3. Dynamic Frequency Adjustment

```c
typedef struct {
    uint32_t min_freq;
    uint32_t max_freq;
    uint32_t current_freq;
    int32_t step;
} FrequencySweep_t;

FrequencySweep_t sweep = {
    .min_freq = 1,
    .max_freq = 10,
    .current_freq = 1,
    .step = 1
};

void sweep_callback(void) {
    static uint32_t tick = 0;
    tick++;
    
    // เปลี่ยนความถี่ทุก 10 ticks
    if (tick >= 10) {
        tick = 0;
        
        sweep.current_freq += sweep.step;
        
        // Reverse direction at limits
        if (sweep.current_freq >= sweep.max_freq) {
            sweep.step = -1;
        } else if (sweep.current_freq <= sweep.min_freq) {
            sweep.step = 1;
        }
        
        TIM_SetFrequency(TIM_2, sweep.current_freq);
        TIM_Start(TIM_2);
        
        printf("Frequency: %lu Hz\n", sweep.current_freq);
    }
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    pinMode(D0, PIN_MODE_OUTPUT);
    
    // Control timer
    TIM_SimpleInit(TIM_1, 10);
    TIM_AttachInterrupt(TIM_1, sweep_callback);
    TIM_Start(TIM_1);
    
    // Output timer
    TIM_SimpleInit(TIM_2, sweep.current_freq);
    TIM_AttachInterrupt(TIM_2, led_blink_callback);
    TIM_Start(TIM_2);
    
    while(1) {
        Delay_Ms(100);
    }
}
```

---

## เทคนิคพิเศษ

### 1. Timer Pool Management

```c
#define MAX_TIMERS 2

typedef struct {
    TIM_Instance instance;
    uint32_t frequency;
    void (*callback)(void);
    uint8_t active;
} TimerConfig_t;

TimerConfig_t timer_pool[MAX_TIMERS] = {
    {TIM_1, 0, NULL, 0},
    {TIM_2, 0, NULL, 0}
};

uint8_t allocateTimer(uint32_t freq, void (*callback)(void)) {
    for (uint8_t i = 0; i < MAX_TIMERS; i++) {
        if (!timer_pool[i].active) {
            timer_pool[i].frequency = freq;
            timer_pool[i].callback = callback;
            timer_pool[i].active = 1;
            
            TIM_SimpleInit(timer_pool[i].instance, freq);
            TIM_AttachInterrupt(timer_pool[i].instance, callback);
            TIM_Start(timer_pool[i].instance);
            
            return i;
        }
    }
    return 0xFF;  // No timer available
}

void freeTimer(uint8_t id) {
    if (id < MAX_TIMERS) {
        TIM_Stop(timer_pool[id].instance);
        TIM_DetachInterrupt(timer_pool[id].instance);
        timer_pool[id].active = 0;
    }
}
```

---

### 2. Timeout Handler

```c
typedef struct {
    uint32_t start_time;
    uint32_t timeout_ms;
    uint8_t active;
} Timeout_t;

void startTimeout(Timeout_t* timeout, uint32_t ms) {
    timeout->start_time = Get_CurrentMs();
    timeout->timeout_ms = ms;
    timeout->active = 1;
}

uint8_t isTimeout(Timeout_t* timeout) {
    if (!timeout->active) return 0;
    
    uint32_t elapsed = Get_CurrentMs() - timeout->start_time;
    if (elapsed >= timeout->timeout_ms) {
        timeout->active = 0;
        return 1;
    }
    return 0;
}

int main(void) {
    SystemCoreClockUpdate();
    Timer_Init();
    
    Timeout_t timeout;
    
    while(1) {
        printf("Starting operation...\n");
        startTimeout(&timeout, 5000);  // 5 second timeout
        
        while (!isTimeout(&timeout)) {
            // Do work
            printf("Working...\n");
            Delay_Ms(500);
        }
        
        printf("Timeout!\n");
        Delay_Ms(1000);
    }
}
```

---

### 3. Frequency Calculation Helper

```c
void calculateTimerParams(uint32_t desired_freq, 
                         uint16_t* prescaler, 
                         uint16_t* period,
                         uint32_t* actual_freq) {
    uint32_t ticks = SystemCoreClock / desired_freq;
    
    if (ticks <= 65536) {
        *prescaler = 0;
        *period = ticks - 1;
    } else {
        *prescaler = (ticks / 65536);
        *period = (ticks / (*prescaler + 1)) - 1;
        
        while (*period > 65535) {
            (*prescaler)++;
            *period = (ticks / (*prescaler + 1)) - 1;
        }
    }
    
    *actual_freq = SystemCoreClock / ((*prescaler + 1) * (*period + 1));
}

int main(void) {
    uint16_t psc, per;
    uint32_t actual;
    
    calculateTimerParams(1000, &psc, &per, &actual);
    
    printf("Desired: 1000 Hz\n");
    printf("Prescaler: %u\n", psc);
    printf("Period: %u\n", per);
    printf("Actual: %lu Hz\n", actual);
    printf("Error: %ld Hz\n", (int32_t)actual - 1000);
}
```

---

## API Reference

### Timer Instances
```c
TIM_1  // TIM1 - Advanced timer
TIM_2  // TIM2 - General purpose timer
```

### Timer Modes
```c
TIM_MODE_UP    // Count up mode
TIM_MODE_DOWN  // Count down mode
```

### Functions

#### TIM_SimpleInit()
```c
void TIM_SimpleInit(TIM_Instance timer, uint32_t frequency_hz);
```
เริ่มต้น timer ด้วยความถี่ที่ต้องการ (Hz)

#### TIM_Start()
```c
void TIM_Start(TIM_Instance timer);
```
เริ่มการนับของ timer

#### TIM_Stop()
```c
void TIM_Stop(TIM_Instance timer);
```
หยุดการนับของ timer

#### TIM_SetFrequency()
```c
void TIM_SetFrequency(TIM_Instance timer, uint32_t frequency_hz);
```
เปลี่ยนความถี่ของ timer

#### TIM_GetCounter()
```c
uint16_t TIM_GetCounter(TIM_Instance timer);
```
อ่านค่า counter ปัจจุบัน

#### TIM_SetCounter()
```c
void TIM_SetCounter(TIM_Instance timer, uint16_t value);
```
ตั้งค่า counter

#### TIM_GetPeriod()
```c
uint16_t TIM_GetPeriod(TIM_Instance timer);
```
อ่านค่า period (ARR register)

#### TIM_AttachInterrupt()
```c
void TIM_AttachInterrupt(TIM_Instance timer, void (*callback)(void));
```
ตั้งค่า update interrupt callback

#### TIM_DetachInterrupt()
```c
void TIM_DetachInterrupt(TIM_Instance timer);
```
ยกเลิก interrupt

#### TIM_AdvancedInit()
```c
void TIM_AdvancedInit(TIM_Instance timer, uint16_t prescaler, 
                      uint16_t period, TIM_Mode mode);
```
ตั้งค่า timer แบบละเอียด

---

## Troubleshooting

### ปัญหา: Timer ไม่ทำงาน
**สาเหตุ:**
- ลืมเรียก `TIM_Start()`
- Callback ไม่ถูกต้อง

**วิธีแก้:**
```c
TIM_SimpleInit(TIM_1, 1);
TIM_AttachInterrupt(TIM_1, callback);
TIM_Start(TIM_1);  // ⚠️ ต้องมี!
```

---

### ปัญหา: Interrupt ไม่ทำงาน
**สาเหตุ:**
- ไม่ได้ attach callback
- Callback signature ผิด

**วิธีแก้:**
```c
// Callback ต้องเป็น void(void)
void my_callback(void) {
    // Code here
}

TIM_AttachInterrupt(TIM_1, my_callback);
```

---

### ปัญหา: ความถี่ไม่ตรง
**สาเหตุ:**
- SystemCoreClock ไม่ถูกต้อง
- ความถี่สูงเกินไป

**วิธีแก้:**
```c
// ตรวจสอบ clock
SystemCoreClockUpdate();
printf("Clock: %lu Hz\n", SystemCoreClock);

// ความถี่สูงสุด = SystemCoreClock / 2
// ที่ 48MHz: max ≈ 24MHz
```

---

## สรุป

SimpleTIM ให้ความสามารถ:
- ✅ ตั้งค่าง่ายด้วยความถี่
- ✅ Auto-calculation ของ parameters
- ✅ Interrupt callbacks
- ✅ เหมาะสำหรับ periodic tasks

**Next Steps:**
- ลองสร้าง stopwatch
- ทำ task scheduler
- ผสมกับ SimpleGPIO และ SimplePWM!
