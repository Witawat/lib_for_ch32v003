/********************************** SimplePWR Example *******************************
 * Example Name       : 05_LowPowerBlink.c
 * Description        : Ultra-low-power LED blink demonstration
 *                      Achieves minimal power consumption for battery applications
 **********************************************************************************/

#include "ch32v00x.h"
#include "SimpleHAL.h"

/*
 * Hardware Setup:
 * - LED connected to PD6
 * - Battery power source (e.g., CR2032 coin cell)
 * - Current meter to verify low power consumption
 * 
 * Expected Behavior:
 * - LED blinks once every 5 seconds
 * - System enters standby mode between blinks
 * - Ultra-low average power consumption
 * 
 * Power Profile:
 * - Active (100ms): ~5mA
 * - Standby (4900ms): ~5uA
 * - Average: (5 * 100 + 0.005 * 4900) / 5000 = ~0.105mA
 * 
 * Battery Life (CR2032 - 220mAh):
 * - 220mAh / 0.105mA = ~2095 hours = ~87 days
 */

#define LED_PIN             PD6
#define BLINK_INTERVAL_MS   5000    // 5 seconds between blinks
#define BLINK_DURATION_MS   100     // LED on for 100ms

void main(void)
{
    // Initialize system
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    
    // Configure LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Quick blink to indicate startup
    digitalWrite(LED_PIN, HIGH);
    Delay_Ms(BLINK_DURATION_MS);
    digitalWrite(LED_PIN, LOW);
    
    // Enter standby mode for (interval - blink_duration)
    // System will reset and restart main() after wake-up
    PWR_Standby(BLINK_INTERVAL_MS - BLINK_DURATION_MS);
    
    // This point should never be reached
    while(1);
}

/*
 * Power Optimization Techniques:
 * 
 * 1. Use Standby Mode (not Sleep):
 *    - Sleep: ~1-2mA
 *    - Standby: ~5uA
 *    - Savings: 99.5%
 * 
 * 2. Minimize Active Time:
 *    - Only wake up when necessary
 *    - Do work quickly and return to standby
 *    - Use efficient code
 * 
 * 3. Disable Unused Peripherals:
 *    - Turn off ADC, USART, SPI, I2C when not needed
 *    - Disable GPIO pull-ups/pull-downs
 * 
 * 4. Use Low-Power Components:
 *    - High-efficiency LEDs
 *    - Low quiescent current regulators
 *    - Minimize external pull-up/pull-down resistors
 * 
 * 
 * Battery Life Calculations:
 * 
 * Example 1: CR2032 (220mAh) - 5 second blink
 * - Active: 100ms @ 5mA = 0.5mA·ms
 * - Standby: 4900ms @ 0.005mA = 24.5mA·ms
 * - Total per cycle: 25mA·ms
 * - Average current: 25 / 5000 = 0.005mA = 5uA
 * - Battery life: 220mAh / 0.005mA = 44,000 hours = 5 years!
 * 
 * Example 2: AAA Battery (1000mAh) - 1 second blink
 * - Active: 100ms @ 5mA = 0.5mA·ms
 * - Standby: 900ms @ 0.005mA = 4.5mA·ms
 * - Total per cycle: 5mA·ms
 * - Average current: 5 / 1000 = 0.005mA = 5uA
 * - Battery life: 1000mAh / 0.005mA = 200,000 hours = 22 years!
 * 
 * 
 * Real-World Considerations:
 * 
 * 1. Battery Self-Discharge:
 *    - CR2032: ~1% per year
 *    - Alkaline: ~2-3% per year
 *    - Actual life limited by self-discharge, not MCU consumption
 * 
 * 2. Temperature Effects:
 *    - Cold temperatures reduce battery capacity
 *    - Hot temperatures increase self-discharge
 * 
 * 3. Voltage Drop:
 *    - Battery voltage decreases over time
 *    - Use PVD to detect low battery
 *    - Consider boost converter for stable operation
 * 
 * 
 * Further Optimization:
 * 
 * 1. Use External RTC:
 *    - Even lower power than AWU
 *    - More accurate timing
 *    - Can wake MCU via interrupt
 * 
 * 2. Use Energy Harvesting:
 *    - Solar panel + supercapacitor
 *    - Infinite battery life
 * 
 * 3. Optimize LED Current:
 *    - Use high-efficiency LED
 *    - Reduce current (still visible at 1-2mA)
 *    - Use PWM for brightness control
 */
