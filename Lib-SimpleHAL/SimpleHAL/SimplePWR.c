/********************************** SimplePWR Library *******************************
 * File Name          : SimplePWR.c
 * Author             : SimpleHAL
 * Version            : V1.0.0
 * Date               : 2025-12-22
 * Description        : Simple Power Management library implementation for CH32V003
 **********************************************************************************/
#include "SimplePWR.h"
#include "core_riscv.h"

// Private variables
static uint8_t pwr_initialized = 0;

/******************************************************************************/
/*                              Private Functions                             */
/******************************************************************************/

/**
 * @brief  Initialize PWR peripheral (called automatically)
 * @retval None
 */
static void PWR_Init(void)
{
    if (!pwr_initialized) {
        // Enable PWR clock
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        pwr_initialized = 1;
    }
}

/**
 * @brief  Select best AWU prescaler for desired timeout
 * @param  timeout_ms: Desired timeout in milliseconds
 * @param  prescaler: Pointer to store selected prescaler
 * @param  window: Pointer to store calculated window value
 * @retval None
 */
static void PWR_SelectAWUParams(uint32_t timeout_ms, uint32_t *prescaler, uint8_t *window)
{
    // Prescaler values in order
    const uint32_t prescalers[] = {
        1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 10240, 61440
    };
    
    const uint32_t prescaler_constants[] = {
        PWR_AWU_PRESCALER_1, PWR_AWU_PRESCALER_2, PWR_AWU_PRESCALER_4,
        PWR_AWU_PRESCALER_8, PWR_AWU_PRESCALER_16, PWR_AWU_PRESCALER_32,
        PWR_AWU_PRESCALER_64, PWR_AWU_PRESCALER_128, PWR_AWU_PRESCALER_256,
        PWR_AWU_PRESCALER_512, PWR_AWU_PRESCALER_1024, PWR_AWU_PRESCALER_2048,
        PWR_AWU_PRESCALER_4096, PWR_AWU_PRESCALER_10240, PWR_AWU_PRESCALER_61440
    };
    
    // Find the smallest prescaler that can achieve the timeout
    for (int i = 0; i < 15; i++) {
        uint32_t calc_window = PWR_AWU_CALC_WINDOW(prescalers[i], timeout_ms);
        
        if (calc_window <= PWR_AWU_MAX_WINDOW) {
            *prescaler = prescaler_constants[i];
            *window = (uint8_t)calc_window;
            return;
        }
    }
    
    // If timeout is too large, use maximum values
    *prescaler = PWR_AWU_PRESCALER_61440;
    *window = PWR_AWU_MAX_WINDOW;
}

/******************************************************************************/
/*                              Basic API Functions                           */
/******************************************************************************/

/**
 * @brief  Enter Sleep Mode (CPU stops, peripherals continue)
 */
void PWR_Sleep(void)
{
    PWR_EnterSleepMode(PWR_ENTRY_WFI);
}

/**
 * @brief  Enter Standby Mode with auto wake-up timeout
 */
void PWR_Standby(uint32_t timeout_ms)
{
    uint32_t prescaler;
    uint8_t window;
    
    // Initialize PWR if needed
    PWR_Init();
    
    // Calculate best AWU parameters
    PWR_SelectAWUParams(timeout_ms, &prescaler, &window);
    
    // Configure AWU
    PWR_ConfigureAWU(prescaler, window);
    
    // Enter standby mode
    PWR_EnterStandbyMode(PWR_ENTRY_WFI);
}

/**
 * @brief  Enter Standby Mode until external interrupt
 */
void PWR_StandbyUntilInterrupt(void)
{
    // Initialize PWR if needed
    PWR_Init();
    
    // Disable AWU to rely only on external wake-up
    PWR_AutoWakeUpCmd(DISABLE);
    
    // Enter standby mode
    PWR_EnterStandbyMode(PWR_ENTRY_WFI);
}

/******************************************************************************/
/*                              Advanced API Functions                        */
/******************************************************************************/

/**
 * @brief  Enter Sleep Mode with entry method selection
 */
void PWR_EnterSleepMode(uint8_t entry_method)
{
    // Clear SLEEPDEEP bit to enter Sleep mode (not Standby)
    NVIC->SCTLR &= ~(1 << 2);
    
    if (entry_method == PWR_ENTRY_WFE) {
        __WFE();  // Wait For Event
    } else {
        __WFI();  // Wait For Interrupt (default)
    }
}

/**
 * @brief  Enter Standby Mode with entry method selection
 */
void PWR_EnterStandbyMode(uint8_t entry_method)
{
    // Initialize PWR if needed
    PWR_Init();
    
    // Enter standby mode using HAL function
    PWR_EnterSTANDBYMode(entry_method);
    
    // This point should never be reached (system will reset on wake-up)
}

/**
 * @brief  Configure Auto Wake-Up timer
 */
void PWR_ConfigureAWU(uint32_t prescaler, uint8_t window)
{
    // Initialize PWR if needed
    PWR_Init();
    
    // Limit window to valid range
    if (window > PWR_AWU_MAX_WINDOW) {
        window = PWR_AWU_MAX_WINDOW;
    }
    
    // Configure AWU prescaler
    PWR_AWU_SetPrescaler(prescaler);
    
    // Configure AWU window value
    PWR_AWU_SetWindowValue(window);
    
    // Enable AWU
    PWR_AutoWakeUpCmd(ENABLE);
}

/**
 * @brief  Enable Power Voltage Detector
 */
void PWR_EnablePVD(uint32_t voltage_level)
{
    // Initialize PWR if needed
    PWR_Init();
    
    // Configure PVD level
    PWR_PVDLevelConfig(voltage_level);
    
    // Enable PVD
    PWR_PVDCmd(ENABLE);
}

/**
 * @brief  Disable Power Voltage Detector
 */
void PWR_DisablePVD(void)
{
    // Initialize PWR if needed
    PWR_Init();
    
    // Disable PVD
    PWR_PVDCmd(DISABLE);
}

/**
 * @brief  Get PVD status
 */
uint8_t PWR_GetPVDStatus(void)
{
    // Check PVDO flag
    if (PWR_GetFlagStatus(PWR_FLAG_PVDO) != RESET) {
        return 1;  // VDD is below threshold
    }
    return 0;  // VDD is above threshold
}

/**
 * @brief  Get actual AWU timeout value in milliseconds
 */
uint32_t PWR_GetAWUTimeout(uint32_t prescaler, uint8_t window)
{
    // Convert prescaler constant to actual value
    uint32_t prescaler_val;
    
    switch (prescaler) {
        case PWR_AWU_PRESCALER_1:     prescaler_val = 1; break;
        case PWR_AWU_PRESCALER_2:     prescaler_val = 2; break;
        case PWR_AWU_PRESCALER_4:     prescaler_val = 4; break;
        case PWR_AWU_PRESCALER_8:     prescaler_val = 8; break;
        case PWR_AWU_PRESCALER_16:    prescaler_val = 16; break;
        case PWR_AWU_PRESCALER_32:    prescaler_val = 32; break;
        case PWR_AWU_PRESCALER_64:    prescaler_val = 64; break;
        case PWR_AWU_PRESCALER_128:   prescaler_val = 128; break;
        case PWR_AWU_PRESCALER_256:   prescaler_val = 256; break;
        case PWR_AWU_PRESCALER_512:   prescaler_val = 512; break;
        case PWR_AWU_PRESCALER_1024:  prescaler_val = 1024; break;
        case PWR_AWU_PRESCALER_2048:  prescaler_val = 2048; break;
        case PWR_AWU_PRESCALER_4096:  prescaler_val = 4096; break;
        case PWR_AWU_PRESCALER_10240: prescaler_val = 10240; break;
        case PWR_AWU_PRESCALER_61440: prescaler_val = 61440; break;
        default: prescaler_val = 1; break;
    }
    
    return PWR_AWU_TIMEOUT_MS(prescaler_val, window);
}

/******************************************************************************/
/*                              Utility Functions                             */
/******************************************************************************/

/**
 * @brief  Check if last reset was caused by Standby wake-up
 */
uint8_t PWR_WasStandbyWakeup(void)
{
    // Check low-power reset flag in RCC (CH32V003 uses LPWRRST for standby wake-up)
    if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST) != RESET) {
        return 1;
    }
    return 0;
}

/**
 * @brief  Clear standby wake-up flag
 */
void PWR_ClearStandbyFlag(void)
{
    // Clear standby flag
    RCC_ClearFlag();
}

/**
 * @brief  Enable wake-up pin (PA0)
 */
void PWR_EnableWakeupPin(void)
{
    // Initialize PWR if needed
    PWR_Init();
    
    // Enable wake-up pin functionality
    // Note: On CH32V003, wake-up pin is typically PA0
    // Configure in PWR control register
    PWR->CTLR |= (1 << 8);  // Enable WKUP pin
}

/**
 * @brief  Disable wake-up pin
 */
void PWR_DisableWakeupPin(void)
{
    // Initialize PWR if needed
    PWR_Init();
    
    // Disable wake-up pin functionality
    PWR->CTLR &= ~(1 << 8);  // Disable WKUP pin
}

/**
 * @brief  Estimate standby mode current consumption
 */
uint32_t PWR_EstimateStandbyCurrent(uint8_t pvd_enabled, uint8_t awu_enabled)
{
    uint32_t current_ua = 2;  // Base standby current: ~2uA
    
    if (awu_enabled) {
        current_ua += 3;  // AWU adds ~3uA
    }
    
    if (pvd_enabled) {
        current_ua += 5;  // PVD adds ~5uA
    }
    
    return current_ua;
}

/**
 * @brief  Calculate battery life in hours
 */
uint32_t PWR_CalculateBatteryLife(uint16_t battery_mah, uint8_t active_time_percent, 
                                   uint16_t active_current_ma, uint16_t standby_current_ua)
{
    // Validate input
    if (active_time_percent > 100) {
        active_time_percent = 100;
    }
    
    // Calculate average current in mA
    // Formula: avg_current = (active_current * active_percent + standby_current * standby_percent) / 100
    uint32_t active_current_total = (uint32_t)active_current_ma * active_time_percent;
    uint32_t standby_current_total = ((uint32_t)standby_current_ua * (100 - active_time_percent)) / 1000;
    uint32_t avg_current_ma = (active_current_total + standby_current_total) / 100;
    
    // Prevent division by zero
    if (avg_current_ma == 0) {
        avg_current_ma = 1;
    }
    
    // Calculate battery life in hours
    // Formula: hours = battery_capacity / average_current
    uint32_t hours = (uint32_t)battery_mah / avg_current_ma;
    
    return hours;
}
