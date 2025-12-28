/********************************** WWDG Example 02 *******************************
 * File Name          : 02_WWDG_Interrupt.c
 * Description        : WWDG with Early Wakeup Interrupt
 * Hardware           : CH32V003
 * Connections        : LED1 on PC0, LED2 on PC1
 **********************************************************************************/

#include "debug.h"
#include "SimpleHAL/SimpleGPIO.h"
#include "SimpleHAL/SimpleWWDG.h"
#include "Lib/Timer/timer.h"

/*
 * ตัวอย่างระดับกลาง: WWDG with Early Wakeup Interrupt
 * 
 * WWDG มีฟีเจอร์พิเศษคือ Early Wakeup Interrupt (EWI)
 * Interrupt จะถูกเรียกเมื่อ counter = 0x40 (ก่อนจะ reset)
 * 
 * ประโยชน์:
 * 1. แจ้งเตือนก่อนระบบจะ reset
 * 2. มีโอกาสบันทึก log หรือสถานะ
 * 3. ทำ emergency shutdown อย่างปลอดภัย
 * 4. ส่งการแจ้งเตือน
 * 
 * ในตัวอย่างนี้:
 * - LED1 (PC0): กระพริบปกติ
 * - LED2 (PC1): กระพริบเมื่อเกิด Early Wakeup Interrupt
 */

volatile uint32_t early_wakeup_count = 0;

/*
 * Early Wakeup Interrupt Callback
 * ฟังก์ชันนี้จะถูกเรียกเมื่อ counter = 0x40
 */
void WWDG_EarlyWakeup_Callback(void)
{
    early_wakeup_count++;
    
    // Toggle LED2 to indicate interrupt
    digitalToggle(PC1);
    
    printf("[!] Early Wakeup Interrupt! Count: %lu\n", early_wakeup_count);
    
    // Refresh watchdog in interrupt to prevent reset
    WWDG_Refresh(0x7F);
    
    /*
     * หมายเหตุ:
     * - ใน interrupt handler ควรทำงานให้เร็วที่สุด
     * - ไม่ควรใช้ printf() ใน interrupt (ใช้เพื่อการสาธิตเท่านั้น)
     * - ควรตั้ง flag แล้วประมวลผลใน main loop
     */
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
    printf("  WWDG Early Wakeup Interrupt Example\n");
    printf("========================================\n");
    
    // Initialize LEDs
    pinMode(PC0, PIN_MODE_OUTPUT);  // Normal operation LED
    pinMode(PC1, PIN_MODE_OUTPUT);  // Interrupt LED
    
    digitalWrite(PC0, LOW);
    digitalWrite(PC1, LOW);
    
    printf("\n[*] Configuration:\n");
    printf("    Counter: 0x7F (127)\n");
    printf("    Window:  0x50 (80)\n");
    printf("    Prescaler: 8\n");
    printf("\n[*] LEDs:\n");
    printf("    PC0 (LED1): Normal operation\n");
    printf("    PC1 (LED2): Early Wakeup Interrupt\n");
    
    // Calculate timeout
    uint32_t timeout = WWDG_CalcTimeout(8, 0x7F);
    printf("\n[*] Timeout: ~%lu ms\n", timeout);
    printf("[*] Early Wakeup at: ~%lu ms\n\n", timeout - 10);
    
    // Set callback function
    WWDG_SetCallback(WWDG_EarlyWakeup_Callback);
    
    // Initialize WWDG with interrupt
    WWDG_InitWithInterrupt(0x7F, 0x50, WWDG_PRESCALER_8);
    
    printf("[*] WWDG with interrupt started!\n");
    printf("[*] Interrupt will trigger when counter = 0x40\n\n");
    
    uint32_t loop_counter = 0;
    
    while(1)
    {
        // Toggle LED1 for normal operation
        digitalToggle(PC0);
        
        loop_counter++;
        printf("[%lu] Main loop - Early wakeup count: %lu\n", 
               loop_counter, early_wakeup_count);
        
        // Delay - watchdog will be refreshed in interrupt
        Delay_Ms(100);
        
        /*
         * การทำงาน:
         * 1. Main loop ทำงานปกติ
         * 2. เมื่อ WWDG counter = 0x40 -> Interrupt!
         * 3. Interrupt handler refresh watchdog
         * 4. ระบบทำงานต่อไปได้
         * 
         * ถ้าไม่ refresh ใน interrupt:
         * - ระบบจะ reset ทันทีหลัง interrupt
         * - ใช้สำหรับ emergency shutdown
         */
    }
}

/*
 * การเพิ่ม Interrupt Handler ใน ch32v00x_it.c:
 * 
 * เพิ่มโค้ดนี้ใน ch32v00x_it.c:
 * 
 * void WWDG_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
 * void WWDG_IRQHandler(void)
 * {
 *     WWDG_IRQHandler_Callback();
 * }
 * 
 * หมายเหตุ:
 * - ต้องเพิ่ม #include "SimpleHAL/SimpleWWDG.h" ใน ch32v00x_it.c
 * - Interrupt handler จะเรียก callback ที่เรากำหนด
 */

/*
 * ทดสอบการทำงาน:
 * 
 * 1. เพิ่ม interrupt handler ใน ch32v00x_it.c
 * 2. Upload และดูผลลัพธ์:
 *    - LED1 กระพริบทุก 100ms (main loop)
 *    - LED2 กระพริบเมื่อเกิด interrupt (~87ms)
 *    - USART แสดงข้อความ interrupt
 * 
 * 3. ทดสอบ emergency shutdown:
 *    - Comment บรรทัด WWDG_Refresh() ใน callback
 *    - Upload ใหม่
 *    - ระบบจะ reset ทันทีหลัง interrupt
 * 
 * 4. การประยุกต์ใช้:
 *    - บันทึก log ก่อน reset
 *    - ส่งการแจ้งเตือนผ่าน USART/Network
 *    - ปิดระบบอย่างปลอดภัย (save data, turn off peripherals)
 *    - แสดงสถานะบน LCD/OLED
 */
