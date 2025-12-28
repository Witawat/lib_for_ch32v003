/********************************** SimplePWR Example *******************************
 * Example Name       : 07_BatteryMonitor.c
 * Description        : Battery-powered application with voltage monitoring
 *                      Combines ADC, PVD, and power management
 **********************************************************************************/

#include "ch32v00x.h"
#include "SimpleHAL.h"

/*
 * Hardware Setup:
 * - Battery connected to VDD (e.g., 3.7V LiPo)
 * - Voltage divider on PA2 (ADC channel) for battery monitoring
 *   - R1: 10kΩ (to battery+)
 *   - R2: 10kΩ (to GND)
 *   - ADC reads Vbat/2
 * - LED on PD6 (status indicator)
 * - Optional: OLED display for battery percentage
 * 
 * Expected Behavior:
 * - Continuously monitors battery voltage
 * - Displays battery percentage via LED blink pattern
 * - Enters low-power mode when idle
 * - Emergency shutdown on critical low battery
 */

#define LED_PIN             PD6
#define BATTERY_ADC_PIN     PA2

// Battery voltage thresholds (for 3.7V LiPo)
#define BATTERY_FULL_MV     4200    // 4.2V (100%)
#define BATTERY_NOMINAL_MV  3700    // 3.7V (50%)
#define BATTERY_LOW_MV      3500    // 3.5V (20%)
#define BATTERY_CRITICAL_MV 3200    // 3.2V (5%)
#define BATTERY_CUTOFF_MV   3000    // 3.0V (0% - shutdown)

// Voltage divider ratio (R1 = R2 = 10kΩ)
#define VOLTAGE_DIVIDER_RATIO   2

// Function prototypes
uint16_t ReadBatteryVoltage(void);
uint8_t CalculateBatteryPercent(uint16_t voltage_mv);
void DisplayBatteryStatus(uint8_t percent);
void EmergencyShutdown(void);

void main(void)
{
    // Initialize system
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    
    // Configure LED
    pinMode(LED_PIN, OUTPUT);
    
    // Configure ADC for battery monitoring
    analogReadResolution(10);  // 10-bit ADC
    
    // Enable PVD at 3.3V threshold
    PWR_EnablePVD(PWR_PVD_3V3);
    
    // Startup indication
    for (uint8_t i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        Delay_Ms(100);
        digitalWrite(LED_PIN, LOW);
        Delay_Ms(100);
    }
    
    while(1)
    {
        // Read battery voltage
        uint16_t battery_mv = ReadBatteryVoltage();
        
        // Calculate battery percentage
        uint8_t battery_percent = CalculateBatteryPercent(battery_mv);
        
        // Display battery status
        DisplayBatteryStatus(battery_percent);
        
        // Check for critical low battery
        if (battery_mv < BATTERY_CUTOFF_MV || PWR_GetPVDStatus()) {
            EmergencyShutdown();
        }
        
        // Enter sleep mode to save power
        // Wake up every 5 seconds to check battery
        Delay_Ms(5000);
    }
}

uint16_t ReadBatteryVoltage(void)
{
    // Read ADC value
    uint16_t adc_value = analogRead(BATTERY_ADC_PIN);
    
    // Convert to voltage (mV)
    // ADC reference = 3.3V, 10-bit resolution
    // Voltage = (adc_value * 3300) / 1024
    uint32_t voltage_mv = ((uint32_t)adc_value * 3300) / 1024;
    
    // Account for voltage divider
    voltage_mv *= VOLTAGE_DIVIDER_RATIO;
    
    return (uint16_t)voltage_mv;
}

uint8_t CalculateBatteryPercent(uint16_t voltage_mv)
{
    // Linear approximation of LiPo discharge curve
    // 4.2V = 100%, 3.0V = 0%
    
    if (voltage_mv >= BATTERY_FULL_MV) {
        return 100;
    }
    
    if (voltage_mv <= BATTERY_CUTOFF_MV) {
        return 0;
    }
    
    // Calculate percentage
    uint32_t percent = ((uint32_t)(voltage_mv - BATTERY_CUTOFF_MV) * 100) / 
                       (BATTERY_FULL_MV - BATTERY_CUTOFF_MV);
    
    return (uint8_t)percent;
}

void DisplayBatteryStatus(uint8_t percent)
{
    // Display battery percentage using LED blink pattern
    // 100-80%: 5 blinks (full)
    // 79-60%: 4 blinks (good)
    // 59-40%: 3 blinks (medium)
    // 39-20%: 2 blinks (low)
    // 19-0%:  1 blink  (critical)
    
    uint8_t blinks;
    
    if (percent >= 80) {
        blinks = 5;
    } else if (percent >= 60) {
        blinks = 4;
    } else if (percent >= 40) {
        blinks = 3;
    } else if (percent >= 20) {
        blinks = 2;
    } else {
        blinks = 1;
    }
    
    // Blink LED
    for (uint8_t i = 0; i < blinks; i++) {
        digitalWrite(LED_PIN, HIGH);
        Delay_Ms(200);
        digitalWrite(LED_PIN, LOW);
        Delay_Ms(200);
    }
}

void EmergencyShutdown(void)
{
    // Critical low battery - save data and shutdown
    
    // Rapid blink warning
    for (uint8_t i = 0; i < 10; i++) {
        digitalWrite(LED_PIN, HIGH);
        Delay_Ms(50);
        digitalWrite(LED_PIN, LOW);
        Delay_Ms(50);
    }
    
    // Save critical data to flash if needed
    // Flash_WriteData(...);
    
    // Disable all peripherals
    digitalWrite(LED_PIN, LOW);
    
    // Enter standby mode indefinitely
    // System will not wake up until battery is replaced/recharged
    PWR_StandbyUntilInterrupt();
    
    // Should never reach here
    while(1);
}

/*
 * LiPo Battery Discharge Curve:
 * 
 * Voltage | Capacity | State
 * --------|----------|------------------
 * 4.20V   | 100%     | Fully charged
 * 4.00V   | ~90%     | Good
 * 3.80V   | ~70%     | Good
 * 3.70V   | ~50%     | Nominal
 * 3.60V   | ~30%     | Fair
 * 3.50V   | ~20%     | Low (warning)
 * 3.40V   | ~10%     | Very low
 * 3.20V   | ~5%      | Critical
 * 3.00V   | 0%       | Cutoff (damage threshold)
 * 
 * Note: Discharge curve is non-linear. This example uses linear approximation.
 * For accurate readings, use a lookup table or polynomial approximation.
 * 
 * 
 * Battery Protection:
 * 
 * 1. Over-discharge Protection:
 *    - Never discharge below 3.0V
 *    - Permanent damage can occur below 2.5V
 *    - Use PVD to detect low voltage
 * 
 * 2. Over-charge Protection:
 *    - LiPo should not exceed 4.2V
 *    - Use proper charging circuit
 *    - Monitor voltage during charging
 * 
 * 3. Temperature Monitoring:
 *    - LiPo performance degrades in cold
 *    - Danger of fire/explosion if overheated
 *    - Consider adding temperature sensor
 * 
 * 
 * Power Optimization for Battery Life:
 * 
 * 1. Reduce Sampling Rate:
 *    - Check battery every 5-10 seconds (not continuously)
 *    - Enter sleep mode between checks
 * 
 * 2. Use PVD as Primary Monitor:
 *    - PVD consumes less power than ADC
 *    - Use ADC only for detailed percentage
 * 
 * 3. Adaptive Power Management:
 *    - Reduce functionality when battery is low
 *    - Increase sleep time when battery < 20%
 * 
 * 4. Voltage Divider Optimization:
 *    - Use high-value resistors (100kΩ+) to reduce current
 *    - Or use MOSFET to disconnect divider when not measuring
 * 
 * 
 * Battery Life Estimation:
 * 
 * Example: 1000mAh LiPo battery
 * - Active (100ms every 5s): 5mA
 * - Sleep (4900ms): 5uA
 * - Average: (5 * 100 + 0.005 * 4900) / 5000 = 0.105mA
 * - Battery life: 1000 / 0.105 = 9523 hours = 397 days
 * 
 * 
 * Advanced Features:
 * 
 * 1. Coulomb Counting:
 *    - Track charge in/out
 *    - More accurate than voltage-based estimation
 *    - Requires current sensor
 * 
 * 2. State of Health (SOH):
 *    - Track battery degradation over time
 *    - Adjust capacity estimation
 * 
 * 3. Wireless Battery Reporting:
 *    - Send battery status via BLE/WiFi
 *    - Remote monitoring
 * 
 * 4. Smart Charging:
 *    - CC/CV charging algorithm
 *    - Temperature compensation
 *    - Balancing (for multi-cell)
 */
