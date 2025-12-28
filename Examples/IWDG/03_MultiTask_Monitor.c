/********************************** IWDG Example 03 *******************************
 * File Name          : 03_MultiTask_Monitor.c
 * Description        : Advanced multi-task monitoring with IWDG
 * Hardware           : CH32V003
 * Connections        : LED1 on PC0, LED2 on PC1, LED3 on PC2
 **********************************************************************************/

#include "debug.h"
#include "SimpleHAL/SimpleGPIO.h"
#include "SimpleHAL/SimpleIWDG.h"
#include "Lib/Timer/timer.h"

/*
 * ตัวอย่างขั้นสูง: Multi-Task Monitoring
 * 
 * ในระบบที่มีหลาย tasks ทำงานพร้อมกัน เราต้องมั่นใจว่าทุก task ทำงานปกติ
 * ถ้า task ใดค้าง ระบบควรถูก reset โดย watchdog
 * 
 * เทคนิค: Task Flags
 * - แต่ละ task ต้อง set flag เมื่อทำงานเสร็จ
 * - Main loop ตรวจสอบ flags ทั้งหมด
 * - ถ้าทุก task ทำงานปกติ จึงจะ feed watchdog
 * - ถ้า task ใดค้าง watchdog จะ reset ระบบ
 */

// Task flags
#define TASK_FLAG_LED1      (1 << 0)
#define TASK_FLAG_LED2      (1 << 1)
#define TASK_FLAG_SENSOR    (1 << 2)
#define TASK_ALL_COMPLETE   (TASK_FLAG_LED1 | TASK_FLAG_LED2 | TASK_FLAG_SENSOR)

volatile uint8_t task_flags = 0;
volatile uint32_t task1_counter = 0;
volatile uint32_t task2_counter = 0;
volatile uint32_t task3_counter = 0;

// Simulate task failure after certain iterations
#define TASK_FAIL_AFTER     50

/*
 * Task 1: LED1 Blink (Fast - 200ms)
 */
void Task_LED1(void)
{
    static uint32_t last_time = 0;
    
    if(millis() - last_time >= 200)
    {
        last_time = millis();
        
        digitalToggle(PC0);
        task1_counter++;
        
        // Set task complete flag
        task_flags |= TASK_FLAG_LED1;
        
        // Simulate task failure after certain iterations
        if(task1_counter == TASK_FAIL_AFTER)
        {
            printf("\n[!] Task 1 FAILED! (Simulated)\n");
            printf("[!] Watchdog will reset system...\n\n");
            
            // Infinite loop - task stuck!
            while(1)
            {
                // Task is stuck here
                Delay_Ms(100);
            }
        }
    }
}

/*
 * Task 2: LED2 Blink (Medium - 500ms)
 */
void Task_LED2(void)
{
    static uint32_t last_time = 0;
    
    if(millis() - last_time >= 500)
    {
        last_time = millis();
        
        digitalToggle(PC1);
        task2_counter++;
        
        // Set task complete flag
        task_flags |= TASK_FLAG_LED2;
    }
}

/*
 * Task 3: Sensor Reading (Slow - 1000ms)
 */
void Task_SensorRead(void)
{
    static uint32_t last_time = 0;
    
    if(millis() - last_time >= 1000)
    {
        last_time = millis();
        
        digitalToggle(PC2);
        task3_counter++;
        
        // Simulate sensor reading
        uint16_t sensor_value = task3_counter * 10;
        
        printf("[Sensor] Reading: %d\n", sensor_value);
        
        // Set task complete flag
        task_flags |= TASK_FLAG_SENSOR;
    }
}

int main(void)
{
    // Initialize system
    SystemCoreClockUpdate();
    Delay_Init();
    Timer_Init();
    
    // Initialize USART for debugging
    USART_Printf_Init(115200);
    printf("\n\n");
    printf("========================================\n");
    printf("  Multi-Task Monitoring with IWDG\n");
    printf("========================================\n");
    
    // Check reset cause
    if(IWDG_WasResetCause())
    {
        printf("[!] System recovered from IWDG reset!\n");
        printf("    One of the tasks was stuck\n");
        IWDG_ClearResetFlag();
    }
    else
    {
        printf("[*] Normal startup\n");
    }
    
    // Initialize LEDs
    pinMode(PC0, PIN_MODE_OUTPUT);  // Task 1 LED
    pinMode(PC1, PIN_MODE_OUTPUT);  // Task 2 LED
    pinMode(PC2, PIN_MODE_OUTPUT);  // Task 3 LED
    
    digitalWrite(PC0, LOW);
    digitalWrite(PC1, LOW);
    digitalWrite(PC2, LOW);
    
    printf("\n[*] Tasks:\n");
    printf("    Task 1: LED1 (PC0) - 200ms\n");
    printf("    Task 2: LED2 (PC1) - 500ms\n");
    printf("    Task 3: Sensor (PC2) - 1000ms\n");
    printf("\n[*] Initializing IWDG with 3 second timeout\n");
    printf("[*] All tasks must complete within 3 seconds\n");
    printf("[*] Task 1 will fail after %d iterations\n\n", TASK_FAIL_AFTER);
    
    // Initialize IWDG with 3 second timeout
    IWDG_SimpleInit(3000);
    
    uint32_t watchdog_feed_count = 0;
    uint32_t last_status_time = 0;
    
    while(1)
    {
        // Run all tasks
        Task_LED1();
        Task_LED2();
        Task_SensorRead();
        
        // Check if all tasks completed
        if(task_flags == TASK_ALL_COMPLETE)
        {
            // All tasks are healthy - feed watchdog
            IWDG_Feed();
            watchdog_feed_count++;
            
            // Clear flags for next cycle
            task_flags = 0;
        }
        
        // Print status every 5 seconds
        if(millis() - last_status_time >= 5000)
        {
            last_status_time = millis();
            
            printf("\n[Status] Watchdog fed: %lu times\n", watchdog_feed_count);
            printf("         Task1: %lu, Task2: %lu, Task3: %lu\n",
                   task1_counter, task2_counter, task3_counter);
        }
        
        // Small delay to prevent CPU overload
        Delay_Ms(10);
        
        /*
         * หมายเหตุ:
         * - ถ้า task ใดค้าง flag จะไม่ครบ
         * - Watchdog จะไม่ถูก feed
         * - ระบบจะ reset หลัง 3 วินาที
         * - เหมาะสำหรับระบบที่ต้องการความน่าเชื่อถือสูง
         */
    }
}

/*
 * ทดสอบการทำงาน:
 * 
 * 1. Upload และดูผลลัพธ์:
 *    - LED ทั้ง 3 ดวงกระพริบด้วยความเร็วต่างกัน
 *    - USART แสดงสถานะทุก 5 วินาที
 *    - หลัง 50 iterations Task 1 จะค้าง
 *    - Watchdog จะ reset ระบบหลัง 3 วินาที
 *    - ระบบจะเริ่มต้นใหม่
 * 
 * 2. การประยุกต์ใช้:
 *    - ระบบ Real-time ที่มีหลาย tasks
 *    - ระบบควบคุมอุตสาหกรรม
 *    - ระบบ IoT ที่ต้องทำงานต่อเนื่อง
 *    - ระบบที่ต้องการ automatic recovery
 * 
 * 3. เทคนิคขั้นสูง:
 *    - ใช้ RTC backup register เก็บจำนวนครั้งที่ reset
 *    - บันทึก task ที่ค้างใน Flash memory
 *    - ส่งการแจ้งเตือนผ่าน network
 *    - เข้าสู่ safe mode ถ้า reset บ่อยเกินไป
 */
