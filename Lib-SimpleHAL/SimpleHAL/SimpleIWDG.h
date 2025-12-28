/********************************** SimpleIWDG Library *******************************
 * File Name          : SimpleIWDG.h
 * Author             : SimpleHAL
 * Version            : V1.0.0
 * Date               : 2025-12-21
 * Description        : Simple Independent Watchdog (IWDG) library for CH32V003
 *                      Easy-to-use Arduino-like API
 **********************************************************************************/
#ifndef __SIMPLE_IWDG_H
#define __SIMPLE_IWDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include "ch32v00x_iwdg.h"

/******************************************************************************/
/*                              Prescaler Constants                           */
/******************************************************************************/
#define IWDG_PRESCALER_4        IWDG_Prescaler_4    // Timeout: 0.125ms - 512ms
#define IWDG_PRESCALER_8        IWDG_Prescaler_8    // Timeout: 0.25ms - 1024ms
#define IWDG_PRESCALER_16       IWDG_Prescaler_16   // Timeout: 0.5ms - 2048ms
#define IWDG_PRESCALER_32       IWDG_Prescaler_32   // Timeout: 1ms - 4096ms
#define IWDG_PRESCALER_64       IWDG_Prescaler_64   // Timeout: 2ms - 8192ms
#define IWDG_PRESCALER_128      IWDG_Prescaler_128  // Timeout: 4ms - 16384ms
#define IWDG_PRESCALER_256      IWDG_Prescaler_256  // Timeout: 8ms - 32768ms (32.7s)

/******************************************************************************/
/*                              Helper Macros                                 */
/******************************************************************************/

// LSI frequency (typically 40kHz, but can vary from 30-60kHz)
#define IWDG_LSI_FREQ           40000

// Calculate timeout in milliseconds from prescaler and reload value
// Formula: timeout_ms = (prescaler * reload * 1000) / LSI_FREQ
#define IWDG_TIMEOUT_MS(prescaler_val, reload_val) \
    (((prescaler_val) * (reload_val) * 1000UL) / IWDG_LSI_FREQ)

// Calculate reload value from prescaler and desired timeout in ms
// Formula: reload = (timeout_ms * LSI_FREQ) / (prescaler * 1000)
#define IWDG_CALC_RELOAD(prescaler_val, timeout_ms) \
    (((timeout_ms) * IWDG_LSI_FREQ) / ((prescaler_val) * 1000UL))

// Maximum reload value (12-bit counter)
#define IWDG_MAX_RELOAD         0x0FFF

/******************************************************************************/
/*                              Basic API Functions                           */
/******************************************************************************/

/**
 * @brief  Initialize IWDG with timeout in milliseconds (Simple API)
 * @param  timeout_ms: Desired timeout in milliseconds (1 - 32768ms)
 * @note   This function automatically selects the best prescaler
 * @note   Actual timeout may vary slightly due to LSI frequency tolerance
 * @retval None
 * 
 * Example:
 *   IWDG_SimpleInit(1000);  // 1 second timeout
 *   while(1) {
 *       IWDG_Feed();        // Feed watchdog
 *       // Your code here
 *   }
 */
void IWDG_SimpleInit(uint16_t timeout_ms);

/**
 * @brief  Feed the watchdog (reload counter)
 * @note   Must be called before timeout expires to prevent reset
 * @retval None
 * 
 * Example:
 *   IWDG_Feed();  // Reset watchdog counter
 */
void IWDG_Feed(void);

/******************************************************************************/
/*                              Advanced API Functions                        */
/******************************************************************************/

/**
 * @brief  Initialize IWDG with custom prescaler and reload value
 * @param  prescaler: Prescaler value (use IWDG_PRESCALER_x constants)
 *           - IWDG_PRESCALER_4   : 4   (max timeout: 512ms)
 *           - IWDG_PRESCALER_8   : 8   (max timeout: 1024ms)
 *           - IWDG_PRESCALER_16  : 16  (max timeout: 2048ms)
 *           - IWDG_PRESCALER_32  : 32  (max timeout: 4096ms)
 *           - IWDG_PRESCALER_64  : 64  (max timeout: 8192ms)
 *           - IWDG_PRESCALER_128 : 128 (max timeout: 16384ms)
 *           - IWDG_PRESCALER_256 : 256 (max timeout: 32768ms)
 * @param  reload: Reload value (0x0000 - 0x0FFF)
 * @retval None
 * 
 * Example:
 *   // 500ms timeout with prescaler 32
 *   uint16_t reload = IWDG_CALC_RELOAD(32, 500);
 *   IWDG_Init(IWDG_PRESCALER_32, reload);
 */
void IWDG_Init(uint8_t prescaler, uint16_t reload);

/**
 * @brief  Check if IWDG registers are being updated
 * @retval 1: IWDG is busy updating registers
 *         0: IWDG is ready
 * 
 * Example:
 *   while(IWDG_IsBusy());  // Wait for IWDG ready
 */
uint8_t IWDG_IsBusy(void);

/**
 * @brief  Get actual timeout value in milliseconds
 * @param  prescaler: Prescaler value (4, 8, 16, 32, 64, 128, 256)
 * @param  reload: Reload value (0x0000 - 0x0FFF)
 * @retval Timeout in milliseconds
 * 
 * Example:
 *   uint32_t timeout = IWDG_GetTimeout(32, 625);  // Returns 500ms
 */
uint32_t IWDG_GetTimeout(uint8_t prescaler, uint16_t reload);

/******************************************************************************/
/*                              Utility Functions                             */
/******************************************************************************/

/**
 * @brief  Check if last reset was caused by IWDG
 * @retval 1: Reset was caused by IWDG
 *         0: Reset was not caused by IWDG
 * 
 * Example:
 *   if(IWDG_WasResetCause()) {
 *       printf("System recovered from watchdog reset\n");
 *   }
 */
uint8_t IWDG_WasResetCause(void);

/**
 * @brief  Clear IWDG reset flag
 * @retval None
 * 
 * Example:
 *   IWDG_ClearResetFlag();
 */
void IWDG_ClearResetFlag(void);

/******************************************************************************/
/*                              Prescaler Value Table                         */
/******************************************************************************/
/*
 * Prescaler | Actual Value | Min Timeout | Max Timeout (reload=0xFFF)
 * ----------|--------------|-------------|---------------------------
 *     4     |      4       |   0.1 ms    |        512 ms
 *     8     |      8       |   0.2 ms    |       1024 ms (1.0 s)
 *    16     |     16       |   0.4 ms    |       2048 ms (2.0 s)
 *    32     |     32       |   0.8 ms    |       4096 ms (4.1 s)
 *    64     |     64       |   1.6 ms    |       8192 ms (8.2 s)
 *   128     |    128       |   3.2 ms    |      16384 ms (16.4 s)
 *   256     |    256       |   6.4 ms    |      32768 ms (32.8 s)
 *
 * Note: Based on LSI = 40kHz (typical). Actual values may vary ±25%
 */

#ifdef __cplusplus
}
#endif

#endif /* __SIMPLE_IWDG_H */
