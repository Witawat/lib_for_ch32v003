/********************************** SimpleWWDG Library *******************************
 * File Name          : SimpleWWDG.c
 * Author             : SimpleHAL
 * Version            : V1.0.0
 * Date               : 2025-12-21
 * Description        : Simple Window Watchdog (WWDG) library for CH32V003
 **********************************************************************************/
#include "SimpleWWDG.h"

/******************************************************************************/
/*                              Private Variables                             */
/******************************************************************************/

static void (*WWDG_Callback)(void) = 0;

/******************************************************************************/
/*                              Basic API Functions                           */
/******************************************************************************/

/**
 * @brief  Initialize WWDG with simple configuration
 */
void WWDG_SimpleInit(uint8_t counter, uint8_t window)
{
    WWDG_Init(counter, window, WWDG_PRESCALER_8);
}

/**
 * @brief  Refresh WWDG counter
 */
void WWDG_Refresh(uint8_t counter)
{
    // Ensure counter is within valid range
    if(counter < WWDG_COUNTER_MIN)
    {
        counter = WWDG_COUNTER_MIN;
    }
    else if(counter > WWDG_COUNTER_MAX)
    {
        counter = WWDG_COUNTER_MAX;
    }
    
    WWDG_SetCounter(counter);
}

/******************************************************************************/
/*                              Advanced API Functions                        */
/******************************************************************************/

/**
 * @brief  Initialize WWDG with custom prescaler
 */
void WWDG_Init(uint8_t counter, uint8_t window, uint32_t prescaler)
{
    // Validate counter and window values
    if(counter < WWDG_COUNTER_MIN) counter = WWDG_COUNTER_MIN;
    if(counter > WWDG_COUNTER_MAX) counter = WWDG_COUNTER_MAX;
    if(window < WWDG_WINDOW_MIN) window = WWDG_WINDOW_MIN;
    if(window > WWDG_WINDOW_MAX) window = WWDG_WINDOW_MAX;
    
    // Enable WWDG clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
    
    // Set prescaler
    WWDG_SetPrescaler(prescaler);
    
    // Set window value
    WWDG_SetWindowValue(window);
    
    // Enable WWDG with initial counter value
    WWDG_Enable(counter);
}

/**
 * @brief  Initialize WWDG with Early Wakeup Interrupt
 */
void WWDG_InitWithInterrupt(uint8_t counter, uint8_t window, uint32_t prescaler)
{
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    
    // Validate counter and window values
    if(counter < WWDG_COUNTER_MIN) counter = WWDG_COUNTER_MIN;
    if(counter > WWDG_COUNTER_MAX) counter = WWDG_COUNTER_MAX;
    if(window < WWDG_WINDOW_MIN) window = WWDG_WINDOW_MIN;
    if(window > WWDG_WINDOW_MAX) window = WWDG_WINDOW_MAX;
    
    // Enable WWDG clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
    
    // Configure NVIC for WWDG interrupt
    NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // Set prescaler
    WWDG_SetPrescaler(prescaler);
    
    // Set window value
    WWDG_SetWindowValue(window);
    
    // Enable Early Wakeup Interrupt
    WWDG_EnableIT();
    
    // Enable WWDG with initial counter value
    WWDG_Enable(counter);
}

/**
 * @brief  Set callback function for Early Wakeup Interrupt
 */
void WWDG_SetCallback(void (*callback)(void))
{
    WWDG_Callback = callback;
}

/******************************************************************************/
/*                              Utility Functions                             */
/******************************************************************************/

/**
 * @brief  Calculate timeout in milliseconds
 */
uint32_t WWDG_CalcTimeout(uint32_t prescaler, uint8_t counter)
{
    // Convert prescaler constant to actual value
    uint32_t prescaler_value;
    
    switch(prescaler)
    {
        case WWDG_PRESCALER_1: prescaler_value = 1; break;
        case WWDG_PRESCALER_2: prescaler_value = 2; break;
        case WWDG_PRESCALER_4: prescaler_value = 4; break;
        case WWDG_PRESCALER_8: prescaler_value = 8; break;
        default: prescaler_value = 1; break;
    }
    
    return WWDG_TIMEOUT_MS(prescaler_value, counter);
}

/**
 * @brief  Get Early Wakeup Interrupt flag status
 */
uint8_t WWDG_GetInterruptFlag(void)
{
    if(WWDG_GetFlagStatus() == SET)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief  Clear Early Wakeup Interrupt flag
 */
void WWDG_ClearInterruptFlag(void)
{
    WWDG_ClearFlag();
}

/**
 * @brief  Disable WWDG (reset peripheral)
 */
void WWDG_Disable(void)
{
    WWDG_DeInit();
}

/******************************************************************************/
/*                              Interrupt Handler                             */
/******************************************************************************/

/**
 * @brief  WWDG Interrupt Handler Callback
 */
void WWDG_IRQHandler_Callback(void)
{
    if(WWDG_GetFlagStatus() == SET)
    {
        // Clear interrupt flag
        WWDG_ClearFlag();
        
        // Call user callback if set
        if(WWDG_Callback != 0)
        {
            WWDG_Callback();
        }
    }
}
