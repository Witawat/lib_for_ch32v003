/**
 * @file SimpleHAL.c
 * @brief SimpleHAL Initialization Implementation
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * Optional initialization function for SimpleHAL
 * Auto-initialization is already enabled, so this is optional
 */

#include "SimpleHAL.h"

/**
 * @brief Initialize SimpleHAL (Optional)
 * 
 * @note Auto-initialization is already enabled via constructor attributes
 *       This function is provided for explicit control if needed
 * 
 * @details
 * Currently this function does nothing because:
 * - SimpleDelay auto-initializes via constructor
 * - Other peripherals don't require global initialization
 * 
 * Future additions may include:
 * - Global peripheral configuration
 * - Default pin mappings
 * - System-wide settings
 * 
 * @example
 * int main(void) {
 *     SimpleHAL_Init();  // Optional
 *     Delay_Ms(1000);
 * }
 */
void SimpleHAL_Init(void) {
    // Currently empty - auto-init handles everything
    // Reserved for future use
}
