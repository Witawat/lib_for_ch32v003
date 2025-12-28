/********************************** SimpleOPAMP Library *******************************
 * File Name          : SimpleOPAMP.c
 * Author             : SimpleHAL
 * Version            : V1.0.0
 * Date               : 2025-12-21
 * Description        : Simple OPAMP (Operational Amplifier) library implementation
 **********************************************************************************/
#include "SimpleOPAMP.h"

// Internal state tracking
static OPAMP_Mode current_mode = OPAMP_MODE_VOLTAGE_FOLLOWER;
static uint8_t opamp_enabled = 0;

/******************************************************************************/
/*                              Basic API Functions                           */
/******************************************************************************/

/**
 * @brief  Initialize OPAMP in specified mode (Simple API)
 */
void OPAMP_SimpleInit(OPAMP_Mode mode) {
    current_mode = mode;
    
    switch(mode) {
        case OPAMP_MODE_VOLTAGE_FOLLOWER:
            // Voltage follower: positive input CHP0, negative feedback CHN0
            OPAMP_ConfigVoltageFollower(OPAMP_CHP0);
            break;
            
        case OPAMP_MODE_NON_INVERTING:
            // Non-inverting: positive input CHP0, negative feedback CHN0
            OPAMP_ConfigNonInverting(OPAMP_CHP0, OPAMP_CHN0);
            break;
            
        case OPAMP_MODE_INVERTING:
            // Inverting: positive reference CHP0, negative input CHN0
            OPAMP_ConfigInverting(OPAMP_CHP0, OPAMP_CHN0);
            break;
            
        case OPAMP_MODE_COMPARATOR:
            // Comparator: positive signal CHP0, negative threshold CHN0
            OPAMP_ConfigComparator(OPAMP_CHP0, OPAMP_CHN0);
            break;
            
        default:
            // Default to voltage follower
            OPAMP_ConfigVoltageFollower(OPAMP_CHP0);
            break;
    }
}

/**
 * @brief  Enable OPAMP
 */
void OPAMP_Enable(void) {
    OPA_Cmd(ENABLE);
    opamp_enabled = 1;
}

/**
 * @brief  Disable OPAMP
 */
void OPAMP_Disable(void) {
    OPA_Cmd(DISABLE);
    opamp_enabled = 0;
}

/**
 * @brief  Change OPAMP operating mode
 */
void OPAMP_SetMode(OPAMP_Mode mode) {
    // Disable OPAMP during mode change
    uint8_t was_enabled = opamp_enabled;
    if(was_enabled) {
        OPAMP_Disable();
    }
    
    // Reinitialize with new mode
    OPAMP_SimpleInit(mode);
    
    // Re-enable if it was enabled before
    if(was_enabled) {
        OPAMP_Enable();
    }
}

/******************************************************************************/
/*                              Advanced API Functions                        */
/******************************************************************************/

/**
 * @brief  Initialize OPAMP with custom channels
 */
void OPAMP_Init(OPAMP_Channel_Positive pos_channel, OPAMP_Channel_Negative neg_channel) {
    OPA_InitTypeDef OPA_InitStructure;
    
    // Configure channels
    OPA_InitStructure.PSEL = (OPA_PSEL_TypeDef)pos_channel;
    OPA_InitStructure.NSEL = (OPA_NSEL_TypeDef)neg_channel;
    
    // Initialize OPAMP
    OPA_Init(&OPA_InitStructure);
}

/**
 * @brief  Configure OPAMP as Voltage Follower (Buffer)
 */
void OPAMP_ConfigVoltageFollower(OPAMP_Channel_Positive pos_channel) {
    // For voltage follower, output is connected to negative input externally
    // We set both channels, but negative channel will receive feedback from output
    OPAMP_Init(pos_channel, OPAMP_CHN0);
    current_mode = OPAMP_MODE_VOLTAGE_FOLLOWER;
}

/**
 * @brief  Configure OPAMP as Non-Inverting Amplifier
 */
void OPAMP_ConfigNonInverting(OPAMP_Channel_Positive pos_channel, 
                              OPAMP_Channel_Negative neg_channel) {
    // Non-inverting: signal on positive input, feedback on negative input
    OPAMP_Init(pos_channel, neg_channel);
    current_mode = OPAMP_MODE_NON_INVERTING;
}

/**
 * @brief  Configure OPAMP as Inverting Amplifier
 */
void OPAMP_ConfigInverting(OPAMP_Channel_Positive pos_channel, 
                           OPAMP_Channel_Negative neg_channel) {
    // Inverting: reference on positive input, signal on negative input
    OPAMP_Init(pos_channel, neg_channel);
    current_mode = OPAMP_MODE_INVERTING;
}

/**
 * @brief  Configure OPAMP as Comparator
 */
void OPAMP_ConfigComparator(OPAMP_Channel_Positive pos_channel, 
                            OPAMP_Channel_Negative neg_channel) {
    // Comparator: signal on positive, threshold on negative
    OPAMP_Init(pos_channel, neg_channel);
    current_mode = OPAMP_MODE_COMPARATOR;
}

/**
 * @brief  Set OPAMP input channels
 */
void OPAMP_SetChannels(OPAMP_Channel_Positive pos_channel, 
                       OPAMP_Channel_Negative neg_channel) {
    OPAMP_Init(pos_channel, neg_channel);
}

/******************************************************************************/
/*                              Utility Functions                             */
/******************************************************************************/

/**
 * @brief  Calculate gain for non-inverting amplifier
 * @note   Formula: Gain = 1 + (R2/R1)
 */
float OPAMP_CalculateGainNonInv(uint32_t r1, uint32_t r2) {
    if(r1 == 0) return 1.0f;  // Prevent division by zero
    return 1.0f + ((float)r2 / (float)r1);
}

/**
 * @brief  Calculate gain for inverting amplifier
 * @note   Formula: Gain = -(R2/R1)
 */
float OPAMP_CalculateGainInv(uint32_t r1, uint32_t r2) {
    if(r1 == 0) return 0.0f;  // Prevent division by zero
    return -((float)r2 / (float)r1);
}

/**
 * @brief  Calculate R2 value for desired non-inverting gain
 * @note   Formula: R2 = R1 * (Gain - 1)
 */
uint32_t OPAMP_CalculateR2NonInv(uint32_t r1, float desired_gain) {
    if(desired_gain < 1.0f) desired_gain = 1.0f;  // Minimum gain is 1
    return (uint32_t)(r1 * (desired_gain - 1.0f));
}

/**
 * @brief  Calculate R2 value for desired inverting gain
 * @note   Formula: R2 = R1 * |Gain|
 */
uint32_t OPAMP_CalculateR2Inv(uint32_t r1, float desired_gain) {
    if(desired_gain < 0.0f) desired_gain = -desired_gain;  // Use absolute value
    return (uint32_t)(r1 * desired_gain);
}

/**
 * @brief  Check if OPAMP is enabled
 */
uint8_t OPAMP_IsEnabled(void) {
    return opamp_enabled;
}

/**
 * @brief  Get current OPAMP configuration
 */
void OPAMP_GetConfig(OPAMP_Channel_Positive* pos_channel, 
                     OPAMP_Channel_Negative* neg_channel) {
    // Read current configuration from EXTEN register
    uint32_t exten_ctr = EXTEN->EXTEN_CTR;
    
    // Extract PSEL (bit 18)
    *pos_channel = (OPAMP_Channel_Positive)((exten_ctr >> 18) & 0x01);
    
    // Extract NSEL (bit 17)
    *neg_channel = (OPAMP_Channel_Negative)((exten_ctr >> 17) & 0x01);
}
