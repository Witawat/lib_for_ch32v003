/********************************** SimplePWR Library *******************************
 * File Name          : SimplePWR.h
 * Author             : SimpleHAL
 * Version            : V1.0.0
 * Date               : 2025-12-22
 * Description        : Simple Power Management library for CH32V003
 *                      Easy-to-use Arduino-like API for Sleep, Standby, PVD, and AWU
 **********************************************************************************/
#ifndef __SIMPLE_PWR_H
#define __SIMPLE_PWR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include "ch32v00x_pwr.h"
#include "ch32v00x_rcc.h"

/******************************************************************************/
/*                              Power Mode Constants                          */
/******************************************************************************/

// Entry methods for low-power modes
#define PWR_ENTRY_WFI           PWR_STANDBYEntry_WFI    // Wait For Interrupt
#define PWR_ENTRY_WFE           PWR_STANDBYEntry_WFE    // Wait For Event

// Wake-up sources (for status checking)
#define PWR_WAKEUP_UNKNOWN      0x00
#define PWR_WAKEUP_INTERRUPT    0x01
#define PWR_WAKEUP_AWU          0x02
#define PWR_WAKEUP_RESET        0x03

/******************************************************************************/
/*                              PVD Voltage Levels                            */
/******************************************************************************/

// PVD Detection Levels (Power Voltage Detector)
#define PWR_PVD_2V9             PWR_PVDLevel_2V9        // 2.9V threshold
#define PWR_PVD_3V1             PWR_PVDLevel_3V1        // 3.1V threshold
#define PWR_PVD_3V3             PWR_PVDLevel_3V3        // 3.3V threshold
#define PWR_PVD_3V5             PWR_PVDLevel_3V5        // 3.5V threshold
#define PWR_PVD_3V7             PWR_PVDLevel_3V7        // 3.7V threshold
#define PWR_PVD_3V9             PWR_PVDLevel_3V9        // 3.9V threshold
#define PWR_PVD_4V1             PWR_PVDLevel_4V1        // 4.1V threshold
#define PWR_PVD_4V4             PWR_PVDLevel_4V4        // 4.4V threshold

/******************************************************************************/
/*                              AWU Prescaler Values                          */
/******************************************************************************/

// Auto Wake-Up Prescaler (for standby mode wake-up timing)
#define PWR_AWU_PRESCALER_1         PWR_AWU_Prescaler_1         // ~30.5 us per count
#define PWR_AWU_PRESCALER_2         PWR_AWU_Prescaler_2         // ~61 us per count
#define PWR_AWU_PRESCALER_4         PWR_AWU_Prescaler_4         // ~122 us per count
#define PWR_AWU_PRESCALER_8         PWR_AWU_Prescaler_8         // ~244 us per count
#define PWR_AWU_PRESCALER_16        PWR_AWU_Prescaler_16        // ~488 us per count
#define PWR_AWU_PRESCALER_32        PWR_AWU_Prescaler_32        // ~976 us per count
#define PWR_AWU_PRESCALER_64        PWR_AWU_Prescaler_64        // ~1.95 ms per count
#define PWR_AWU_PRESCALER_128       PWR_AWU_Prescaler_128       // ~3.9 ms per count
#define PWR_AWU_PRESCALER_256       PWR_AWU_Prescaler_256       // ~7.8 ms per count
#define PWR_AWU_PRESCALER_512       PWR_AWU_Prescaler_512       // ~15.6 ms per count
#define PWR_AWU_PRESCALER_1024      PWR_AWU_Prescaler_1024      // ~31.25 ms per count
#define PWR_AWU_PRESCALER_2048      PWR_AWU_Prescaler_2048      // ~62.5 ms per count
#define PWR_AWU_PRESCALER_4096      PWR_AWU_Prescaler_4096      // ~125 ms per count
#define PWR_AWU_PRESCALER_10240     PWR_AWU_Prescaler_10240     // ~312.5 ms per count
#define PWR_AWU_PRESCALER_61440     PWR_AWU_Prescaler_61440     // ~1.875 s per count

/******************************************************************************/
/*                              Helper Macros                                 */
/******************************************************************************/

// LSI frequency for AWU (typically 128kHz, but can vary)
#define PWR_LSI_FREQ            128000UL

// Maximum AWU window value (6-bit counter)
#define PWR_AWU_MAX_WINDOW      0x3F    // 63

// Calculate AWU timeout in milliseconds
// Formula: timeout_ms = (prescaler * window * 1000) / LSI_FREQ
#define PWR_AWU_TIMEOUT_MS(prescaler_val, window_val) \
    (((prescaler_val) * (window_val) * 1000UL) / PWR_LSI_FREQ)

// Calculate AWU window value from desired timeout
// Formula: window = (timeout_ms * LSI_FREQ) / (prescaler * 1000)
#define PWR_AWU_CALC_WINDOW(prescaler_val, timeout_ms) \
    (((timeout_ms) * PWR_LSI_FREQ) / ((prescaler_val) * 1000UL))

/******************************************************************************/
/*                              Basic API Functions                           */
/******************************************************************************/

/**
 * @brief  Enter Sleep Mode (CPU stops, peripherals continue)
 * @note   CPU will wake up on any interrupt
 * @note   Power consumption: ~1-2mA (depending on active peripherals)
 * @retval None
 * 
 * Example:
 *   PWR_Sleep();  // Enter sleep, wake on any interrupt
 */
void PWR_Sleep(void);

/**
 * @brief  Enter Standby Mode with auto wake-up timeout
 * @param  timeout_ms: Wake-up timeout in milliseconds (1 - 30000ms)
 * @note   This function automatically selects the best prescaler
 * @note   All RAM contents will be lost after wake-up
 * @note   System will reset after wake-up (main() will restart)
 * @note   Power consumption: ~2-5uA in standby mode
 * @retval None
 * 
 * Example:
 *   PWR_Standby(5000);  // Sleep for 5 seconds, then reset
 */
void PWR_Standby(uint32_t timeout_ms);

/**
 * @brief  Enter Standby Mode until external interrupt
 * @note   Wake-up sources: External interrupt on PA0-PA7, PC0-PC7, PD0-PD7
 * @note   All RAM contents will be lost after wake-up
 * @note   System will reset after wake-up (main() will restart)
 * @retval None
 * 
 * Example:
 *   // Configure PA0 as external interrupt before calling
 *   PWR_StandbyUntilInterrupt();  // Sleep until interrupt on configured pin
 */
void PWR_StandbyUntilInterrupt(void);

/******************************************************************************/
/*                              Advanced API Functions                        */
/******************************************************************************/

/**
 * @brief  Enter Sleep Mode with entry method selection
 * @param  entry_method: Entry method
 *           - PWR_ENTRY_WFI: Wait For Interrupt (default)
 *           - PWR_ENTRY_WFE: Wait For Event
 * @retval None
 * 
 * Example:
 *   PWR_EnterSleepMode(PWR_ENTRY_WFI);  // Sleep with WFI
 */
void PWR_EnterSleepMode(uint8_t entry_method);

/**
 * @brief  Enter Standby Mode with entry method selection
 * @param  entry_method: Entry method
 *           - PWR_ENTRY_WFI: Wait For Interrupt
 *           - PWR_ENTRY_WFE: Wait For Event
 * @note   Configure AWU before calling this function for auto wake-up
 * @retval None
 * 
 * Example:
 *   PWR_ConfigureAWU(PWR_AWU_PRESCALER_1024, 31);  // ~1 second
 *   PWR_EnterStandbyMode(PWR_ENTRY_WFI);
 */
void PWR_EnterStandbyMode(uint8_t entry_method);

/**
 * @brief  Configure Auto Wake-Up timer
 * @param  prescaler: Prescaler value (use PWR_AWU_PRESCALER_x constants)
 *           - PWR_AWU_PRESCALER_1     : 1     (max timeout: ~1.9ms)
 *           - PWR_AWU_PRESCALER_2     : 2     (max timeout: ~3.8ms)
 *           - PWR_AWU_PRESCALER_4     : 4     (max timeout: ~7.7ms)
 *           - PWR_AWU_PRESCALER_8     : 8     (max timeout: ~15.4ms)
 *           - PWR_AWU_PRESCALER_16    : 16    (max timeout: ~30.8ms)
 *           - PWR_AWU_PRESCALER_32    : 32    (max timeout: ~61.5ms)
 *           - PWR_AWU_PRESCALER_64    : 64    (max timeout: ~123ms)
 *           - PWR_AWU_PRESCALER_128   : 128   (max timeout: ~246ms)
 *           - PWR_AWU_PRESCALER_256   : 256   (max timeout: ~492ms)
 *           - PWR_AWU_PRESCALER_512   : 512   (max timeout: ~984ms)
 *           - PWR_AWU_PRESCALER_1024  : 1024  (max timeout: ~1.97s)
 *           - PWR_AWU_PRESCALER_2048  : 2048  (max timeout: ~3.94s)
 *           - PWR_AWU_PRESCALER_4096  : 4096  (max timeout: ~7.88s)
 *           - PWR_AWU_PRESCALER_10240 : 10240 (max timeout: ~19.7s)
 *           - PWR_AWU_PRESCALER_61440 : 61440 (max timeout: ~118s)
 * @param  window: Window value (0x00 - 0x3F)
 * @retval None
 * 
 * Example:
 *   // Configure for ~500ms wake-up
 *   uint8_t window = PWR_AWU_CALC_WINDOW(1024, 500);
 *   PWR_ConfigureAWU(PWR_AWU_PRESCALER_1024, window);
 */
void PWR_ConfigureAWU(uint32_t prescaler, uint8_t window);

/**
 * @brief  Enable Power Voltage Detector
 * @param  voltage_level: PVD threshold voltage (use PWR_PVD_x constants)
 *           - PWR_PVD_2V9: 2.9V threshold
 *           - PWR_PVD_3V1: 3.1V threshold
 *           - PWR_PVD_3V3: 3.3V threshold
 *           - PWR_PVD_3V5: 3.5V threshold
 *           - PWR_PVD_3V7: 3.7V threshold
 *           - PWR_PVD_3V9: 3.9V threshold
 *           - PWR_PVD_4V1: 4.1V threshold
 *           - PWR_PVD_4V4: 4.4V threshold
 * @retval None
 * 
 * Example:
 *   PWR_EnablePVD(PWR_PVD_3V3);  // Trigger when VDD < 3.3V
 *   if(PWR_GetPVDStatus()) {
 *       // Voltage is below threshold!
 *   }
 */
void PWR_EnablePVD(uint32_t voltage_level);

/**
 * @brief  Disable Power Voltage Detector
 * @retval None
 * 
 * Example:
 *   PWR_DisablePVD();
 */
void PWR_DisablePVD(void);

/**
 * @brief  Get PVD status
 * @retval 1: VDD is below PVD threshold
 *         0: VDD is above PVD threshold
 * 
 * Example:
 *   if(PWR_GetPVDStatus()) {
 *       printf("Warning: Low voltage detected!\n");
 *   }
 */
uint8_t PWR_GetPVDStatus(void);

/**
 * @brief  Get actual AWU timeout value in milliseconds
 * @param  prescaler: Prescaler value (1, 2, 4, 8, ..., 61440)
 * @param  window: Window value (0x00 - 0x3F)
 * @retval Timeout in milliseconds
 * 
 * Example:
 *   uint32_t timeout = PWR_GetAWUTimeout(1024, 31);  // Returns ~977ms
 */
uint32_t PWR_GetAWUTimeout(uint32_t prescaler, uint8_t window);

/******************************************************************************/
/*                              Utility Functions                             */
/******************************************************************************/

/**
 * @brief  Check if last reset was caused by Standby wake-up
 * @retval 1: Reset was caused by standby wake-up
 *         0: Reset was not caused by standby wake-up
 * 
 * Example:
 *   if(PWR_WasStandbyWakeup()) {
 *       printf("Woke up from standby mode\n");
 *   }
 */
uint8_t PWR_WasStandbyWakeup(void);

/**
 * @brief  Clear standby wake-up flag
 * @retval None
 * 
 * Example:
 *   PWR_ClearStandbyFlag();
 */
void PWR_ClearStandbyFlag(void);

/**
 * @brief  Enable wake-up pin (PA0)
 * @note   PA0 can wake the system from standby mode on rising edge
 * @retval None
 * 
 * Example:
 *   PWR_EnableWakeupPin();  // Enable PA0 as wake-up source
 *   PWR_StandbyUntilInterrupt();
 */
void PWR_EnableWakeupPin(void);

/**
 * @brief  Disable wake-up pin
 * @retval None
 * 
 * Example:
 *   PWR_DisableWakeupPin();
 */
void PWR_DisableWakeupPin(void);

/**
 * @brief  Estimate standby mode current consumption
 * @param  pvd_enabled: 1 if PVD is enabled, 0 otherwise
 * @param  awu_enabled: 1 if AWU is enabled, 0 otherwise
 * @retval Estimated current in microamperes (uA)
 * 
 * Example:
 *   uint32_t current_ua = PWR_EstimateStandbyCurrent(0, 1);
 *   printf("Estimated standby current: %lu uA\n", current_ua);
 */
uint32_t PWR_EstimateStandbyCurrent(uint8_t pvd_enabled, uint8_t awu_enabled);

/**
 * @brief  Calculate battery life in hours
 * @param  battery_mah: Battery capacity in mAh
 * @param  active_time_percent: Percentage of time in active mode (0-100)
 * @param  active_current_ma: Current consumption in active mode (mA)
 * @param  standby_current_ua: Current consumption in standby mode (uA)
 * @retval Estimated battery life in hours
 * 
 * Example:
 *   // 1000mAh battery, 1% active (20mA), 99% standby (5uA)
 *   uint32_t hours = PWR_CalculateBatteryLife(1000, 1, 20, 5);
 *   printf("Battery life: %lu hours\n", hours);
 */
uint32_t PWR_CalculateBatteryLife(uint16_t battery_mah, uint8_t active_time_percent, 
                                   uint16_t active_current_ma, uint16_t standby_current_ua);

/******************************************************************************/
/*                              AWU Timeout Reference Table                   */
/******************************************************************************/
/*
 * Prescaler | Actual Value | Time per Count | Max Timeout (window=63)
 * ----------|--------------|----------------|-------------------------
 *     1     |      1       |    ~7.8 us     |       ~0.49 ms
 *     2     |      2       |   ~15.6 us     |       ~0.98 ms
 *     4     |      4       |   ~31.25 us    |       ~1.97 ms
 *     8     |      8       |   ~62.5 us     |       ~3.94 ms
 *    16     |     16       |   ~125 us      |       ~7.88 ms
 *    32     |     32       |   ~250 us      |      ~15.75 ms
 *    64     |     64       |   ~500 us      |      ~31.5 ms
 *   128     |    128       |    ~1 ms       |      ~63 ms
 *   256     |    256       |    ~2 ms       |     ~126 ms
 *   512     |    512       |    ~4 ms       |     ~252 ms
 *  1024     |   1024       |    ~8 ms       |     ~504 ms
 *  2048     |   2048       |   ~16 ms       |    ~1008 ms (1.0 s)
 *  4096     |   4096       |   ~32 ms       |    ~2016 ms (2.0 s)
 * 10240     |  10240       |   ~80 ms       |    ~5040 ms (5.0 s)
 * 61440     |  61440       |  ~480 ms       |   ~30240 ms (30.2 s)
 *
 * Note: Based on LSI = 128kHz (typical). Actual values may vary ±25%
 */

/******************************************************************************/
/*                              Power Consumption Reference                   */
/******************************************************************************/
/*
 * Mode              | Typical Current | Conditions
 * ------------------|-----------------|----------------------------------
 * Active (Run)      |    ~3-5 mA      | @ 24MHz, all peripherals off
 * Sleep Mode        |    ~1-2 mA      | CPU stopped, peripherals running
 * Standby (no AWU)  |    ~2 uA        | All off, no wake-up timer
 * Standby (AWU on)  |    ~5 uA        | With auto wake-up enabled
 * Standby (PVD on)  |    ~10 uA       | With PVD monitoring
 *
 * Note: Actual consumption depends on temperature, voltage, and silicon variant
 */

#ifdef __cplusplus
}
#endif

#endif /* __SIMPLE_PWR_H */
