/********************************** SimplePWR Example *******************************
 * Example Name       : 03_PVD_Monitor.c
 * Description        : Power Voltage Detector (PVD) demonstration
 *                      Monitor supply voltage and warn when it drops
 **********************************************************************************/

#include "ch32v00x.h"
#include "SimpleHAL.h"

/*
 * Hardware Setup:
 * - LED connected to PD6 (status indicator)
 * - Variable power supply (2.5V - 5V)
 * - Voltmeter to monitor VDD (optional)
 * 
 * Expected Behavior:
 * - LED blinks slowly when voltage is normal (>3.3V)
 * - LED blinks rapidly when voltage drops below 3.3V
 * - System can take protective action on low voltage
 */

#define LED_PIN         PD6
#define PVD_THRESHOLD   PWR_PVD_3V3  // 3.3V threshold

void main(void)
{
    // Initialize system
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    
    // Configure LED pin
    pinMode(LED_PIN, OUTPUT);
    
    // Enable PVD with 3.3V threshold
    PWR_EnablePVD(PVD_THRESHOLD);
    
    // Startup blink
    for (uint8_t i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        Delay_Ms(100);
        digitalWrite(LED_PIN, LOW);
        Delay_Ms(100);
    }
    
    while(1)
    {
        // Check PVD status
        if (PWR_GetPVDStatus()) {
            // Voltage is BELOW threshold - WARNING!
            // Blink LED rapidly
            digitalWrite(LED_PIN, HIGH);
            Delay_Ms(100);
            digitalWrite(LED_PIN, LOW);
            Delay_Ms(100);
            
            // Optional: Take protective action
            // - Save critical data to flash
            // - Disable power-hungry peripherals
            // - Enter low-power mode
            // - Send warning message
            
        } else {
            // Voltage is ABOVE threshold - Normal operation
            // Blink LED slowly
            digitalWrite(LED_PIN, HIGH);
            Delay_Ms(50);
            digitalWrite(LED_PIN, LOW);
            Delay_Ms(950);
        }
    }
}

/*
 * PVD Voltage Thresholds:
 * 
 * Threshold    | Voltage | Use Case
 * -------------|---------|----------------------------------
 * PWR_PVD_2V9  | 2.9V    | Detect critical low voltage
 * PWR_PVD_3V1  | 3.1V    | 3.0V supply monitoring
 * PWR_PVD_3V3  | 3.3V    | 3.3V supply monitoring (common)
 * PWR_PVD_3V5  | 3.5V    | LiPo battery low warning
 * PWR_PVD_3V7  | 3.7V    | LiPo battery monitoring
 * PWR_PVD_3V9  | 3.9V    | 4.2V supply monitoring
 * PWR_PVD_4V1  | 4.1V    | USB power monitoring
 * PWR_PVD_4V4  | 4.4V    | 5V supply monitoring
 * 
 * 
 * Application Examples:
 * 
 * 1. Battery-Powered Device:
 *    - Set threshold to minimum operating voltage
 *    - Save data and shutdown gracefully when triggered
 * 
 * 2. Data Logger:
 *    - Monitor power supply
 *    - Write data to flash before power loss
 * 
 * 3. Industrial Controller:
 *    - Detect power supply issues
 *    - Enter safe state on low voltage
 * 
 * 4. LiPo Battery Monitor:
 *    - Set threshold to 3.5V (low battery warning)
 *    - Prevent over-discharge (damage threshold: 3.0V)
 * 
 * 
 * Testing PVD:
 * 
 * 1. Use variable power supply
 * 2. Start with 5V
 * 3. Slowly reduce voltage
 * 4. Observe LED change when crossing threshold
 * 5. Increase voltage again to see LED return to normal
 * 
 * Note: PVD has ~100mV hysteresis to prevent oscillation
 */
