/********************************** IWDG Example 01 *******************************
 * File Name          : 01_Basic_IWDG.c
 * Description        : Basic IWDG example - LED blink with watchdog protection
 * Hardware           : CH32V003
 * Connections        : LED on PC0
 **********************************************************************************/

#include "debug.h"
#include "SimpleHAL/SimpleGPIO.h"
#include "SimpleHAL/SimpleIWDG.h"
#include "Lib/Timer/timer.h"

/*
 * ตัวอย่างพื้นฐาน: IWDG (Independent Watchdog)
 * 
 * IWDG คือ watchdog timer ที่ทำงานอิสระจาก system clock
 * ใช้สำหรับป้องกันระบบค้าง โดยจะ reset ระบบถ้าไม่มีการ "feed" watchdog ภายในเวลาที่กำหนด
 * 
 * การทำงาน:
 * 1. กำหนด timeout (เวลาที่ต้อง feed watchdog)
 * 2. เริ่มต้น IWDG
 * 3. ต้อง feed watchdog ก่อนหมดเวลา มิฉะนั้นระบบจะ reset
 * 
 * ในตัวอย่างนี้:
 * - ตั้ง timeout = 1000ms (1 วินาที)
 * - LED กระพริบทุก 500ms
 * - Feed watchdog ทุก 500ms (ก่อนหมดเวลา)
 */

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
    printf("  IWDG Basic Example\n");
    printf("========================================\n");
    
    // Initialize LED
    pinMode(PC0, PIN_MODE_OUTPUT);
    digitalWrite(PC0, LOW);
    
    // Check if last reset was caused by IWDG
    if(IWDG_WasResetCause())
    {
        printf("[!] System recovered from IWDG reset!\n");
        IWDG_ClearResetFlag();
    }
    else
    {
        printf("[*] Normal startup\n");
    }
    
    printf("\n[*] Initializing IWDG...\n");
    printf("    Timeout: 1000ms\n");
    printf("    Feed interval: 500ms\n");
    
    // Initialize IWDG with 1 second timeout
    IWDG_SimpleInit(1000);
    
    printf("[*] IWDG started!\n");
    printf("[*] LED will blink every 500ms\n");
    printf("[*] Watchdog is fed every 500ms\n\n");
    
    uint32_t counter = 0;
    
    while(1)
    {
        // Toggle LED
        digitalToggle(PC0);
        
        // Feed watchdog (CRITICAL!)
        IWDG_Feed();
        
        counter++;
        printf("[%lu] LED toggle + Watchdog fed\n", counter);
        
        // Delay 500ms
        Delay_Ms(500);
        
        /*
         * หมายเหตุ:
         * - ถ้าลบ IWDG_Feed() ออก ระบบจะ reset ทุก 1 วินาที
         * - ถ้าเปลี่ยน delay เป็น 1500ms ระบบจะ reset เพราะ feed ช้าเกินไป
         * - IWDG เหมาะสำหรับป้องกันระบบค้างโดยรวม
         */
    }
}

/*
 * ทดสอบการทำงาน:
 * 
 * 1. Upload โค้ดนี้และดูผลลัพธ์
 *    - LED ควรกระพริบปกติ
 *    - USART แสดงข้อความทุก 500ms
 * 
 * 2. ทดสอบ watchdog reset:
 *    - Comment บรรทัด IWDG_Feed()
 *    - Upload ใหม่
 *    - ระบบจะ reset ทุก 1 วินาที
 *    - USART จะแสดง "System recovered from IWDG reset!"
 * 
 * 3. ทดสอบ timeout:
 *    - เปลี่ยน Delay_Ms(500) เป็น Delay_Ms(1500)
 *    - Upload ใหม่
 *    - ระบบจะ reset เพราะ feed ช้าเกินไป
 */
