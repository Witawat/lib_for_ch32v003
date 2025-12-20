#include "simple_task.h"
#include "debug.h" // Use millis() from debug.c

/**
 * @brief  Initialize SimpleTask
 * @note   Actually Delay_Init() in debug.c handles the timer.
 *         We just wrap it here or do nothing if user called Delay_Init.
 */
void TM_Init(void) {
  // Delay_Init() should be called in main() already.
  // If not, we could call it, but let's assume system init does it.
  // For safety, we do nothing here as Delay_Init handles SysTick.
  // Or we can alias it.
}

/**
 * @brief  Get current millis
 */
uint32_t TM_Millis(void) { return millis(); }

// No ISR required here anymore, logic moved to debug.c SysTick_Handler
