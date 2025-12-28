/********************************** IWDG Example 02 *******************************
 * File Name          : 02_System_Recovery.c
 * Description        : System recovery with reset cause detection
 * Hardware           : CH32V003
 * Connections        : LED on PC0
 **********************************************************************************/

#include "debug.h"
#include "SimpleHAL/SimpleGPIO.h"
#include "SimpleHAL/SimpleIWDG.h"
#include "Lib/Timer/timer.h"

/*
 * ตัวอย่างระดับกลาง: System Recovery
 * 
 * ในตัวอย่างนี้จะแสดงวิธีการ:
 * 1. ตรวจสอบสาเหตุการ reset
 * 2. จัดการกับ watchdog reset
 * 3. นับจำนวนครั้งที่ระบบ reset
 * 4. จำลองสถานการณ์ระบบค้าง
 */

// Simulated system states
typedef enum {
    STATE_INIT,
    STATE_RUNNING,
    STATE_BUSY,
    STATE_STUCK    // This will trigger watchdog reset
} SystemState_t;

SystemState_t system_state = STATE_INIT;
uint32_t loop_counter = 0;

void PrintResetCause(void)
{
    printf("\n========================================\n");
    printf("  Reset Cause Detection\n");
    printf("========================================\n");
    
    if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
    {
        printf("[!] IWDG Reset detected!\n");
        printf("    System recovered from watchdog reset\n");
    }
    else if(RCC_GetFlagStatus(RCC_FLAG_WWDGRST) == SET)
    {
        printf("[!] WWDG Reset detected!\n");
    }
    else if(RCC_GetFlagStatus(RCC_FLAG_PORRST) == SET)
    {
        printf("[*] Power-On Reset\n");
    }
    else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) == SET)
    {
        printf("[*] External Pin Reset\n");
    }
    else if(RCC_GetFlagStatus(RCC_FLAG_SFTRST) == SET)
    {
        printf("[*] Software Reset\n");
    }
    else
    {
        printf("[*] Unknown Reset Cause\n");
    }
    
    // Clear all reset flags
    RCC_ClearFlag();
    printf("========================================\n\n");
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
    
    // Check and print reset cause
    PrintResetCause();
    
    // Initialize LED
    pinMode(PC0, PIN_MODE_OUTPUT);
    digitalWrite(PC0, LOW);
    
    printf("[*] System Recovery Example\n");
    printf("[*] Initializing IWDG with 2 second timeout\n\n");
    
    // Initialize IWDG with 2 second timeout
    IWDG_SimpleInit(2000);
    
    printf("Commands:\n");
    printf("  - System will run normally for 20 loops\n");
    printf("  - After 20 loops, system will 'stuck' (simulate hang)\n");
    printf("  - IWDG will reset the system after 2 seconds\n");
    printf("  - System will recover and continue\n\n");
    
    while(1)
    {
        loop_counter++;
        
        // Simulate different system states
        if(loop_counter <= 20)
        {
            system_state = STATE_RUNNING;
            
            // Normal operation
            digitalToggle(PC0);
            printf("[%lu] State: RUNNING - Watchdog fed\n", loop_counter);
            
            // Feed watchdog
            IWDG_Feed();
            
            Delay_Ms(500);
        }
        else if(loop_counter == 21)
        {
            system_state = STATE_STUCK;
            
            printf("\n[!] Simulating system hang...\n");
            printf("[!] Watchdog will NOT be fed!\n");
            printf("[!] System will reset in 2 seconds...\n\n");
            
            // Turn on LED to indicate stuck state
            digitalWrite(PC0, HIGH);
            
            // Simulate system hang - infinite loop without feeding watchdog
            while(1)
            {
                // System is stuck here!
                // IWDG will reset the system after 2 seconds
                Delay_Ms(100);
            }
        }
        
        /*
         * หมายเหตุ:
         * - ในการใช้งานจริง ควรมีกลไกตรวจสอบว่าแต่ละ task ทำงานปกติ
         * - ถ้า task ใดค้าง ก็ไม่ควร feed watchdog
         * - ให้ watchdog reset ระบบเพื่อกู้คืนสู่สถานะปกติ
         */
    }
}

/*
 * ทดสอบการทำงาน:
 * 
 * 1. Upload และดูผลลัพธ์:
 *    - ระบบจะทำงานปกติ 20 loops (10 วินาที)
 *    - LED กระพริบทุก 500ms
 *    - หลัง 20 loops ระบบจะ "ค้าง"
 *    - LED จะติดค้าง
 *    - หลัง 2 วินาที IWDG จะ reset ระบบ
 *    - ระบบจะเริ่มต้นใหม่และแสดง "IWDG Reset detected!"
 * 
 * 2. การประยุกต์ใช้:
 *    - ตรวจสอบ reset cause เพื่อบันทึก log
 *    - นับจำนวนครั้งที่ reset (เก็บใน Flash หรือ RTC backup register)
 *    - ถ้า reset บ่อยเกินไป อาจเข้าสู่ safe mode
 *    - ส่งการแจ้งเตือนผ่าน USART หรือ network
 */
