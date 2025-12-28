/**
 * @file SimpleTIM.c
 * @brief Simple Timer Library Implementation
 * @version 1.0
 * @date 2025-12-21
 */

#include "SimpleTIM.h"

/* ========== Internal Data ========== */

/**
 * @brief Timer peripheral mapping
 */
static TIM_TypeDef* const tim_peripherals[] = {
    TIM1,  // TIM_1
    TIM2   // TIM_2
};

/**
 * @brief Timer RCC peripherals
 */
static const uint32_t tim_rcc[] = {
    RCC_APB2Periph_TIM1,  // TIM_1
    RCC_APB1Periph_TIM2   // TIM_2
};

/**
 * @brief Timer IRQ channels
 */
static const uint8_t tim_irq[] = {
    TIM1_UP_IRQn,  // TIM_1
    TIM2_IRQn      // TIM_2
};

/**
 * @brief Interrupt callback functions
 */
static void (*tim_callbacks[2])(void) = {NULL, NULL};

/* ========== Internal Helper Functions ========== */

/**
 * @brief ดึง TIM peripheral
 */
static TIM_TypeDef* getTIM(TIM_Instance timer) {
    if (timer >= 2) return NULL;
    return tim_peripherals[timer];
}

/**
 * @brief เปิด timer clock
 */
static void enableTimerClock(TIM_Instance timer) {
    if (timer >= 2) return;
    
    if (timer == TIM_1) {
        RCC_APB2PeriphClockCmd(tim_rcc[timer], ENABLE);
    } else {
        RCC_APB2PeriphClockCmd(tim_rcc[timer], ENABLE);
    }
}

/**
 * @brief คำนวณ prescaler และ period จากความถี่
 */
static void calculateTimerParams(uint32_t frequency_hz, uint16_t* prescaler, uint16_t* period) {
    uint32_t ticks = SystemCoreClock / frequency_hz;
    
    if (ticks <= 65536) {
        // ไม่ต้องใช้ prescaler
        *prescaler = 0;
        *period = ticks - 1;
    } else {
        // ต้องใช้ prescaler
        *prescaler = (ticks / 65536);
        *period = (ticks / (*prescaler + 1)) - 1;
        
        // ปรับให้ได้ความถี่ใกล้เคียงที่สุด
        while (*period > 65535) {
            (*prescaler)++;
            *period = (ticks / (*prescaler + 1)) - 1;
        }
    }
}

/* ========== Public Functions ========== */

/**
 * @brief เริ่มต้น timer ด้วยความถี่
 */
void TIM_SimpleInit(TIM_Instance timer, uint32_t frequency_hz) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return;
    
    // เปิด clock
    enableTimerClock(timer);
    
    // คำนวณ prescaler และ period
    uint16_t prescaler, period;
    calculateTimerParams(frequency_hz, &prescaler, &period);
    
    // ตั้งค่า timer
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    TIM_TimeBaseStructure.TIM_Period = period;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
    
    // Clear update flag
    TIM_ClearFlag(TIMx, TIM_FLAG_Update);
}

/**
 * @brief เริ่มการนับ
 */
void TIM_Start(TIM_Instance timer) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return;
    
    TIM_Cmd(TIMx, ENABLE);
}

/**
 * @brief หยุดการนับ
 */
void TIM_Stop(TIM_Instance timer) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return;
    
    TIM_Cmd(TIMx, DISABLE);
}

/**
 * @brief เปลี่ยนความถี่
 */
void TIM_SetFrequency(TIM_Instance timer, uint32_t frequency_hz) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return;
    
    // หยุด timer
    TIM_Cmd(TIMx, DISABLE);
    
    // คำนวณค่าใหม่
    uint16_t prescaler, period;
    calculateTimerParams(frequency_hz, &prescaler, &period);
    
    // ตั้งค่าใหม่
    TIM_PrescalerConfig(TIMx, prescaler, TIM_PSCReloadMode_Immediate);
    TIM_SetAutoreload(TIMx, period);
    
    // Generate update event
    TIM_GenerateEvent(TIMx, TIM_EventSource_Update);
    
    // Clear flag
    TIM_ClearFlag(TIMx, TIM_FLAG_Update);
}

/**
 * @brief อ่านค่า counter
 */
uint16_t Simple_TIM_GetCounter(TIM_Instance timer) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return 0;
    
    return TIMx->CNT;
}

/**
 * @brief ตั้งค่า counter
 */
void Simple_TIM_SetCounter(TIM_Instance timer, uint16_t value) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return;
    
    TIMx->CNT = value;
}

/**
 * @brief อ่านค่า period
 */
uint16_t TIM_GetPeriod(TIM_Instance timer) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return 0;
    
    return TIMx->ATRLR;
}

/**
 * @brief ตั้งค่า interrupt callback
 */
void TIM_AttachInterrupt(TIM_Instance timer, void (*callback)(void)) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx || !callback) return;
    
    // เก็บ callback
    tim_callbacks[timer] = callback;
    
    // เปิด update interrupt
    TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
    
    // ตั้งค่า NVIC
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = tim_irq[timer];
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief ยกเลิก interrupt
 */
void TIM_DetachInterrupt(TIM_Instance timer) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return;
    
    // ปิด interrupt
    TIM_ITConfig(TIMx, TIM_IT_Update, DISABLE);
    
    // ลบ callback
    tim_callbacks[timer] = NULL;
    
    // ปิด NVIC
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = tim_irq[timer];
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/* ========== Advanced Functions ========== */

/**
 * @brief ตั้งค่า timer แบบละเอียด
 */
void TIM_AdvancedInit(TIM_Instance timer, uint16_t prescaler, uint16_t period, TIM_Mode mode) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return;
    
    enableTimerClock(timer);
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    TIM_TimeBaseStructure.TIM_Period = period;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = (mode == TIM_MODE_UP) ? 
                                             TIM_CounterMode_Up : TIM_CounterMode_Down;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIMx, TIM_FLAG_Update);
}

/**
 * @brief ตั้งค่า prescaler
 */
void TIM_SetPrescaler(TIM_Instance timer, uint16_t prescaler) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return;
    
    TIM_PrescalerConfig(TIMx, prescaler, TIM_PSCReloadMode_Immediate);
}

/**
 * @brief อ่านค่า prescaler
 */
uint16_t Simple_TIM_GetPrescaler(TIM_Instance timer) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return 0;
    
    return TIMx->PSC;
}

/**
 * @brief ตั้งค่าโหมดการนับ
 */
void TIM_SetMode(TIM_Instance timer, TIM_Mode mode) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return;
    
    uint16_t counter_mode = (mode == TIM_MODE_UP) ? 
                            TIM_CounterMode_Up : TIM_CounterMode_Down;
    TIM_CounterModeConfig(TIMx, counter_mode);
}

/**
 * @brief Generate update event
 */
void TIM_GenerateUpdate(TIM_Instance timer) {
    TIM_TypeDef* TIMx = getTIM(timer);
    if (!TIMx) return;
    
    TIM_GenerateEvent(TIMx, TIM_EventSource_Update);
    TIM_ClearFlag(TIMx, TIM_FLAG_Update);
}

/* ========== Interrupt Handlers ========== */

/**
 * @brief TIM1 update interrupt handler
 */
void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void) {
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
        // เรียก callback
        if (tim_callbacks[TIM_1]) {
            tim_callbacks[TIM_1]();
        }
        
        // Clear flag
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
}

/**
 * @brief TIM2 interrupt handler
 */
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        // เรียก callback
        if (tim_callbacks[TIM_2]) {
            tim_callbacks[TIM_2]();
        }
        
        // Clear flag
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
