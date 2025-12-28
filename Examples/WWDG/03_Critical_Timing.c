/********************************** WWDG Example 03 *******************************
 * File Name          : 03_Critical_Timing.c
 * Description        : Advanced WWDG for critical timing protection
 * Hardware           : CH32V003
 * Connections        : LED on PC0, Button on PD6
 **********************************************************************************/

#include "debug.h"
#include "SimpleHAL/SimpleGPIO.h"
#include "SimpleHAL/SimpleWWDG.h"
#include "Lib/Timer/timer.h"

/*
 * ตัวอย่างขั้นสูง: Critical Timing Protection
 * 
 * WWDG เหมาะสำหรับระบบที่ต้องการควบคุม timing อย่างเข้มงวด
 * เช่น:
 * - ระบบควบคุมมอเตอร์ (ต้อง update PWM ในช่วงเวลาที่กำหนด)
 * - ระบบสื่อสาร (ต้องส่งข้อมูลตาม protocol timing)
 * - ระบบ safety-critical (ต้องตรวจสอบ sensor ตามกำหนด)
 * 
 * ในตัวอย่างนี้จะจำลองระบบควบคุมที่:
 * 1. ต้องอ่านค่า sensor ทุก 50ms (±10ms)
 * 2. ถ้าอ่านเร็วหรือช้าเกินไป -> ระบบไม่ปลอดภัย -> Reset!
 * 3. ใช้ WWDG ตรวจสอบ timing
 */

// Timing constraints
#define SENSOR_READ_INTERVAL_MS     50
#define TIMING_TOLERANCE_MS         10
#define MIN_INTERVAL_MS             (SENSOR_READ_INTERVAL_MS - TIMING_TOLERANCE_MS)
#define MAX_INTERVAL_MS             (SENSOR_READ_INTERVAL_MS + TIMING_TOLERANCE_MS)

// Simulation control
volatile uint8_t simulate_timing_violation = 0;
volatile uint32_t sensor_read_count = 0;
volatile uint32_t timing_violation_count = 0;

/*
 * Simulated sensor reading
 */
uint16_t ReadCriticalSensor(void)
{
    sensor_read_count++;
    
    // Simulate sensor value
    uint16_t value = sensor_read_count * 10;
    
    return value;
}

/*
 * Critical task with strict timing requirement
 */
void CriticalTask(void)
{
    static uint32_t last_time = 0;
    uint32_t current_time = millis();
    uint32_t elapsed = current_time - last_time;
    
    if(last_time == 0)
    {
        // First run
        last_time = current_time;
        return;
    }
    
    // Check timing constraint
    if(elapsed < MIN_INTERVAL_MS || elapsed > MAX_INTERVAL_MS)
    {
        timing_violation_count++;
        printf("[!] TIMING VIOLATION! Elapsed: %lu ms (Expected: %d ±%d ms)\n",
               elapsed, SENSOR_READ_INTERVAL_MS, TIMING_TOLERANCE_MS);
        
        // Don't refresh WWDG - let it reset the system
        return;
    }
    
    // Timing is OK - read sensor
    uint16_t sensor_value = ReadCriticalSensor();
    
    printf("[OK] Sensor read: %d (Timing: %lu ms)\n", sensor_value, elapsed);
    
    // Refresh WWDG to indicate task completed successfully
    WWDG_Refresh(0x7F);
    
    // Toggle LED to show activity
    digitalToggle(PC0);
    
    last_time = current_time;
}

/*
 * Button interrupt handler - simulate timing violation
 */
void Button_Callback(void)
{
    simulate_timing_violation = 1;
    printf("\n[!] Button pressed - Simulating timing violation!\n\n");
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
    printf("  WWDG Critical Timing Protection\n");
    printf("========================================\n");
    
    // Initialize LED
    pinMode(PC0, PIN_MODE_OUTPUT);
    digitalWrite(PC0, LOW);
    
    // Initialize button with interrupt
    pinMode(PD6, PIN_MODE_INPUT_PULLUP);
    attachInterrupt(PD6, Button_Callback, FALLING);
    
    printf("\n[*] Critical Task Requirements:\n");
    printf("    Sensor read interval: %d ms\n", SENSOR_READ_INTERVAL_MS);
    printf("    Timing tolerance: ±%d ms\n", TIMING_TOLERANCE_MS);
    printf("    Valid range: %d - %d ms\n", MIN_INTERVAL_MS, MAX_INTERVAL_MS);
    
    printf("\n[*] WWDG Configuration:\n");
    printf("    Counter: 0x7F\n");
    printf("    Window: 0x60\n");
    printf("    Prescaler: 8\n");
    
    uint32_t timeout = WWDG_CalcTimeout(8, 0x7F);
    printf("    Timeout: ~%lu ms\n", timeout);
    
    printf("\n[*] Press button (PD6) to simulate timing violation\n\n");
    
    // Initialize WWDG
    // Window = 0x60 allows refresh in specific timing window
    WWDG_SimpleInit(0x7F, 0x60);
    
    printf("[*] System started!\n");
    printf("[*] LED blinks when sensor is read successfully\n\n");
    
    uint32_t last_task_time = millis();
    
    while(1)
    {
        uint32_t current_time = millis();
        
        // Normal operation - run task every 50ms
        if(!simulate_timing_violation)
        {
            if(current_time - last_task_time >= SENSOR_READ_INTERVAL_MS)
            {
                CriticalTask();
                last_task_time = current_time;
            }
        }
        else
        {
            // Simulate timing violation - delay too long
            printf("[!] Delaying task execution...\n");
            Delay_Ms(200);  // Delay too long - timing violation!
            
            CriticalTask();
            
            // Reset simulation flag
            simulate_timing_violation = 0;
            last_task_time = millis();
        }
        
        // Small delay
        Delay_Ms(1);
        
        /*
         * การทำงาน:
         * 1. Task ทำงานทุก 50ms (±10ms)
         * 2. ถ้า timing ถูกต้อง -> refresh WWDG
         * 3. ถ้า timing ผิดพลาด -> ไม่ refresh WWDG -> Reset!
         * 4. กดปุ่มเพื่อจำลอง timing violation
         */
    }
}

/*
 * ทดสอบการทำงาน:
 * 
 * 1. Upload และดูผลลัพธ์:
 *    - LED กระพริบทุก 50ms
 *    - USART แสดงค่า sensor และ timing
 *    - ระบบทำงานปกติ
 * 
 * 2. ทดสอบ timing violation:
 *    - กดปุ่ม (PD6)
 *    - Task จะ delay 200ms (เกินกำหนด)
 *    - WWDG จะ reset ระบบ
 *    - ระบบจะเริ่มต้นใหม่
 * 
 * 3. การประยุกต์ใช้:
 *    - ระบบควบคุมมอเตอร์ (PWM update timing)
 *    - ระบบสื่อสาร (Protocol timing)
 *    - ระบบ safety-critical (Sensor monitoring)
 *    - ระบบ real-time (Task scheduling)
 * 
 * 4. เทคนิคขั้นสูง:
 *    - ใช้ร่วมกับ IWDG (IWDG = overall, WWDG = critical task)
 *    - บันทึก timing violation ใน Flash
 *    - ปรับ window ตาม load ของระบบ
 *    - ใช้ interrupt เพื่อบันทึก log ก่อน reset
 * 
 * 5. ข้อควรระวัง:
 *    - WWDG เข้มงวดกว่า IWDG มาก
 *    - ต้องคำนวณ timing อย่างระมัดระวัง
 *    - ไม่เหมาะสำหรับระบบที่มี timing ไม่แน่นอน
 *    - ควรทดสอบให้ดีก่อนใช้ใน production
 */
