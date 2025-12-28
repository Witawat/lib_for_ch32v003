/**
 * @file SimpleGPIO.c
 * @brief Simple GPIO Library Implementation
 * @version 1.3
 * @date 2025-12-22
 */

#include "SimpleGPIO.h"
#include "SimpleADC.h"
#include "SimplePWM.h"
#include "SimpleDelay.h"

/* ========== Internal Structures ========== */

/**
 * @brief Pin mapping structure
 */
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t pin_source;
    uint8_t port_source;
} PinMap_t;

/* ========== Pin Mapping Table ========== */

/**
 * @brief แมป CH32V003 pin numbers กับ physical pins
 */
static const PinMap_t pin_map[] = {
    // GPIOA pins (PA1-PA2)
    {GPIOA, GPIO_Pin_1, GPIO_PinSource1, GPIO_PortSourceGPIOA},  // PA1 (0)
    {GPIOA, GPIO_Pin_2, GPIO_PinSource2, GPIO_PortSourceGPIOA},  // PA2 (1)
    
    // Reserved (2-9)
    {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
    {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
    
    // GPIOC pins (PC0-PC7)
    {GPIOC, GPIO_Pin_0, GPIO_PinSource0, GPIO_PortSourceGPIOC},  // PC0 (10)
    {GPIOC, GPIO_Pin_1, GPIO_PinSource1, GPIO_PortSourceGPIOC},  // PC1 (11)
    {GPIOC, GPIO_Pin_2, GPIO_PinSource2, GPIO_PortSourceGPIOC},  // PC2 (12)
    {GPIOC, GPIO_Pin_3, GPIO_PinSource3, GPIO_PortSourceGPIOC},  // PC3 (13)
    {GPIOC, GPIO_Pin_4, GPIO_PinSource4, GPIO_PortSourceGPIOC},  // PC4 (14)
    {GPIOC, GPIO_Pin_5, GPIO_PinSource5, GPIO_PortSourceGPIOC},  // PC5 (15)
    {GPIOC, GPIO_Pin_6, GPIO_PinSource6, GPIO_PortSourceGPIOC},  // PC6 (16)
    {GPIOC, GPIO_Pin_7, GPIO_PinSource7, GPIO_PortSourceGPIOC},  // PC7 (17)
    
    // Reserved (18-19)
    {0, 0, 0, 0}, {0, 0, 0, 0},
    
    // GPIOD pins (PD2-PD7)
    {GPIOD, GPIO_Pin_2, GPIO_PinSource2, GPIO_PortSourceGPIOD},  // PD2 (20)
    {GPIOD, GPIO_Pin_3, GPIO_PinSource3, GPIO_PortSourceGPIOD},  // PD3 (21)
    {GPIOD, GPIO_Pin_4, GPIO_PinSource4, GPIO_PortSourceGPIOD},  // PD4 (22)
    {GPIOD, GPIO_Pin_5, GPIO_PinSource5, GPIO_PortSourceGPIOD},  // PD5 (23)
    {GPIOD, GPIO_Pin_6, GPIO_PinSource6, GPIO_PortSourceGPIOD},  // PD6 (24)
    {GPIOD, GPIO_Pin_7, GPIO_PinSource7, GPIO_PortSourceGPIOD}   // PD7 (25)
};

#define PIN_MAP_SIZE (sizeof(pin_map) / sizeof(PinMap_t))

/* ========== Interrupt Callback Storage ========== */

/**
 * @brief Interrupt callback functions
 * @note สูงสุด 8 EXTI lines (0-7)
 */
static void (*exti_callbacks[8])(void) = {0};

/* ========== Analog Function State ========== */

/**
 * @brief ADC initialization state
 */
static uint8_t adc_initialized = 0;

/* ========== Internal Helper Functions ========== */

/**
 * @brief ดึงข้อมูล pin mapping
 */
static const PinMap_t* getPinMap(uint8_t pin) {
    if (pin >= PIN_MAP_SIZE) return NULL;
    if (pin_map[pin].port == 0) return NULL;
    return &pin_map[pin];
}

/**
 * @brief เปิดใช้งาน GPIO clock
 */
static void enableGPIOClock(GPIO_TypeDef* port) {
    if (port == GPIOA) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    } else if (port == GPIOC) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    } else if (port == GPIOD) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    }
}

/* ========== Public Functions ========== */

/**
 * @brief ตั้งค่าโหมดของ GPIO pin
 */
void pinMode(uint8_t pin, GPIO_PinMode mode) {
    const PinMap_t* map = getPinMap(pin);
    if (!map) return;
    
    // เปิด clock
    enableGPIOClock(map->port);
    
    // ตั้งค่า GPIO
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = map->pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    
    switch (mode) {
        case PIN_MODE_INPUT:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            break;
            
        case PIN_MODE_OUTPUT:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
            break;
            
        case PIN_MODE_INPUT_PULLUP:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            break;
            
        case PIN_MODE_INPUT_PULLDOWN:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
            break;
            
        case PIN_MODE_OUTPUT_OD:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
            break;
            
        default:
            return;
    }
    
    GPIO_Init(map->port, &GPIO_InitStructure);
}

/**
 * @brief ตั้งค่าโหมดของหลาย GPIO pins พร้อมกัน (internal)
 */
void _pinModeMultiple(const uint8_t* pins, uint8_t count, GPIO_PinMode mode) {
    if (!pins || count == 0) return;
    
    for (uint8_t i = 0; i < count; i++) {
        pinMode(pins[i], mode);
    }
}

/**
 * @brief เขียนค่า digital ไปยัง output pin
 */
void digitalWrite(uint8_t pin, uint8_t value) {
    const PinMap_t* map = getPinMap(pin);
    if (!map) return;
    
    if (value) {
        GPIO_SetBits(map->port, map->pin);
    } else {
        GPIO_ResetBits(map->port, map->pin);
    }
}

/**
 * @brief อ่านค่า digital จาก input pin
 */
uint8_t digitalRead(uint8_t pin) {
    const PinMap_t* map = getPinMap(pin);
    if (!map) return 0;
    
    return GPIO_ReadInputDataBit(map->port, map->pin) ? HIGH : LOW;
}

/**
 * @brief สลับสถานะของ output pin
 */
void digitalToggle(uint8_t pin) {
    const PinMap_t* map = getPinMap(pin);
    if (!map) return;
    
    // อ่านค่าปัจจุบันแล้วสลับ
    if (GPIO_ReadOutputDataBit(map->port, map->pin)) {
        GPIO_ResetBits(map->port, map->pin);
    } else {
        GPIO_SetBits(map->port, map->pin);
    }
}

/**
 * @brief ตั้งค่า external interrupt
 */
void attachInterrupt(uint8_t pin, void (*callback)(void), GPIO_InterruptMode mode) {
    const PinMap_t* map = getPinMap(pin);
    if (!map || !callback) return;
    
    // เปิด AFIO clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    // ตั้งค่า EXTI line
    GPIO_EXTILineConfig(map->port_source, map->pin_source);
    
    // ตั้งค่า EXTI
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    EXTI_InitStructure.EXTI_Line = map->pin;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    
    switch (mode) {
        case RISING:
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
            break;
        case FALLING:
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
            break;
        case CHANGE:
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
            break;
    }
    
    EXTI_Init(&EXTI_InitStructure);
    
    // เก็บ callback
    exti_callbacks[map->pin_source] = callback;
    
    // เปิด NVIC
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    
    // CH32V003 มี EXTI7_0_IRQn สำหรับ EXTI0-7
    NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief ยกเลิก external interrupt
 */
void detachInterrupt(uint8_t pin) {
    const PinMap_t* map = getPinMap(pin);
    if (!map) return;
    
    // ปิด EXTI line
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    EXTI_InitStructure.EXTI_Line = map->pin;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    // ลบ callback
    exti_callbacks[map->pin_source] = NULL;
}

/**
 * @brief เขียนค่าไปยัง port ทั้งหมด
 */
void portWrite(GPIO_TypeDef* port, uint8_t value) {
    GPIO_Write(port, value);
}

/**
 * @brief อ่านค่าจาก port ทั้งหมด
 */
uint8_t portRead(GPIO_TypeDef* port) {
    return (uint8_t)(GPIO_ReadInputData(port) & 0xFF);
}

/* ========== Analog Function Helpers ========== */

/**
 * @brief แมป GPIO pin เป็น ADC channel
 * @param pin GPIO pin number
 * @return ADC_Channel หรือ 0xFF ถ้าไม่รองรับ
 */
static uint8_t mapPinToADC(uint8_t pin) {
    switch(pin) {
        case PA2: return 0;  // ADC Channel 0
        case PA1: return 1;  // ADC Channel 1
        case PC4: return 2;  // ADC Channel 2
        case PD2: return 3;  // ADC Channel 3
        case PD3: return 4;  // ADC Channel 4
        case PD5: return 5;  // ADC Channel 5
        case PD6: return 6;  // ADC Channel 6
        case PD4: return 7;  // ADC Channel 7
        default: return 0xFF;
    }
}

/**
 * @brief แมป GPIO pin เป็น PWM channel
 * @param pin GPIO pin number
 * @return PWM_Channel หรือ 0xFF ถ้าไม่รองรับ
 */
static uint8_t mapPinToPWM(uint8_t pin) {
    switch(pin) {
        case PA1: return PWM1_CH2;
        case PC0: return PWM2_CH3;
        case PC3: return PWM1_CH3;
        case PC4: return PWM1_CH4;
        case PD2: return PWM1_CH1;
        case PD3: return PWM2_CH2;
        case PD4: return PWM2_CH1;
        case PD7: return PWM2_CH4;
        default: return 0xFF;
    }
}

/* ========== Analog Functions ========== */

/**
 * @brief อ่านค่า analog จาก ADC pin - Implementation with runtime validation
 */
uint16_t _analogRead_impl(uint8_t pin) {
    // Runtime validation สำหรับกรณีใช้ตัวแปร
    // ADC pins: PA1, PA2, PC4, PD2-PD6 (ไม่รวม PD7!)
    if (pin != PA1 && pin != PA2 && pin != PC4 &&
        (pin < PD2 || pin > PD6)) {
        return 0;  // Pin ไม่รองรับ ADC
    }
    
    // แมป pin เป็น ADC channel
    uint8_t adc_ch = mapPinToADC(pin);
    
    // ตรวจสอบว่า pin รองรับ ADC หรือไม่ (double-check)
    if (adc_ch == 0xFF) {
        return 0;  // Pin ไม่รองรับ ADC
    }
    
    // Init ADC ครั้งแรก
    if (!adc_initialized) {
        ADC_SimpleInit();
        adc_initialized = 1;
    }
    
    // อ่านค่า ADC
    return ADC_Read((ADC_Channel)adc_ch);
}

/**
 * @brief เขียนค่า PWM ไปยัง pin - Implementation with runtime validation
 */
void _analogWrite_impl(uint8_t pin, uint8_t value) {
    // Runtime validation สำหรับกรณีใช้ตัวแปร
    if (pin != PA1 && pin != PC0 && pin != PC3 && pin != PC4 &&
        pin != PD2 && pin != PD3 && pin != PD4 && pin != PD7) {
        return;  // Pin ไม่รองรับ PWM
    }
    
    // แมป pin เป็น PWM channel
    uint8_t pwm_ch = mapPinToPWM(pin);
    
    // ตรวจสอบว่า pin รองรับ PWM หรือไม่ (double-check)
    if (pwm_ch == 0xFF) {
        return;  // Pin ไม่รองรับ PWM
    }
    
    // เรียกใช้ PWM_Write (มี auto-init อยู่แล้ว)
    PWM_Write((PWM_Channel)pwm_ch, value);
}

/* ========== Advanced GPIO Functions ========== */

/**
 * @brief เขียนค่า digital ไปหลาย pins พร้อมกัน (internal)
 */
void _digitalWriteMultiple(const uint8_t* pins, const uint8_t* values, uint8_t count) {
    if (!pins || !values || count == 0) return;
    
    for (uint8_t i = 0; i < count; i++) {
        digitalWrite(pins[i], values[i]);
    }
}

/**
 * @brief วัดความกว้างของ pulse (microseconds)
 */
uint32_t pulseIn(uint8_t pin, uint8_t state, uint32_t timeout) {
    const PinMap_t* map = getPinMap(pin);
    if (!map) return 0;
    
    uint32_t start_time, pulse_start, pulse_end;
    uint8_t current_state;
    
    // รอให้ pin เป็นสถานะตรงข้ามกับที่ต้องการวัดก่อน
    start_time = Get_CurrentUs();
    while (1) {
        current_state = GPIO_ReadInputDataBit(map->port, map->pin);
        if (current_state != state) break;
        if (timeout && (Get_CurrentUs() - start_time) > timeout) return 0;
    }
    
    // รอให้ pulse เริ่ม
    start_time = Get_CurrentUs();
    while (1) {
        current_state = GPIO_ReadInputDataBit(map->port, map->pin);
        if (current_state == state) break;
        if (timeout && (Get_CurrentUs() - start_time) > timeout) return 0;
    }
    pulse_start = Get_CurrentUs();
    
    // รอให้ pulse จบ
    while (1) {
        current_state = GPIO_ReadInputDataBit(map->port, map->pin);
        if (current_state != state) break;
        if (timeout && (Get_CurrentUs() - pulse_start) > timeout) return 0;
    }
    pulse_end = Get_CurrentUs();
    
    return pulse_end - pulse_start;
}

/**
 * @brief ส่งข้อมูล 1 byte ผ่าน software SPI (shift out)
 */
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t value) {
    uint8_t i;
    
    for (i = 0; i < 8; i++) {
        if (bitOrder == LSBFIRST) {
            // ส่ง LSB ก่อน
            digitalWrite(dataPin, (value >> i) & 0x01);
        } else {
            // ส่ง MSB ก่อน
            digitalWrite(dataPin, (value >> (7 - i)) & 0x01);
        }
        
        // สร้าง clock pulse
        digitalWrite(clockPin, HIGH);
        Delay_Us(1);  // Clock high time
        digitalWrite(clockPin, LOW);
        Delay_Us(1);  // Clock low time
    }
}

/**
 * @brief รับข้อมูล 1 byte ผ่าน software SPI (shift in)
 */
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
    uint8_t value = 0;
    uint8_t i;
    
    for (i = 0; i < 8; i++) {
        // สร้าง clock pulse
        digitalWrite(clockPin, HIGH);
        Delay_Us(1);  // Clock high time
        
        // อ่านข้อมูล
        if (bitOrder == LSBFIRST) {
            // รับ LSB ก่อน
            value |= (digitalRead(dataPin) << i);
        } else {
            // รับ MSB ก่อน
            value |= (digitalRead(dataPin) << (7 - i));
        }
        
        digitalWrite(clockPin, LOW);
        Delay_Us(1);  // Clock low time
    }
    
    return value;
}

/* ========== Interrupt Handlers ========== */

/**
 * @brief EXTI interrupt handler
 * @note ใช้สำหรับ EXTI lines 0-7
 */
void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void) {
    // ตรวจสอบแต่ละ EXTI line
    for (uint8_t i = 0; i < 8; i++) {
        uint16_t exti_line = (1 << i);
        
        if (EXTI_GetITStatus(exti_line) != RESET) {
            // เรียก callback ถ้ามี
            if (exti_callbacks[i]) {
                exti_callbacks[i]();
            }
            
            // Clear interrupt flag
            EXTI_ClearITPendingBit(exti_line);
        }
    }
}
