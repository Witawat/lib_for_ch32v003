/**
 * @file SimpleTIM_Examples.c
 * @brief ตัวอย่างการใช้งาน SimpleTIM Library
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * ไฟล์นี้มีตัวอย่างการใช้งาน SimpleTIM แบบต่างๆ
 * คัดลอกโค้ดไปใส่ใน main.c เพื่อทดสอบ
 */

#include "../SimpleTIM.h"
#include "../SimpleGPIO.h"
#include "../Lib/Timer/timer.h"
#include <stdio.h>

/* ========== Example 1: Basic Timer Interrupt ========== */

volatile uint32_t tick_count = 0;

/**
 * @brief Timer callback - เรียกทุก 1 วินาที
 */
void timer1_callback(void) {
    tick_count++;
    printf("Tick: %lu\r\n", tick_count);
}

/**
 * @brief ตัวอย่างการใช้ timer interrupt พื้นฐาน
 */
void example_basic_timer(void) {
    printf("Basic Timer Example\r\n");
    
    // ตั้งค่า timer ให้ทำงานที่ 1 Hz (1 ครั้งต่อวินาที)
    TIM_SimpleInit(TIM_1, 1);
    TIM_AttachInterrupt(TIM_1, timer1_callback);
    TIM_Start(TIM_1);
    
    while(1) {
        // Main loop ว่าง - ทุกอย่างทำงานใน interrupt
        Delay_Ms(100);
    }
}

/* ========== Example 2: LED Blink with Timer ========== */

volatile uint8_t led_state = 0;

void led_blink_callback(void) {
    led_state = !led_state;
    digitalWrite(PC0, led_state);
}

/**
 * @brief ตัวอย่างการกระพริบ LED ด้วย timer
 */
void example_led_blink_timer(void) {
    printf("LED Blink Timer Example\r\n");
    
    // Setup LED
    pinMode(PC0, PIN_MODE_OUTPUT);
    
    // ตั้งค่า timer ให้ทำงานที่ 2 Hz (กระพริบ 1 Hz)
    TIM_SimpleInit(TIM_2, 2);
    TIM_AttachInterrupt(TIM_2, led_blink_callback);
    TIM_Start(TIM_2);
    
    while(1) {
        Delay_Ms(100);
    }
}

/* ========== Example 3: Multiple Timers ========== */

volatile uint32_t fast_count = 0;
volatile uint32_t slow_count = 0;

void fast_timer_callback(void) {
    fast_count++;
}

void slow_timer_callback(void) {
    slow_count++;
    printf("Fast: %lu, Slow: %lu\r\n", fast_count, slow_count);
}

/**
 * @brief ตัวอย่างการใช้หลาย timers พร้อมกัน
 */
void example_multiple_timers(void) {
    printf("Multiple Timers Example\r\n");
    
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

/* ========== Example 4: Timer Control ========== */

/**
 * @brief ตัวอย่างการควบคุม timer (start/stop)
 */
void example_timer_control(void) {
    printf("Timer Control Example\r\n");
    
    pinMode(PC0, PIN_MODE_OUTPUT);
    
    TIM_SimpleInit(TIM_1, 2);
    TIM_AttachInterrupt(TIM_1, led_blink_callback);
    
    while(1) {
        printf("Starting timer...\r\n");
        TIM_Start(TIM_1);
        Delay_Ms(5000);  // Run for 5 seconds
        
        printf("Stopping timer...\r\n");
        TIM_Stop(TIM_1);
        digitalWrite(PC0, LOW);  // Turn off LED
        Delay_Ms(2000);  // Pause for 2 seconds
    }
}

/* ========== Example 5: Frequency Change ========== */

/**
 * @brief ตัวอย่างการเปลี่ยนความถี่ของ timer
 */
void example_frequency_change(void) {
    printf("Frequency Change Example\r\n");
    
    pinMode(PC0, PIN_MODE_OUTPUT);
    
    TIM_SimpleInit(TIM_1, 1);
    TIM_AttachInterrupt(TIM_1, led_blink_callback);
    TIM_Start(TIM_1);
    
    uint32_t frequencies[] = {1, 2, 4, 8, 4, 2};  // Hz
    uint8_t freq_index = 0;
    
    while(1) {
        Delay_Ms(5000);  // เปลี่ยนทุก 5 วินาที
        
        freq_index = (freq_index + 1) % 6;
        uint32_t new_freq = frequencies[freq_index];
        
        printf("Changing frequency to %lu Hz\r\n", new_freq);
        TIM_SetFrequency(TIM_1, new_freq);
        TIM_Start(TIM_1);
    }
}

/* ========== Example 6: Counter Reading ========== */

/**
 * @brief ตัวอย่างการอ่านค่า counter
 */
void example_counter_reading(void) {
    printf("Counter Reading Example\r\n");
    
    // ตั้งค่า timer ที่ 1000 Hz
    TIM_SimpleInit(TIM_1, 1000);
    TIM_Start(TIM_1);
    
    while(1) {
        uint16_t counter = Simple_TIM_GetCounter(TIM_1);
        uint16_t period = TIM_GetPeriod(TIM_1);
        
        printf("Counter: %u / %u\r\n", counter, period);
        Delay_Ms(100);
    }
}

/* ========== Example 7: Advanced Timer Setup ========== */

/**
 * @brief ตัวอย่างการตั้งค่า timer แบบละเอียด
 */
void example_advanced_setup(void) {
    printf("Advanced Timer Setup Example\r\n");
    
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
    
    printf("Timer configured: PSC=%u, ARR=%u\r\n", prescaler, period);
    printf("Expected frequency: %lu Hz\r\n", 
           SystemCoreClock / ((prescaler+1) * (period+1)));
    
    while(1) {
        Delay_Ms(100);
    }
}

/* ========== Example 8: Stopwatch ========== */

volatile uint32_t milliseconds = 0;

void stopwatch_callback(void) {
    milliseconds++;
}

/**
 * @brief ตัวอย่างการทำ stopwatch
 */
void example_stopwatch(void) {
    printf("Stopwatch Example\r\n");
    printf("Press any key to start/stop\r\n");
    
    // Timer ที่ 1000 Hz (1ms resolution)
    TIM_SimpleInit(TIM_1, 1000);
    TIM_AttachInterrupt(TIM_1, stopwatch_callback);
    
    uint8_t running = 0;
    
    while(1) {
        // Toggle on button press (simulated with delay)
        Delay_Ms(3000);
        
        if (!running) {
            printf("Starting stopwatch...\r\n");
            milliseconds = 0;
            TIM_Start(TIM_1);
            running = 1;
        } else {
            printf("Stopping stopwatch...\r\n");
            TIM_Stop(TIM_1);
            running = 0;
            
            uint32_t seconds = milliseconds / 1000;
            uint32_t ms = milliseconds % 1000;
            printf("Time: %lu.%03lu seconds\r\n", seconds, ms);
        }
    }
}

/* ========== Example 9: Periodic Task Scheduler ========== */

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

/**
 * @brief ตัวอย่างการทำ task scheduler
 */
void example_task_scheduler(void) {
    printf("Task Scheduler Example\r\n");
    
    // Base timer ที่ 10 Hz
    TIM_SimpleInit(TIM_1, 10);
    TIM_AttachInterrupt(TIM_1, scheduler_callback);
    TIM_Start(TIM_1);
    
    while(1) {
        // Task 10Hz
        if (task_flags & TASK_10HZ) {
            task_flags &= ~TASK_10HZ;
            // Fast task
        }
        
        // Task 2Hz
        if (task_flags & TASK_2HZ) {
            task_flags &= ~TASK_2HZ;
            printf("2Hz task\r\n");
        }
        
        // Task 1Hz
        if (task_flags & TASK_1HZ) {
            task_flags &= ~TASK_1HZ;
            printf("1Hz task\r\n");
        }
        
        // Idle
        Delay_Ms(1);
    }
}

/* ========== Example 10: Precise Timing ========== */

/**
 * @brief ตัวอย่างการวัดเวลาแบบแม่นยำ
 */
void example_precise_timing(void) {
    printf("Precise Timing Example\r\n");
    
    // Timer ที่ 1 MHz (1us resolution)
    TIM_SimpleInit(TIM_1, 1000000);
    TIM_Start(TIM_1);
    
    while(1) {
        // วัดเวลาการทำงานของโค้ด
        uint16_t start = Simple_TIM_GetCounter(TIM_1);
        
        // โค้ดที่ต้องการวัดเวลา
        for (volatile int i = 0; i < 1000; i++);
        
        uint16_t end = Simple_TIM_GetCounter(TIM_1);
        uint16_t elapsed = end - start;
        
        printf("Execution time: %u us\r\n", elapsed);
        
        Delay_Ms(1000);
    }
}

/* ========== Main Function Template ========== */

#if 0  // เปลี่ยนเป็น 1 เพื่อ compile

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Timer_Init();
    
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#endif
    
    printf("SimpleTIM Examples\r\n");
    printf("SystemClk: %d Hz\r\n", SystemCoreClock);
    
    // เลือก example ที่ต้องการทดสอบ
    // example_basic_timer();
    // example_led_blink_timer();
    // example_multiple_timers();
    // example_timer_control();
    // example_frequency_change();
    // example_counter_reading();
    // example_advanced_setup();
    // example_stopwatch();
    // example_task_scheduler();
    example_precise_timing();
    
    return 0;
}

#endif
