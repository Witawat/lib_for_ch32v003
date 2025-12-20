#ifndef __SIMPLE_TASK_H
#define __SIMPLE_TASK_H

#include <ch32v00x.h> // Ensure we have system defs
#include <stdint.h>   // For uint32_t


/**
 * @brief  Initialize Timer for 1ms Tick
 */
void TM_Init(void);

/**
 * @brief  Get current millis
 */
uint32_t TM_Millis(void);

/**
 * @brief  Calculate time passed since 'start'
 */
#define TM_Diff(start) (TM_Millis() - (start))

/**
 * @brief  Macro for periodic execution (RUN_EVERY) - Overflow Safe
 * @param  ms: Interval in milliseconds
 */
#define RUN_EVERY(ms)                                                          \
  static uint32_t _last_##__LINE__ = 0;                                        \
  if (TM_Millis() - _last_##__LINE__ >= ms &&                                  \
      (_last_##__LINE__ = TM_Millis(), 1))

/**
 * @brief  Macro for running code ONCE after a delay
 * @param  ms: Delay in milliseconds before executing
 */
#define RUN_ONCE(ms)                                                           \
  static uint32_t _start_##__LINE__ = 0;                                       \
  static uint8_t _done_##__LINE__ = 0;                                         \
  if (_start_##__LINE__ == 0)                                                  \
    _start_##__LINE__ = TM_Millis();                                           \
  if (!_done_##__LINE__ && (TM_Millis() - _start_##__LINE__ >= ms) &&          \
      (_done_##__LINE__ = 1))

#endif
