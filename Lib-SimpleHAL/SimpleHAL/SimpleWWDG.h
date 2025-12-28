/********************************** SimpleWWDG Library *******************************
 * File Name          : SimpleWWDG.h
 * Author             : SimpleHAL
 * Version            : V1.0.0
 * Date               : 2025-12-21
 * Description        : Simple Window Watchdog (WWDG) library for CH32V003
 *                      Easy-to-use Arduino-like API with interrupt support
 **********************************************************************************/
#ifndef __SIMPLE_WWDG_H
#define __SIMPLE_WWDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v00x.h"
#include "ch32v00x_wwdg.h"
#include "ch32v00x_rcc.h"

/******************************************************************************/
/*                              Prescaler Constants                           */
/******************************************************************************/
#define WWDG_PRESCALER_1        WWDG_Prescaler_1    // PCLK1/4096/1
#define WWDG_PRESCALER_2        WWDG_Prescaler_2    // PCLK1/4096/2
#define WWDG_PRESCALER_4        WWDG_Prescaler_4    // PCLK1/4096/4
#define WWDG_PRESCALER_8        WWDG_Prescaler_8    // PCLK1/4096/8

/******************************************************************************/
/*                              Counter Constants                             */
/******************************************************************************/
#define WWDG_COUNTER_MIN        0x40    // Minimum counter value
#define WWDG_COUNTER_MAX        0x7F    // Maximum counter value
#define WWDG_WINDOW_MIN         0x40    // Minimum window value
#define WWDG_WINDOW_MAX         0x7F    // Maximum window value

/******************************************************************************/
/*                              Helper Macros                                 */
/******************************************************************************/

// Calculate timeout in microseconds
// Formula: timeout_us = (4096 * prescaler * (counter - 0x3F)) / PCLK1
// Assuming PCLK1 = 24MHz
#define WWDG_PCLK1_FREQ         24000000UL

#define WWDG_TIMEOUT_US(prescaler_val, counter) \
    ((4096UL * (prescaler_val) * ((counter) - 0x3F) * 1000000UL) / WWDG_PCLK1_FREQ)

#define WWDG_TIMEOUT_MS(prescaler_val, counter) \
    (WWDG_TIMEOUT_US(prescaler_val, counter) / 1000UL)

/******************************************************************************/
/*                              Basic API Functions                           */
/******************************************************************************/

/**
 * @brief  Initialize WWDG with simple configuration
 * @param  counter: Initial counter value (0x40 - 0x7F)
 * @param  window: Window value (0x40 - 0x7F)
 * @note   Counter must be refreshed when: window < counter < 0x40
 * @note   Uses default prescaler of 8 (WWDG_PRESCALER_8)
 * @retval None
 * 
 * Example:
 *   WWDG_SimpleInit(0x7F, 0x50);  // Counter=127, Window=80
 *   while(1) {
 *       WWDG_Refresh(0x7F);       // Refresh when counter < 0x50
 *       // Your code here
 *   }
 */
void WWDG_SimpleInit(uint8_t counter, uint8_t window);

/**
 * @brief  Refresh WWDG counter
 * @param  counter: New counter value (0x40 - 0x7F)
 * @note   Must be called when counter is within window
 * @retval None
 * 
 * Example:
 *   WWDG_Refresh(0x7F);  // Reset counter to 127
 */
void WWDG_Refresh(uint8_t counter);

/******************************************************************************/
/*                              Advanced API Functions                        */
/******************************************************************************/

/**
 * @brief  Initialize WWDG with custom prescaler
 * @param  counter: Initial counter value (0x40 - 0x7F)
 * @param  window: Window value (0x40 - 0x7F)
 * @param  prescaler: Prescaler value (use WWDG_PRESCALER_x constants)
 *           - WWDG_PRESCALER_1: Prescaler = 1
 *           - WWDG_PRESCALER_2: Prescaler = 2
 *           - WWDG_PRESCALER_4: Prescaler = 4
 *           - WWDG_PRESCALER_8: Prescaler = 8
 * @retval None
 * 
 * Example:
 *   WWDG_Init(0x7F, 0x50, WWDG_PRESCALER_4);
 */
void WWDG_Init(uint8_t counter, uint8_t window, uint32_t prescaler);

/**
 * @brief  Initialize WWDG with Early Wakeup Interrupt
 * @param  counter: Initial counter value (0x40 - 0x7F)
 * @param  window: Window value (0x40 - 0x7F)
 * @param  prescaler: Prescaler value (use WWDG_PRESCALER_x constants)
 * @note   Interrupt triggers when counter reaches 0x40
 * @note   Must call WWDG_SetCallback() to set interrupt handler
 * @retval None
 * 
 * Example:
 *   void WWDG_EarlyWarning(void) {
 *       printf("Warning: WWDG about to reset!\n");
 *   }
 *   
 *   WWDG_SetCallback(WWDG_EarlyWarning);
 *   WWDG_InitWithInterrupt(0x7F, 0x50, WWDG_PRESCALER_8);
 */
void WWDG_InitWithInterrupt(uint8_t counter, uint8_t window, uint32_t prescaler);

/**
 * @brief  Set callback function for Early Wakeup Interrupt
 * @param  callback: Pointer to callback function
 * @retval None
 * 
 * Example:
 *   void MyCallback(void) {
 *       // Handle early wakeup
 *   }
 *   WWDG_SetCallback(MyCallback);
 */
void WWDG_SetCallback(void (*callback)(void));

/******************************************************************************/
/*                              Utility Functions                             */
/******************************************************************************/

/**
 * @brief  Calculate timeout in milliseconds
 * @param  prescaler: Prescaler value (1, 2, 4, 8)
 * @param  counter: Counter value (0x40 - 0x7F)
 * @retval Timeout in milliseconds
 * 
 * Example:
 *   uint32_t timeout = WWDG_CalcTimeout(8, 0x7F);
 */
uint32_t WWDG_CalcTimeout(uint32_t prescaler, uint8_t counter);

/**
 * @brief  Get Early Wakeup Interrupt flag status
 * @retval 1: Interrupt flag is set
 *         0: Interrupt flag is not set
 * 
 * Example:
 *   if(WWDG_GetInterruptFlag()) {
 *       // Handle interrupt
 *   }
 */
uint8_t WWDG_GetInterruptFlag(void);

/**
 * @brief  Clear Early Wakeup Interrupt flag
 * @retval None
 * 
 * Example:
 *   WWDG_ClearInterruptFlag();
 */
void WWDG_ClearInterruptFlag(void);

/**
 * @brief  Disable WWDG (reset peripheral)
 * @note   This completely resets the WWDG peripheral
 * @retval None
 * 
 * Example:
 *   WWDG_Disable();
 */
void WWDG_Disable(void);

/******************************************************************************/
/*                              Timing Reference Table                        */
/******************************************************************************/
/*
 * WWDG Timing Calculation (PCLK1 = 24MHz):
 * 
 * Prescaler | Counter Range | Min Timeout | Max Timeout
 * ----------|---------------|-------------|-------------
 *     1     |  0x40 - 0x7F  |   170 µs    |   10.9 ms
 *     2     |  0x40 - 0x7F  |   341 µs    |   21.8 ms
 *     4     |  0x40 - 0x7F  |   683 µs    |   43.7 ms
 *     8     |  0x40 - 0x7F  |   1.37 ms   |   87.4 ms
 *
 * Formula: timeout = (4096 * prescaler * (counter - 0x3F)) / PCLK1
 * 
 * Window Watchdog Behavior:
 * - Counter decrements from initial value
 * - Reset occurs if:
 *   1. Counter reaches 0x3F (timeout)
 *   2. Counter is refreshed when counter > window (too early)
 * - Refresh is valid only when: window < counter < 0x40
 * - Early Wakeup Interrupt triggers when counter = 0x40
 */

/******************************************************************************/
/*                              Interrupt Handler                             */
/******************************************************************************/

/**
 * @brief  WWDG Interrupt Handler (must be defined in ch32v00x_it.c)
 * @note   This function is called automatically by the system
 * @retval None
 * 
 * Add to ch32v00x_it.c:
 *   void WWDG_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
 *   void WWDG_IRQHandler(void) {
 *       WWDG_IRQHandler_Callback();
 *   }
 */
void WWDG_IRQHandler_Callback(void);

#ifdef __cplusplus
}
#endif

#endif /* __SIMPLE_WWDG_H */
