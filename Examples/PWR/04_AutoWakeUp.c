/********************************** SimplePWR Example *******************************
 * Example Name       : 04_AutoWakeUp.c
 * Description        : Auto Wake-Up (AWU) timer demonstration
 *                      Configure precise wake-up intervals from standby
 **********************************************************************************/

#include "ch32v00x.h"
#include "SimpleHAL.h"

/*
 * Hardware Setup:
 * - LED connected to PD6
 * - Current meter (optional)
 * 
 * Expected Behavior:
 * - LED blinks once
 * - System enters standby for exactly 2 seconds
 * - System wakes up and blinks again
 * - Cycle repeats with precise 2-second intervals
 */

#define LED_PIN     PD6

// Custom AWU configuration for precise timing
#define WAKEUP_INTERVAL_MS      2000    // 2 seconds
#define AWU_PRESCALER          PWR_AWU_PRESCALER_2048
#define AWU_WINDOW             31       // Calculated for 2 seconds

void main(void)
{
    // Initialize system
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    
    // Configure LED pin
    pinMode(LED_PIN, OUTPUT);
    
    // Blink LED to indicate wake-up
    digitalWrite(LED_PIN, HIGH);
    Delay_Ms(100);
    digitalWrite(LED_PIN, LOW);
    Delay_Ms(100);
    
    // Method 1: Simple API (automatic prescaler selection)
    // PWR_Standby(WAKEUP_INTERVAL_MS);
    
    // Method 2: Advanced API (manual prescaler configuration)
    // Calculate window value for precise timing
    uint8_t window = PWR_AWU_CALC_WINDOW(2048, WAKEUP_INTERVAL_MS);
    
    // Verify calculated timeout
    uint32_t actual_timeout = PWR_GetAWUTimeout(AWU_PRESCALER, window);
    // actual_timeout should be close to WAKEUP_INTERVAL_MS
    
    // Configure AWU with custom parameters
    PWR_ConfigureAWU(AWU_PRESCALER, window);
    
    // Enter standby mode
    PWR_EnterStandbyMode(PWR_ENTRY_WFI);
    
    // This point should never be reached
    while(1);
}

/*
 * AWU Timing Calculation:
 * 
 * Formula: timeout_ms = (prescaler * window * 1000) / LSI_FREQ
 * 
 * Where:
 * - LSI_FREQ = 128kHz (typical, can vary ±25%)
 * - prescaler = 1, 2, 4, 8, ..., 61440
 * - window = 0 to 63
 * 
 * Example Calculations:
 * 
 * 1. For 1 second timeout:
 *    - Use prescaler = 2048
 *    - window = (1000 * 128000) / (2048 * 1000) = 62.5 ≈ 63
 *    - Actual timeout = (2048 * 63 * 1000) / 128000 = 1008ms
 * 
 * 2. For 5 seconds timeout:
 *    - Use prescaler = 10240
 *    - window = (5000 * 128000) / (10240 * 1000) = 62.5 ≈ 63
 *    - Actual timeout = (10240 * 63 * 1000) / 128000 = 5040ms
 * 
 * 3. For 100ms timeout:
 *    - Use prescaler = 256
 *    - window = (100 * 128000) / (256 * 1000) = 50
 *    - Actual timeout = (256 * 50 * 1000) / 128000 = 100ms
 * 
 * 
 * Timing Accuracy:
 * 
 * - LSI frequency varies ±25% across temperature and voltage
 * - For precise timing, calibrate LSI or use external RTC
 * - For non-critical timing, AWU is sufficient
 * 
 * 
 * Common Wake-Up Intervals:
 * 
 * Interval  | Prescaler | Window | Actual Time
 * ----------|-----------|--------|-------------
 * 100ms     | 256       | 50     | 100ms
 * 500ms     | 1024      | 62     | 496ms
 * 1 sec     | 2048      | 63     | 1008ms
 * 2 sec     | 2048      | 31     | 496ms (use 4096, 31 for 2016ms)
 * 5 sec     | 10240     | 63     | 5040ms
 * 10 sec    | 10240     | 31     | 2480ms (use 61440, 21 for 10080ms)
 * 30 sec    | 61440     | 63     | 30240ms
 * 
 * 
 * Application Examples:
 * 
 * 1. Sensor Reading Every 5 Minutes:
 *    - Wake up every 5 minutes
 *    - Read sensor
 *    - Transmit data
 *    - Return to standby
 *    - Battery life: months to years
 * 
 * 2. LED Heartbeat:
 *    - Blink LED every second
 *    - Ultra-low power indicator
 * 
 * 3. Periodic Data Logging:
 *    - Wake up at intervals
 *    - Log data to flash
 *    - Return to sleep
 */
