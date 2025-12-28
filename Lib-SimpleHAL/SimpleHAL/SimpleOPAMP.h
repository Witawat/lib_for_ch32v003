/********************************** SimpleOPAMP Library *******************************
 * File Name          : SimpleOPAMP.h
 * Author             : SimpleHAL
 * Version            : V1.0.0
 * Date               : 2025-12-21
 * Description        : Simple OPAMP (Operational Amplifier) library for CH32V003
 *                      Easy-to-use Arduino-like API
 * 
 * @details
 * Library นี้ห่อหุ้ม OPAMP peripheral ให้ใช้งานง่าย รองรับการใช้งานหลายรูปแบบ:
 * - Voltage Follower (Buffer)
 * - Non-Inverting Amplifier
 * - Inverting Amplifier
 * - Comparator Mode
 * 
 * **คุณสมบัติ:**
 * - API แบบ Arduino-style ใช้งานง่าย
 * - รองรับ 2 input channels (positive และ negative)
 * - สามารถเชื่อมต่อกับ ADC เพื่ออ่านค่า output
 * - รองรับการคำนวณ gain อัตโนมัติ
 * 
 * **OPAMP Channels ของ CH32V003:**
 * - Positive Input: CHP0, CHP1
 * - Negative Input: CHN0, CHN1
 * - Output: สามารถอ่านผ่าน ADC
 * 
 * @example
 * // ตัวอย่างการใช้งาน Voltage Follower
 * #include "SimpleOPAMP.h"
 * 
 * int main(void) {
 *     // เริ่มต้น OPAMP เป็น voltage follower
 *     OPAMP_SimpleInit(OPAMP_MODE_VOLTAGE_FOLLOWER);
 *     
 *     // เปิดใช้งาน OPAMP
 *     OPAMP_Enable();
 *     
 *     while(1) {
 *         // OPAMP ทำงานเป็น buffer
 *     }
 * }
 * 
 * @note CH32V003 มี OPAMP 1 ตัว ที่สามารถเลือก input channels ได้
 **********************************************************************************/
#ifndef __SIMPLE_OPAMP_H
#define __SIMPLE_OPAMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v00x_opa.h"
#include <stdint.h>

/******************************************************************************/
/*                              Mode Definitions                              */
/******************************************************************************/

/**
 * @brief OPAMP Operating Modes
 * @details โหมดการทำงานของ OPAMP
 */
typedef enum {
    OPAMP_MODE_VOLTAGE_FOLLOWER = 0,  /**< Voltage Follower (Buffer) - Gain = 1 */
    OPAMP_MODE_NON_INVERTING,         /**< Non-Inverting Amplifier - Gain > 1 */
    OPAMP_MODE_INVERTING,             /**< Inverting Amplifier - Gain < 0 */
    OPAMP_MODE_COMPARATOR             /**< Comparator Mode */
} OPAMP_Mode;

/**
 * @brief OPAMP Positive Input Channel
 * @details ช่อง input ด้าน positive
 */
typedef enum {
    OPAMP_CHP0 = CHP0,  /**< Positive Channel 0 */
    OPAMP_CHP1 = CHP1   /**< Positive Channel 1 */
} OPAMP_Channel_Positive;

/**
 * @brief OPAMP Negative Input Channel
 * @details ช่อง input ด้าน negative
 */
typedef enum {
    OPAMP_CHN0 = CHN0,  /**< Negative Channel 0 */
    OPAMP_CHN1 = CHN1   /**< Negative Channel 1 */
} OPAMP_Channel_Negative;

/**
 * @brief OPAMP Gain Presets
 * @details ค่า gain ที่กำหนดไว้ล่วงหน้า (สำหรับการคำนวณ)
 * @note ค่า gain จริงขึ้นอยู่กับความต้านทานภายนอก
 */
typedef enum {
    OPAMP_GAIN_1X  = 1,   /**< Unity Gain (Voltage Follower) */
    OPAMP_GAIN_2X  = 2,   /**< Gain = 2 */
    OPAMP_GAIN_4X  = 4,   /**< Gain = 4 */
    OPAMP_GAIN_8X  = 8,   /**< Gain = 8 */
    OPAMP_GAIN_16X = 16   /**< Gain = 16 */
} OPAMP_Gain;

/******************************************************************************/
/*                              Basic API Functions                           */
/******************************************************************************/

/**
 * @brief  Initialize OPAMP in specified mode (Simple API)
 * @param  mode: Operating mode (OPAMP_MODE_xxx)
 * @note   This function sets up OPAMP with default channels
 * @note   For voltage follower: CHP0 input, CHN0 feedback
 * @retval None
 * 
 * Example:
 *   OPAMP_SimpleInit(OPAMP_MODE_VOLTAGE_FOLLOWER);
 *   OPAMP_Enable();
 */
void OPAMP_SimpleInit(OPAMP_Mode mode);

/**
 * @brief  Enable OPAMP
 * @note   Must call OPAMP_SimpleInit() or OPAMP_Init() first
 * @retval None
 * 
 * Example:
 *   OPAMP_Enable();
 */
void OPAMP_Enable(void);

/**
 * @brief  Disable OPAMP
 * @retval None
 * 
 * Example:
 *   OPAMP_Disable();
 */
void OPAMP_Disable(void);

/**
 * @brief  Change OPAMP operating mode
 * @param  mode: New operating mode
 * @note   OPAMP will be temporarily disabled during mode change
 * @retval None
 * 
 * Example:
 *   OPAMP_SetMode(OPAMP_MODE_NON_INVERTING);
 */
void OPAMP_SetMode(OPAMP_Mode mode);

/******************************************************************************/
/*                              Advanced API Functions                        */
/******************************************************************************/

/**
 * @brief  Initialize OPAMP with custom channels
 * @param  pos_channel: Positive input channel (OPAMP_CHP0 or OPAMP_CHP1)
 * @param  neg_channel: Negative input channel (OPAMP_CHN0 or OPAMP_CHN1)
 * @retval None
 * 
 * Example:
 *   OPAMP_Init(OPAMP_CHP0, OPAMP_CHN0);
 *   OPAMP_Enable();
 */
void OPAMP_Init(OPAMP_Channel_Positive pos_channel, OPAMP_Channel_Negative neg_channel);

/**
 * @brief  Configure OPAMP as Voltage Follower (Buffer)
 * @param  pos_channel: Positive input channel
 * @note   Negative input should be connected to output externally
 * @retval None
 * 
 * Circuit:
 *   Input -> CHP0 (positive input)
 *   Output -> CHN0 (negative input) - external connection
 * 
 * Example:
 *   OPAMP_ConfigVoltageFollower(OPAMP_CHP0);
 *   OPAMP_Enable();
 */
void OPAMP_ConfigVoltageFollower(OPAMP_Channel_Positive pos_channel);

/**
 * @brief  Configure OPAMP as Non-Inverting Amplifier
 * @param  pos_channel: Positive input channel
 * @param  neg_channel: Negative input channel (feedback)
 * @note   External resistors R1 and R2 required for gain setting
 * @note   Gain = 1 + (R2/R1)
 * @retval None
 * 
 * Circuit:
 *   Input -> CHP0 (positive input)
 *   Output -> R2 -> CHN0 (negative input)
 *   CHN0 -> R1 -> GND
 * 
 * Example:
 *   // For gain = 2: R1 = R2 (e.g., both 10kΩ)
 *   OPAMP_ConfigNonInverting(OPAMP_CHP0, OPAMP_CHN0);
 *   OPAMP_Enable();
 */
void OPAMP_ConfigNonInverting(OPAMP_Channel_Positive pos_channel, 
                              OPAMP_Channel_Negative neg_channel);

/**
 * @brief  Configure OPAMP as Inverting Amplifier
 * @param  pos_channel: Positive input channel (usually tied to reference)
 * @param  neg_channel: Negative input channel (signal input)
 * @note   External resistors R1 and R2 required for gain setting
 * @note   Gain = -(R2/R1)
 * @retval None
 * 
 * Circuit:
 *   Reference -> CHP0 (positive input, usually GND or Vref/2)
 *   Input -> R1 -> CHN0 (negative input)
 *   Output -> R2 -> CHN0 (feedback)
 * 
 * Example:
 *   // For gain = -2: R2 = 2*R1 (e.g., R1=10kΩ, R2=20kΩ)
 *   OPAMP_ConfigInverting(OPAMP_CHP0, OPAMP_CHN0);
 *   OPAMP_Enable();
 */
void OPAMP_ConfigInverting(OPAMP_Channel_Positive pos_channel, 
                           OPAMP_Channel_Negative neg_channel);

/**
 * @brief  Configure OPAMP as Comparator
 * @param  pos_channel: Positive input channel (signal)
 * @param  neg_channel: Negative input channel (reference/threshold)
 * @note   Output will be HIGH when V+ > V-, LOW otherwise
 * @retval None
 * 
 * Circuit:
 *   Signal -> CHP0 (positive input)
 *   Threshold -> CHN0 (negative input)
 *   Output -> Digital output (HIGH/LOW)
 * 
 * Example:
 *   // Compare input signal with threshold
 *   OPAMP_ConfigComparator(OPAMP_CHP0, OPAMP_CHN0);
 *   OPAMP_Enable();
 */
void OPAMP_ConfigComparator(OPAMP_Channel_Positive pos_channel, 
                            OPAMP_Channel_Negative neg_channel);

/**
 * @brief  Set OPAMP input channels
 * @param  pos_channel: Positive input channel
 * @param  neg_channel: Negative input channel
 * @retval None
 * 
 * Example:
 *   OPAMP_SetChannels(OPAMP_CHP1, OPAMP_CHN1);
 */
void OPAMP_SetChannels(OPAMP_Channel_Positive pos_channel, 
                       OPAMP_Channel_Negative neg_channel);

/******************************************************************************/
/*                              Utility Functions                             */
/******************************************************************************/

/**
 * @brief  Calculate gain for non-inverting amplifier
 * @param  r1: Resistor to ground (Ω)
 * @param  r2: Feedback resistor (Ω)
 * @retval Calculated gain value
 * @note   Formula: Gain = 1 + (R2/R1)
 * 
 * Example:
 *   float gain = OPAMP_CalculateGainNonInv(10000, 10000);  // Returns 2.0
 */
float OPAMP_CalculateGainNonInv(uint32_t r1, uint32_t r2);

/**
 * @brief  Calculate gain for inverting amplifier
 * @param  r1: Input resistor (Ω)
 * @param  r2: Feedback resistor (Ω)
 * @retval Calculated gain value (negative)
 * @note   Formula: Gain = -(R2/R1)
 * 
 * Example:
 *   float gain = OPAMP_CalculateGainInv(10000, 20000);  // Returns -2.0
 */
float OPAMP_CalculateGainInv(uint32_t r1, uint32_t r2);

/**
 * @brief  Calculate R2 value for desired non-inverting gain
 * @param  r1: Resistor to ground (Ω)
 * @param  desired_gain: Desired gain (must be >= 1)
 * @retval Required R2 value (Ω)
 * @note   Formula: R2 = R1 * (Gain - 1)
 * 
 * Example:
 *   // For gain = 5 with R1 = 10kΩ
 *   uint32_t r2 = OPAMP_CalculateR2NonInv(10000, 5);  // Returns 40000
 */
uint32_t OPAMP_CalculateR2NonInv(uint32_t r1, float desired_gain);

/**
 * @brief  Calculate R2 value for desired inverting gain
 * @param  r1: Input resistor (Ω)
 * @param  desired_gain: Desired gain magnitude (positive value)
 * @retval Required R2 value (Ω)
 * @note   Formula: R2 = R1 * |Gain|
 * 
 * Example:
 *   // For gain = -3 with R1 = 10kΩ
 *   uint32_t r2 = OPAMP_CalculateR2Inv(10000, 3);  // Returns 30000
 */
uint32_t OPAMP_CalculateR2Inv(uint32_t r1, float desired_gain);

/**
 * @brief  Check if OPAMP is enabled
 * @retval 1: OPAMP is enabled
 *         0: OPAMP is disabled
 * 
 * Example:
 *   if(OPAMP_IsEnabled()) {
 *       // OPAMP is running
 *   }
 */
uint8_t OPAMP_IsEnabled(void);

/**
 * @brief  Get current OPAMP configuration
 * @param  pos_channel: Pointer to store positive channel
 * @param  neg_channel: Pointer to store negative channel
 * @retval None
 * 
 * Example:
 *   OPAMP_Channel_Positive pos;
 *   OPAMP_Channel_Negative neg;
 *   OPAMP_GetConfig(&pos, &neg);
 */
void OPAMP_GetConfig(OPAMP_Channel_Positive* pos_channel, 
                     OPAMP_Channel_Negative* neg_channel);

/******************************************************************************/
/*                              Pin Mapping Reference                         */
/******************************************************************************/
/*
 * CH32V003 OPAMP Pin Mapping:
 * 
 * Positive Inputs:
 *   CHP0 - Positive Channel 0 (check datasheet for pin)
 *   CHP1 - Positive Channel 1 (check datasheet for pin)
 * 
 * Negative Inputs:
 *   CHN0 - Negative Channel 0 (check datasheet for pin)
 *   CHN1 - Negative Channel 1 (check datasheet for pin)
 * 
 * Output:
 *   OPAMP output can be read via ADC
 *   Can also be routed to GPIO or TIM2
 * 
 * Note: Refer to CH32V003 datasheet for exact pin assignments
 */

/******************************************************************************/
/*                              Configuration Examples                        */
/******************************************************************************/
/*
 * 1. Voltage Follower (Buffer):
 *    - Gain = 1
 *    - Input impedance: Very high
 *    - Output impedance: Very low
 *    - Use: Buffer signals, impedance matching
 * 
 * 2. Non-Inverting Amplifier:
 *    - Gain = 1 + (R2/R1)
 *    - Input impedance: Very high
 *    - Phase: Same as input
 *    - Use: Amplify small signals
 * 
 * 3. Inverting Amplifier:
 *    - Gain = -(R2/R1)
 *    - Input impedance: R1
 *    - Phase: Inverted from input
 *    - Use: Amplify and invert signals
 * 
 * 4. Comparator:
 *    - Output: Digital (HIGH/LOW)
 *    - Use: Threshold detection, zero-crossing detection
 */

#ifdef __cplusplus
}
#endif

#endif /* __SIMPLE_OPAMP_H */
