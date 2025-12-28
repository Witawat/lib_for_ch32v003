/********************************** SimplePWR Example *******************************
 * Example Name       : 02_StandbyMode.c
 * Description        : Standby Mode with Auto Wake-Up demonstration
 *                      System enters deep sleep and wakes up automatically
 **********************************************************************************/

#include "ch32v00x.h"
#include "SimpleHAL.h"

/*
 * Hardware Setup:
 * - LED connected to PD6 (built-in LED)
 * - Current meter to measure power consumption (optional)
 * 
 * Expected Behavior:
 * - LED blinks 3 times at startup
 * - System enters standby mode for 5 seconds
 * - System wakes up and resets (main() restarts)
 * - LED blinks 3 times again
 * - Cycle repeats indefinitely
 * 
 * Power Consumption:
 * - Active: ~3-5 mA
 * - Standby: ~5 uA (1000x less!)
 */

#define LED_PIN     PD6

void BlinkLED(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        Delay_Ms(200);
        digitalWrite(LED_PIN, LOW);
        Delay_Ms(200);
    }
}

void main(void)
{
    // Initialize system
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    
    // Configure LED pin
    pinMode(LED_PIN, OUTPUT);
    
    // Check if we woke up from standby
    if (PWR_WasStandbyWakeup()) {
        // Clear the flag
        PWR_ClearStandbyFlag();
        
        // Blink LED to indicate wake-up
        BlinkLED(3);
        Delay_Ms(1000);
    } else {
        // First boot - blink LED
        BlinkLED(3);
        Delay_Ms(1000);
    }
    
    // Enter standby mode for 5 seconds
    // System will reset after wake-up and main() will restart
    PWR_Standby(5000);
    
    // This point should never be reached
    while(1);
}

/*
 * Important Notes:
 * 
 * 1. RAM Contents Lost:
 *    - All variables are reset after wake-up
 *    - Use backup registers if you need to preserve data
 * 
 * 2. System Reset:
 *    - Wake-up from standby causes a system reset
 *    - main() function will restart from the beginning
 * 
 * 3. Power Savings:
 *    - Standby mode: ~5 uA
 *    - Active mode: ~3-5 mA
 *    - Savings: 99.9% power reduction!
 * 
 * 4. Wake-up Time:
 *    - Wake-up takes ~1-2ms
 *    - System clock must be re-initialized
 * 
 * Battery Life Example:
 * - 1000mAh battery
 * - Active 1% of time (5mA), Standby 99% (5uA)
 * - Average current: (5 * 1 + 0.005 * 99) / 100 = 0.055 mA
 * - Battery life: 1000 / 0.055 = ~18,000 hours (~2 years!)
 */
