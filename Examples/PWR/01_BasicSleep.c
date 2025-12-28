/********************************** SimplePWR Example *******************************
 * Example Name       : 01_BasicSleep.c
 * Description        : Basic Sleep Mode demonstration
 *                      LED blinks with sleep intervals to save power
 **********************************************************************************/

#include "ch32v00x.h"
#include "SimpleHAL.h"

/*
 * Hardware Setup:
 * - LED connected to PD6 (built-in LED on most CH32V003 boards)
 * - Current meter to measure power consumption (optional)
 * 
 * Expected Behavior:
 * - LED blinks every 1 second
 * - CPU enters sleep mode between blinks
 * - Power consumption drops during sleep (~1-2mA vs ~3-5mA active)
 */

#define LED_PIN     PD6

void main(void)
{
    // Initialize system
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    
    // Configure LED pin
    pinMode(LED_PIN, OUTPUT);
    
    // Configure SysTick for wake-up from sleep
    // SysTick will continue running in sleep mode
    SysTick->CTLR |= (1 << 0);  // Enable SysTick
    
    while(1)
    {
        // Turn LED ON
        digitalWrite(LED_PIN, HIGH);
        Delay_Ms(100);
        
        // Turn LED OFF
        digitalWrite(LED_PIN, LOW);
        
        // Enter sleep mode for ~900ms
        // CPU will wake up on SysTick interrupt
        Delay_Ms(900);
        
        // Alternative: Use explicit sleep mode
        // PWR_Sleep();  // Wake on any interrupt (including SysTick)
    }
}

/*
 * Power Consumption Notes:
 * - Active mode (LED on): ~3-5 mA
 * - Sleep mode: ~1-2 mA (peripherals still running)
 * - Power savings: ~50-60% during sleep
 * 
 * To measure:
 * 1. Connect ammeter in series with VDD
 * 2. Observe current drop when LED is off (sleep mode)
 * 3. Compare with version without sleep (just Delay_Ms)
 */
