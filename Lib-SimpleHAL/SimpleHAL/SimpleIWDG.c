/********************************** SimpleIWDG Library *******************************
 * File Name          : SimpleIWDG.c
 * Author             : SimpleHAL
 * Version            : V1.0.0
 * Date               : 2025-12-21
 * Description        : Simple Independent Watchdog (IWDG) library for CH32V003
 **********************************************************************************/
#include "SimpleIWDG.h"

/******************************************************************************/
/*                              Private Functions                             */
/******************************************************************************/

/**
 * @brief  Select best prescaler for desired timeout
 * @param  timeout_ms: Desired timeout in milliseconds
 * @param  prescaler: Pointer to store selected prescaler
 * @param  reload: Pointer to store calculated reload value
 * @retval None
 */
static void IWDG_SelectPrescaler(uint16_t timeout_ms, uint8_t *prescaler, uint16_t *reload)
{
    const uint16_t prescaler_values[] = {4, 8, 16, 32, 64, 128, 256};
    const uint8_t prescaler_codes[] = {
        IWDG_PRESCALER_4,
        IWDG_PRESCALER_8,
        IWDG_PRESCALER_16,
        IWDG_PRESCALER_32,
        IWDG_PRESCALER_64,
        IWDG_PRESCALER_128,
        IWDG_PRESCALER_256
    };
    
    // Try each prescaler from smallest to largest
    for(uint8_t i = 0; i < 7; i++)
    {
        uint32_t calc_reload = IWDG_CALC_RELOAD(prescaler_values[i], timeout_ms);
        
        if(calc_reload <= IWDG_MAX_RELOAD)
        {
            *prescaler = prescaler_codes[i];
            *reload = (uint16_t)calc_reload;
            return;
        }
    }
    
    // If timeout is too large, use maximum prescaler and reload
    *prescaler = IWDG_PRESCALER_256;
    *reload = IWDG_MAX_RELOAD;
}

/******************************************************************************/
/*                              Basic API Functions                           */
/******************************************************************************/

/**
 * @brief  Initialize IWDG with timeout in milliseconds
 */
void IWDG_SimpleInit(uint16_t timeout_ms)
{
    uint8_t prescaler;
    uint16_t reload;
    
    // Select best prescaler and calculate reload value
    IWDG_SelectPrescaler(timeout_ms, &prescaler, &reload);
    
    // Initialize with calculated values
    IWDG_Init(prescaler, reload);
}

/**
 * @brief  Feed the watchdog (reload counter)
 */
void IWDG_Feed(void)
{
    IWDG_ReloadCounter();
}

/******************************************************************************/
/*                              Advanced API Functions                        */
/******************************************************************************/

/**
 * @brief  Initialize IWDG with custom prescaler and reload value
 */
void IWDG_Init(uint8_t prescaler, uint16_t reload)
{
    // Ensure reload value is within valid range
    if(reload > IWDG_MAX_RELOAD)
    {
        reload = IWDG_MAX_RELOAD;
    }
    
    // Enable write access to IWDG registers
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    
    // Set prescaler
    IWDG_SetPrescaler(prescaler);
    
    // Set reload value
    IWDG_SetReload(reload);
    
    // Wait for registers to be updated
    while(IWDG_GetFlagStatus(IWDG_FLAG_PVU) == SET);
    while(IWDG_GetFlagStatus(IWDG_FLAG_RVU) == SET);
    
    // Reload counter with new value
    IWDG_ReloadCounter();
    
    // Enable IWDG (cannot be disabled after this!)
    IWDG_Enable();
}

/**
 * @brief  Check if IWDG registers are being updated
 */
uint8_t IWDG_IsBusy(void)
{
    if(IWDG_GetFlagStatus(IWDG_FLAG_PVU) == SET ||
       IWDG_GetFlagStatus(IWDG_FLAG_RVU) == SET)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief  Get actual timeout value in milliseconds
 */
uint32_t IWDG_GetTimeout(uint8_t prescaler, uint16_t reload)
{
    // Convert prescaler code to actual value
    uint16_t prescaler_value;
    
    switch(prescaler)
    {
        case IWDG_PRESCALER_4:   prescaler_value = 4;   break;
        case IWDG_PRESCALER_8:   prescaler_value = 8;   break;
        case IWDG_PRESCALER_16:  prescaler_value = 16;  break;
        case IWDG_PRESCALER_32:  prescaler_value = 32;  break;
        case IWDG_PRESCALER_64:  prescaler_value = 64;  break;
        case IWDG_PRESCALER_128: prescaler_value = 128; break;
        case IWDG_PRESCALER_256: prescaler_value = 256; break;
        default: prescaler_value = 4; break;
    }
    
    return IWDG_TIMEOUT_MS(prescaler_value, reload);
}

/******************************************************************************/
/*                              Utility Functions                             */
/******************************************************************************/

/**
 * @brief  Check if last reset was caused by IWDG
 */
uint8_t IWDG_WasResetCause(void)
{
    if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief  Clear IWDG reset flag
 */
void IWDG_ClearResetFlag(void)
{
    RCC_ClearFlag();
}
