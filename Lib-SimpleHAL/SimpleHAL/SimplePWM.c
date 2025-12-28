/**
 * @file SimplePWM.c
 * @brief Simple PWM Library Implementation
 * @version 1.0
 * @date 2025-12-21
 */


#include "SimplePWM.h"


/* ========== Internal Structures ========== */

/**
 * @brief PWM channel configuration
 */
typedef struct {
    TIM_TypeDef* timer;
    uint8_t tim_channel;
    GPIO_TypeDef* gpio_port;
    uint16_t gpio_pin;
    uint32_t remap;
    uint8_t initialized;
} PWM_ChannelConfig_t;

/* ========== PWM Channel Mapping ========== */

static PWM_ChannelConfig_t pwm_channels[] = {
    // TIM1 Channels
    {TIM1, TIM_Channel_1, GPIOD, GPIO_Pin_2, 0, 0},  // PWM1_CH1 - PD2
    {TIM1, TIM_Channel_2, GPIOA, GPIO_Pin_1, 0, 0},  // PWM1_CH2 - PA1
    {TIM1, TIM_Channel_3, GPIOC, GPIO_Pin_3, 0, 0},  // PWM1_CH3 - PC3
    {TIM1, TIM_Channel_4, GPIOC, GPIO_Pin_4, 0, 0},  // PWM1_CH4 - PC4
    
    // TIM2 Channels
    {TIM2, TIM_Channel_1, GPIOD, GPIO_Pin_4, 0, 0},  // PWM2_CH1 - PD4
    {TIM2, TIM_Channel_2, GPIOD, GPIO_Pin_3, 0, 0},  // PWM2_CH2 - PD3
    {TIM2, TIM_Channel_3, GPIOC, GPIO_Pin_0, 0, 0},  // PWM2_CH3 - PC0
    {TIM2, TIM_Channel_4, GPIOD, GPIO_Pin_7, 0, 0}   // PWM2_CH4 - PD7
};

#define PWM_CHANNEL_COUNT (sizeof(pwm_channels) / sizeof(PWM_ChannelConfig_t))

/* ========== Internal Helper Functions ========== */

/**
 * @brief ดึง channel config
 */
static PWM_ChannelConfig_t* getChannelConfig(PWM_Channel channel) {
    if (channel >= PWM_CHANNEL_COUNT) return NULL;
    return &pwm_channels[channel];
}

/**
 * @brief เปิด GPIO และ Timer clock
 */
static void enablePeripheralClocks(PWM_ChannelConfig_t* config) {
    // เปิด GPIO clock
    if (config->gpio_port == GPIOA) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    } else if (config->gpio_port == GPIOC) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    } else if (config->gpio_port == GPIOD) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    }
    
    // เปิด Timer clock
    if (config->timer == TIM1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    } else if (config->timer == TIM2) {
        RCC_APB2PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    }
    
    // เปิด AFIO clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

/**
 * @brief ตั้งค่า GPIO pin สำหรับ PWM
 */
static void configureGPIO(PWM_ChannelConfig_t* config) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = config->gpio_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(config->gpio_port, &GPIO_InitStructure);
}

/**
 * @brief คำนวณ prescaler และ period
 */
static void calculatePWMParams(uint32_t frequency_hz, uint16_t* prescaler, uint16_t* period) {
    uint32_t ticks = SystemCoreClock / frequency_hz;
    
    if (ticks <= 65536) {
        *prescaler = 0;
        *period = ticks - 1;
    } else {
        *prescaler = (ticks / 65536);
        *period = (ticks / (*prescaler + 1)) - 1;
        
        while (*period > 65535) {
            (*prescaler)++;
            *period = (ticks / (*prescaler + 1)) - 1;
        }
    }
}

/**
 * @brief ตั้งค่า Timer base
 */
static void configureTimerBase(TIM_TypeDef* timer, uint16_t prescaler, uint16_t period) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    TIM_TimeBaseStructure.TIM_Period = period;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    
    TIM_TimeBaseInit(timer, &TIM_TimeBaseStructure);
}

/**
 * @brief ตั้งค่า PWM channel
 */
static void configurePWMChannel(PWM_ChannelConfig_t* config, uint16_t duty_value) {
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = duty_value;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
    // ตั้งค่าตาม channel
    switch (config->tim_channel) {
        case TIM_Channel_1:
            TIM_OC1Init(config->timer, &TIM_OCInitStructure);
            TIM_OC1PreloadConfig(config->timer, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_2:
            TIM_OC2Init(config->timer, &TIM_OCInitStructure);
            TIM_OC2PreloadConfig(config->timer, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_3:
            TIM_OC3Init(config->timer, &TIM_OCInitStructure);
            TIM_OC3PreloadConfig(config->timer, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_4:
            TIM_OC4Init(config->timer, &TIM_OCInitStructure);
            TIM_OC4PreloadConfig(config->timer, TIM_OCPreload_Enable);
            break;
    }
    
    TIM_ARRPreloadConfig(config->timer, ENABLE);
}

/* ========== Public Functions ========== */

/**
 * @brief เริ่มต้น PWM channel
 */
void PWM_Init(PWM_Channel channel, uint32_t frequency_hz) {
    PWM_InitRemap(channel, frequency_hz, PWM_REMAP_NONE);
}

/**
 * @brief เริ่มต้น PWM พร้อม remap
 */
void PWM_InitRemap(PWM_Channel channel, uint32_t frequency_hz, PWM_Remap remap) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    if (!config) return;
    
    // เปิด clocks
    enablePeripheralClocks(config);
    
    // ตั้งค่า pin remap ถ้าต้องการ
    if (remap != PWM_REMAP_NONE) {
        uint32_t remap_value = 0;
        
        if (config->timer == TIM1) {
            switch (remap) {
                case PWM_REMAP_PARTIAL1:
                    remap_value = GPIO_PartialRemap1_TIM1;
                    break;
                case PWM_REMAP_PARTIAL2:
                    remap_value = GPIO_PartialRemap2_TIM1;
                    break;
                case PWM_REMAP_FULL:
                    remap_value = GPIO_FullRemap_TIM1;
                    break;
                default:
                    break;
            }
        } else if (config->timer == TIM2) {
            switch (remap) {
                case PWM_REMAP_PARTIAL1:
                    remap_value = GPIO_PartialRemap1_TIM2;
                    break;
                case PWM_REMAP_PARTIAL2:
                    remap_value = GPIO_PartialRemap2_TIM2;
                    break;
                case PWM_REMAP_FULL:
                    remap_value = GPIO_FullRemap_TIM2;
                    break;
                default:
                    break;
            }
        }
        
        if (remap_value) {
            GPIO_PinRemapConfig(remap_value, ENABLE);
        }
    }
    
    // ตั้งค่า GPIO
    configureGPIO(config);
    
    // คำนวณและตั้งค่า timer
    uint16_t prescaler, period;
    calculatePWMParams(frequency_hz, &prescaler, &period);
    
    configureTimerBase(config->timer, prescaler, period);
    configurePWMChannel(config, 0);  // เริ่มที่ 0% duty
    
    // เปิด timer (แต่ยังไม่เปิด output)
    TIM_Cmd(config->timer, ENABLE);
    
    // สำหรับ TIM1 ต้องเปิด main output
    if (config->timer == TIM1) {
        TIM_CtrlPWMOutputs(TIM1, ENABLE);
    }
    
    config->initialized = 1;
}

/**
 * @brief ตั้งค่า duty cycle (%)
 */
void PWM_SetDutyCycle(PWM_Channel channel, uint8_t duty_percent) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    if (!config || !config->initialized) return;
    
    if (duty_percent > 100) duty_percent = 100;
    
    uint16_t period = config->timer->ATRLR;
    uint16_t duty_value = PWM_PERCENT_TO_RAW(duty_percent, period);
    
    PWM_SetDutyCycleRaw(channel, duty_value);
}

/**
 * @brief ตั้งค่า duty cycle (raw)
 */
void PWM_SetDutyCycleRaw(PWM_Channel channel, uint16_t duty_value) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    if (!config || !config->initialized) return;
    
    switch (config->tim_channel) {
        case TIM_Channel_1:
            TIM_SetCompare1(config->timer, duty_value);
            break;
        case TIM_Channel_2:
            TIM_SetCompare2(config->timer, duty_value);
            break;
        case TIM_Channel_3:
            TIM_SetCompare3(config->timer, duty_value);
            break;
        case TIM_Channel_4:
            TIM_SetCompare4(config->timer, duty_value);
            break;
    }
}

/**
 * @brief เปลี่ยนความถี่
 */
void PWM_SetFrequency(PWM_Channel channel, uint32_t frequency_hz) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    if (!config || !config->initialized) return;
    
    uint16_t prescaler, period;
    calculatePWMParams(frequency_hz, &prescaler, &period);
    
    configureTimerBase(config->timer, prescaler, period);
    
    // Reset duty cycle
    PWM_SetDutyCycleRaw(channel, 0);
}

/**
 * @brief เริ่ม PWM output
 */
void PWM_Start(PWM_Channel channel) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    if (!config || !config->initialized) return;
    
    TIM_CCxCmd(config->timer, config->tim_channel, TIM_CCx_Enable);
}

/**
 * @brief หยุด PWM output
 */
void PWM_Stop(PWM_Channel channel) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    if (!config || !config->initialized) return;
    
    TIM_CCxCmd(config->timer, config->tim_channel, TIM_CCx_Disable);
}

/**
 * @brief Arduino-style analogWrite
 */
void PWM_Write(PWM_Channel channel, uint8_t value) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    
    // Auto-init ถ้ายังไม่ได้ init
    if (!config->initialized) {
        PWM_Init(channel, 1000);  // Default 1kHz
        PWM_Start(channel);
    }
    
    uint8_t duty_percent = PWM_ARDUINO_TO_PERCENT(value);
    PWM_SetDutyCycle(channel, duty_percent);
}

/* ========== Advanced Functions ========== */

/**
 * @brief อ่านค่า period
 */
uint16_t PWM_GetPeriod(PWM_Channel channel) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    if (!config || !config->initialized) return 0;
    
    return config->timer->ATRLR;
}

/**
 * @brief อ่านค่า duty cycle (raw)
 */
uint16_t PWM_GetDutyCycleRaw(PWM_Channel channel) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    if (!config || !config->initialized) return 0;
    
    switch (config->tim_channel) {
        case TIM_Channel_1:
            return TIM_GetCapture1(config->timer);
        case TIM_Channel_2:
            return TIM_GetCapture2(config->timer);
        case TIM_Channel_3:
            return TIM_GetCapture3(config->timer);
        case TIM_Channel_4:
            return TIM_GetCapture4(config->timer);
    }
    
    return 0;
}

/**
 * @brief อ่านค่า duty cycle (%)
 */
uint8_t PWM_GetDutyCycle(PWM_Channel channel) {
    uint16_t duty_raw = PWM_GetDutyCycleRaw(channel);
    uint16_t period = PWM_GetPeriod(channel);
    
    if (period == 0) return 0;
    
    return PWM_RAW_TO_PERCENT(duty_raw, period);
}

/**
 * @brief ตั้งค่า PWM แบบละเอียด
 */
void PWM_AdvancedInit(PWM_Channel channel, uint16_t prescaler, 
                      uint16_t period, uint16_t duty_value) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    if (!config) return;
    
    enablePeripheralClocks(config);
    configureGPIO(config);
    configureTimerBase(config->timer, prescaler, period);
    configurePWMChannel(config, duty_value);
    
    TIM_Cmd(config->timer, ENABLE);
    
    if (config->timer == TIM1) {
        TIM_CtrlPWMOutputs(TIM1, ENABLE);
    }
    
    config->initialized = 1;
}

/**
 * @brief ตั้งค่า polarity
 */
void PWM_SetPolarity(PWM_Channel channel, uint8_t inverted) {
    PWM_ChannelConfig_t* config = getChannelConfig(channel);
    if (!config || !config->initialized) return;
    
    uint16_t polarity = inverted ? TIM_OCPolarity_Low : TIM_OCPolarity_High;
    
    switch (config->tim_channel) {
        case TIM_Channel_1:
            TIM_OC1PolarityConfig(config->timer, polarity);
            break;
        case TIM_Channel_2:
            TIM_OC2PolarityConfig(config->timer, polarity);
            break;
        case TIM_Channel_3:
            TIM_OC3PolarityConfig(config->timer, polarity);
            break;
        case TIM_Channel_4:
            TIM_OC4PolarityConfig(config->timer, polarity);
            break;
    }
}
