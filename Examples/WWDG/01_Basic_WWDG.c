/********************************** WWDG Example 01 *******************************
 * File Name          : 01_Basic_WWDG.c
 * Description        : Basic WWDG example - Window watchdog demonstration
 * Hardware           : CH32V003
 * Connections        : LED on PC0
 **********************************************************************************/

#include "debug.h"
#include "SimpleHAL/SimpleGPIO.h"
#include "SimpleHAL/SimpleWWDG.h"
#include "Lib/Timer/timer.h"

/*
 * ตัวอย่างพื้นฐาน: WWDG (Window Watchdog)
 * 
 * WWDG แตกต่างจาก IWDG ตรงที่มี "window" หรือช่วงเวลาที่กำหนด
 * การ refresh watchdog ต้องทำในช่วงเวลาที่ถูกต้องเท่านั้น
 * 
 * การทำงาน:
 * - Counter นับถอยหลังจากค่าเริ่มต้น (0x7F)
 * - ต้อง refresh เมื่อ: window < counter < 0x40
 * - ถ้า refresh เร็วเกินไป (counter > window): Reset!
 * - ถ้า refresh ช้าเกินไป (counter < 0x40): Reset!
 * 
 * ตัวอย่างนี้:
 * - Counter = 0x7F (127)
 * - Window = 0x50 (80)
 * - Prescaler = 8
 * - ต้อง refresh เมื่อ counter อยู่ระหว่าง 0x50 - 0x40
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
    printf("  WWDG Basic Example\n");
    printf("========================================\n");
    
    // Initialize LED
    pinMode(PC0, PIN_MODE_OUTPUT);
    digitalWrite(PC0, LOW);
    
    printf("\n[*] Window Watchdog Configuration:\n");
    printf("    Counter: 0x7F (127)\n");
    printf("    Window:  0x50 (80)\n");
    printf("    Prescaler: 8\n");
    printf("\n[*] Valid refresh range: 0x50 > counter > 0x40\n");
    printf("    (80 > counter > 64)\n");
    
    // Calculate timeout
    uint32_t timeout = WWDG_CalcTimeout(8, 0x7F);
    printf("\n[*] Timeout: ~%lu ms\n", timeout);
    printf("[*] Refresh interval: ~%lu ms\n\n", timeout / 2);
    
    // Initialize WWDG
    WWDG_SimpleInit(0x7F, 0x50);
    
    printf("[*] WWDG started!\n");
    printf("[*] LED will blink when refreshing watchdog\n\n");
    
    uint32_t counter = 0;
    
    while(1)
    {
        // Toggle LED
        digitalToggle(PC0);
        
        // Refresh watchdog with counter value 0x7F
        WWDG_Refresh(0x7F);
        
        counter++;
        printf("[%lu] Watchdog refreshed\n", counter);
        
        // Delay - must be within valid window
        // Too short: refresh too early (counter > window) -> Reset!
        // Too long: refresh too late (counter < 0x40) -> Reset!
        Delay_Ms(40);  // Safe delay for this configuration
        
        /*
         * การคำนวณ delay ที่เหมาะสม:
         * - Timeout = (4096 * 8 * (127 - 63)) / 24000000 = 87.4ms
         * - Window = (4096 * 8 * (80 - 63)) / 24000000 = 23.2ms
         * - Valid refresh time: 23.2ms - 87.4ms
         * - Safe delay: 40ms (อยู่ในช่วงที่ปลอดภัย)
         */
    }
}

/*
 * ทดสอบการทำงาน:
 * 
 * 1. Upload โค้ดนี้และดูผลลัพธ์:
 *    - LED ควรกระพริบปกติ
 *    - USART แสดงข้อความทุก 40ms
 * 
 * 2. ทดสอบ window violation (refresh เร็วเกินไป):
 *    - เปลี่ยน Delay_Ms(40) เป็น Delay_Ms(10)
 *    - Upload ใหม่
 *    - ระบบจะ reset เพราะ refresh เร็วเกินไป
 * 
 * 3. ทดสอบ timeout (refresh ช้าเกินไป):
 *    - เปลี่ยน Delay_Ms(40) เป็น Delay_Ms(100)
 *    - Upload ใหม่
 *    - ระบบจะ reset เพราะ refresh ช้าเกินไป
 * 
 * 4. ความแตกต่างจาก IWDG:
 *    - IWDG: ต้อง feed ก่อนหมดเวลาเท่านั้น
 *    - WWDG: ต้อง refresh ในช่วงเวลาที่กำหนดเท่านั้น
 *    - WWDG เหมาะสำหรับตรวจสอบ timing ที่เข้มงวด
 */
