/**
 * @file SimpleGPIO.h
 * @brief Simple GPIO Library สำหรับ CH32V003
 * @version 1.3
 * @date 2025-12-22
 * 
 * @details
 * Library นี้ให้ API สำหรับการควบคุม GPIO pins ของ CH32V003
 * รองรับ digital I/O, external interrupts, และ analog I/O
 * 
 * **คุณสมบัติ:**
 * - pinMode() สำหรับตั้งค่าโหมด
 * - digitalWrite() / digitalRead()
 * - digitalToggle() สำหรับสลับสถานะ
 * - attachInterrupt() / detachInterrupt()
 * - analogRead() สำหรับอ่านค่า ADC (PD2-PD7)
 * - analogWrite() สำหรับสร้าง PWM (PA1, PC0, PC3-4, PD2-4, PD7)
 * - ใช้ชื่อ pin ตามจริงของ CH32V003
 * - **Hybrid Pin Validation:** ตรวจสอบ pin ที่ไม่รองรับทั้งตอนคอมไพล์และ runtime
 * 
 * **GPIO Pins ของ CH32V003:**
 * - GPIOA: PA1-PA2 (PA1 รองรับ PWM)
 * - GPIOC: PC0-PC7 (PC0, PC3-4 รองรับ PWM)
 * - GPIOD: PD2-PD7 (รองรับ ADC ทุก pin, PD2-4, PD7 รองรับ PWM)
 * 
 * @example
 * // ตัวอย่างการใช้งาน
 * #include "SimpleGPIO.h"
 * 
 * int main(void) {
 *     // LED blink
 *     pinMode(PC0, PIN_MODE_OUTPUT);
 *     while(1) {
 *         digitalWrite(PC0, HIGH);
 *         Delay_Ms(500);
 *         digitalWrite(PC0, LOW);
 *         Delay_Ms(500);
 *     }
 * }
 * 
 * @note CH32V003 มี GPIO ports: GPIOA, GPIOC, GPIOD
 */

#ifndef __SIMPLE_GPIO_H
#define __SIMPLE_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include <stdint.h>

/* ========== Pin Definitions ========== */

/**
 * @brief CH32V003 GPIO Pin Definitions
 * @details ชื่อ pin ตามจริงของ CH32V003
 */
typedef enum {
    // GPIOA Pins (PA1-PA2)
    PA1 = 0,   /**< GPIOA Pin 1 - PWM1_CH2 */
    PA2 = 1,   /**< GPIOA Pin 2 */
    
    // Reserved (2-9)
    
    // GPIOC Pins (PC0-PC7)
    PC0 = 10,  /**< GPIOC Pin 0 - PWM2_CH3 */
    PC1 = 11,  /**< GPIOC Pin 1 */
    PC2 = 12,  /**< GPIOC Pin 2 */
    PC3 = 13,  /**< GPIOC Pin 3 - PWM1_CH3 */
    PC4 = 14,  /**< GPIOC Pin 4 - PWM1_CH4 */
    PC5 = 15,  /**< GPIOC Pin 5 */
    PC6 = 16,  /**< GPIOC Pin 6 */
    PC7 = 17,  /**< GPIOC Pin 7 */
    
    // Reserved (18-19)
    
    // GPIOD Pins (PD2-PD7)
    PD2 = 20,  /**< GPIOD Pin 2 - ADC Channel 3, PWM1_CH1 */
    PD3 = 21,  /**< GPIOD Pin 3 - ADC Channel 4, PWM2_CH2 */
    PD4 = 22,  /**< GPIOD Pin 4 - ADC Channel 7, PWM2_CH1 */
    PD5 = 23,  /**< GPIOD Pin 5 - ADC Channel 5 */
    PD6 = 24,  /**< GPIOD Pin 6 - ADC Channel 6 */
    PD7 = 25   /**< GPIOD Pin 7 - ADC Channel 0, PWM2_CH4 */
} GPIO_Pin;


/* ========== Pin Mode Definitions ========== */

/**
 * @brief GPIO Pin Modes
 */
typedef enum {
    PIN_MODE_INPUT = 0,           /**< Input floating */
    PIN_MODE_OUTPUT,              /**< Output push-pull */
    PIN_MODE_INPUT_PULLUP,        /**< Input with pull-up */
    PIN_MODE_INPUT_PULLDOWN,      /**< Input with pull-down */
    PIN_MODE_OUTPUT_OD            /**< Output open-drain */
} GPIO_PinMode;

/* ========== Digital Values ========== */

#ifndef HIGH
#define HIGH  1   /**< Logic high */
#endif

#ifndef LOW
#define LOW   0   /**< Logic low */
#endif

/* ========== Bit Order (for Shift Functions) ========== */

#ifndef LSBFIRST
#define LSBFIRST  0   /**< Least Significant Bit first */
#endif

#ifndef MSBFIRST
#define MSBFIRST  1   /**< Most Significant Bit first */
#endif

/* ========== Interrupt Modes ========== */

/**
 * @brief External Interrupt Trigger Modes
 */
typedef enum {
    RISING = 0,    /**< Trigger on rising edge */
    FALLING,       /**< Trigger on falling edge */
    CHANGE         /**< Trigger on both edges */
} GPIO_InterruptMode;

/* ========== Function Prototypes ========== */

/**
 * @brief ตั้งค่าโหมดของ GPIO pin
 * @param pin หมายเลข pin (PA1-PA2, PC0-PC7, PD2-PD7)
 * @param mode โหมดที่ต้องการ (PIN_MODE_INPUT, PIN_MODE_OUTPUT, ...)
 * 
 * @note ต้องเรียกฟังก์ชันนี้ก่อนใช้งาน pin
 * 
 * @example
 * pinMode(PA1, PIN_MODE_OUTPUT);        // PA1 output
 * pinMode(PC0, PIN_MODE_OUTPUT);        // LED output
 * pinMode(PC1, PIN_MODE_INPUT_PULLUP);  // Button input
 */
void pinMode(uint8_t pin, GPIO_PinMode mode);

/**
 * @brief ตั้งค่าโหมดของหลาย GPIO pins พร้อมกัน (internal function)
 * @param pins array ของ pin numbers
 * @param count จำนวน pins
 * @param mode โหมดที่ต้องการ
 * 
 * @note ใช้ macro pinModeMultiple() แทนการเรียกฟังก์ชันนี้โดยตรง
 */
void _pinModeMultiple(const uint8_t* pins, uint8_t count, GPIO_PinMode mode);

/**
 * @brief Macro สำหรับตั้งค่าโหมดของหลาย GPIO pins พร้อมกัน
 * @param pins_array array ของ pin numbers (ต้องเป็น array ที่ประกาศด้วย const uint8_t arr[] = {...})
 * @param mode โหมดที่ต้องการ (PIN_MODE_INPUT, PIN_MODE_OUTPUT, ...)
 * 
 * @note Macro นี้จะคำนวณจำนวน pins อัตโนมัติ ไม่ต้องระบุ count
 * @note ใช้ได้เฉพาะ array ที่ประกาศแบบ const uint8_t arr[] = {...}
 * 
 * @example
 * // ตั้งค่า LED 3 ดวงเป็น output พร้อมกัน (ไม่ต้องบอกจำนวน!)
 * const uint8_t leds[] = {PC0, PC1, PC2};
 * pinModeMultiple(leds, PIN_MODE_OUTPUT);
 * 
 * // ตั้งค่าปุ่ม 4 ปุ่มเป็น input pullup พร้อมกัน
 * const uint8_t buttons[] = {PD2, PD3, PD4, PD5};
 * pinModeMultiple(buttons, PIN_MODE_INPUT_PULLUP);
 */
#define pinModeMultiple(pins_array, mode) \
    _pinModeMultiple(pins_array, sizeof(pins_array)/sizeof(pins_array[0]), mode)

/**
 * @brief เขียนค่า digital ไปยัง output pin
 * @param pin หมายเลข pin (PA1-PA2, PC0-PC7, PD2-PD7)
 * @param value ค่าที่ต้องการเขียน (HIGH หรือ LOW)
 * 
 * @note Pin ต้องถูกตั้งเป็น OUTPUT mode ก่อน
 * 
 * @example
 * digitalWrite(PC0, HIGH);  // เปิด LED
 * digitalWrite(PC0, LOW);   // ปิด LED
 */
void digitalWrite(uint8_t pin, uint8_t value);

/**
 * @brief อ่านค่า digital จาก input pin
 * @param pin หมายเลข pin (PA1-PA2, PC0-PC7, PD2-PD7)
 * @return ค่าที่อ่านได้ (HIGH หรือ LOW)
 * 
 * @note Pin ต้องถูกตั้งเป็น INPUT mode ก่อน
 * 
 * @example
 * uint8_t button_state = digitalRead(PC1);
 * if(button_state == LOW) {
 *     // Button pressed (active low)
 * }
 */
uint8_t digitalRead(uint8_t pin);

/**
 * @brief สลับสถานะของ output pin
 * @param pin หมายเลข pin (PA1-PA2, PC0-PC7, PD2-PD7)
 * 
 * @note Pin ต้องถูกตั้งเป็น OUTPUT mode ก่อน
 * 
 * @example
 * pinMode(PC0, PIN_MODE_OUTPUT);
 * while(1) {
 *     digitalToggle(PC0);  // Toggle LED
 *     Delay_Ms(500);
 * }
 */
void digitalToggle(uint8_t pin);

/**
 * @brief ตั้งค่า external interrupt สำหรับ pin
 * @param pin หมายเลข pin (PA1-PA2, PC0-PC7, PD2-PD7)
 * @param callback ฟังก์ชันที่จะถูกเรียกเมื่อเกิด interrupt
 * @param mode โหมด interrupt (RISING, FALLING, CHANGE)
 * 
 * @note Pin ต้องถูกตั้งเป็น INPUT mode ก่อน
 * @note CH32V003 รองรับ EXTI สูงสุด 8 lines
 * 
 * @example
 * void button_isr(void) {
 *     // Handle button press
 * }
 * 
 * pinMode(PC1, PIN_MODE_INPUT_PULLUP);
 * attachInterrupt(PC1, button_isr, FALLING);
 */
void attachInterrupt(uint8_t pin, void (*callback)(void), GPIO_InterruptMode mode);

/**
 * @brief ยกเลิก external interrupt ของ pin
 * @param pin หมายเลข pin (PC0-PC7, PD2-PD7)
 * 
 * @example
 * detachInterrupt(PC1);  // ปิด interrupt ของ PC1
 */
void detachInterrupt(uint8_t pin);

/* ========== Advanced Functions ========== */

/**
 * @brief เขียนค่าไปยังหลาย pins พร้อมกัน (port write)
 * @param port GPIO port (GPIOA, GPIOC, GPIOD)
 * @param value ค่า 8-bit ที่ต้องการเขียน
 * 
 * @note ใช้สำหรับการควบคุมหลาย pins พร้อมกันอย่างรวดเร็ว
 * 
 * @example
 * // เขียนค่า 0xFF ไปยัง GPIOC (PC0-PC7)
 * portWrite(GPIOC, 0xFF);
 */
void portWrite(GPIO_TypeDef* port, uint8_t value);

/**
 * @brief อ่านค่าจาก port ทั้งหมด
 * @param port GPIO port (GPIOA, GPIOC, GPIOD)
 * @return ค่า 8-bit ที่อ่านได้
 * 
 * @example
 * uint8_t port_value = portRead(GPIOC);
 */
uint8_t portRead(GPIO_TypeDef* port);

/* ========== Pin Validation Macros ========== */

/**
 * @brief ตรวจสอบว่า pin รองรับ ADC หรือไม่
 * @note ADC pins: PA1, PA2, PC4, PD2, PD3, PD4, PD5, PD6 (8 pins)
 */
#define IS_ADC_PIN(pin) ((pin) == PA1 || (pin) == PA2 || (pin) == PC4 || \
                         ((pin) >= PD2 && (pin) <= PD6))

/**
 * @brief ตรวจสอบว่า pin รองรับ PWM หรือไม่
 * @note Pins ที่รองรับ: PA1, PC0, PC3, PC4, PD2, PD3, PD4, PD7
 */
#define IS_PWM_PIN(pin) ((pin) == PA1 || (pin) == PC0 || \
                         (pin) == PC3 || (pin) == PC4 || \
                         (pin) == PD2 || (pin) == PD3 || \
                         (pin) == PD4 || (pin) == PD7)

/* ========== Analog Functions ========== */

/**
 * @brief อ่านค่า analog จาก ADC pin (Arduino-style) - Implementation function
 * @param pin หมายเลข pin (PA1, PA2, PC4, PD2-PD6 เท่านั้น)
 * @return ค่า ADC (0-1023), หรือ 0 ถ้า pin ไม่รองรับ
 * 
 * @note ใช้ macro analogRead() แทนการเรียกฟังก์ชันนี้โดยตรง
 * @note ฟังก์ชันนี้มี runtime validation สำหรับกรณีใช้ตัวแปร
 */
uint16_t _analogRead_impl(uint8_t pin);

/**
 * @brief อ่านค่า analog จาก ADC pin (Arduino-style)
 * @param pin หมายเลข pin (PA1, PA2, PC4, PD2-PD6 เท่านั้น)
 * @return ค่า ADC (0-1023), หรือ 0 ถ้า pin ไม่รองรับ
 * 
 * @note ADC pins: PA1, PA2, PC4, PD2, PD3, PD4, PD5, PD6 (8 pins)
 * @note PD7 ไม่รองรับ ADC!
 * @note ADC จะถูก init อัตโนมัติในครั้งแรกที่เรียกใช้
 * @note ใช้ร่วมกับ SimpleADC library
 * 
 * **Pin Validation:**
 * - ถ้าใช้ constant (เช่น PD2) → ตรวจสอบตอนคอมไพล์ (compile-time error)
 * - ถ้าใช้ตัวแปร (เช่น my_pin) → ตรวจสอบตอน runtime (return 0)
 * 
 * @example
 * // Compile-time validation (constant pins)
 * uint16_t value = analogRead(PD2);  // ✓ OK
 * uint16_t value = analogRead(PA1);  // ✓ OK - PA1 รองรับ ADC
 * uint16_t value = analogRead(PC4);  // ✓ OK - PC4 รองรับ ADC
 * uint16_t value = analogRead(PD7);  // ✗ Compile Error! PD7 ไม่รองรับ ADC
 * uint16_t value = analogRead(PC0);  // ✗ Compile Error!
 * 
 * // Runtime validation (variable pins)
 * uint8_t my_pin = get_sensor_pin();
 * uint16_t value = analogRead(my_pin);  // ตรวจสอบตอน runtime
 * if (value == 0) {
 *     // Pin ไม่รองรับ ADC หรือเกิดข้อผิดพลาด
 * }
 * 
 * float voltage = (value / 1023.0) * 3.3;
 */
#define analogRead(pin) __builtin_choose_expr( \
    __builtin_constant_p(pin), \
    ({ \
        _Static_assert(IS_ADC_PIN(pin), \
            "analogRead: Pin does not support ADC! Only PA1, PA2, PC4, PD2-PD6 support ADC."); \
        _analogRead_impl(pin); \
    }), \
    _analogRead_impl(pin) \
)

/**
 * @brief เขียนค่า PWM ไปยัง pin (Arduino-style) - Implementation function
 * @param pin หมายเลข pin ที่รองรับ PWM
 * @param value ค่า PWM (0-255), 0 = 0%, 255 = 100%
 * 
 * @note ใช้ macro analogWrite() แทนการเรียกฟังก์ชันนี้โดยตรง
 * @note ฟังก์ชันนี้มี runtime validation สำหรับกรณีใช้ตัวแปร
 */
void _analogWrite_impl(uint8_t pin, uint8_t value);

/**
 * @brief เขียนค่า PWM ไปยัง pin (Arduino-style)
 * @param pin หมายเลข pin ที่รองรับ PWM
 * @param value ค่า PWM (0-255), 0 = 0%, 255 = 100%
 * 
 * @note Pins ที่รองรับ: PA1, PC0, PC3, PC4, PD2, PD3, PD4, PD7
 * @note PWM จะถูก init อัตโนมัติที่ 1kHz ในครั้งแรกที่เรียกใช้
 * @note ใช้ร่วมกับ SimplePWM library
 * @note ถ้าต้องการความถี่อื่น ให้ใช้ PWM_Init() ก่อน analogWrite()
 * 
 * **Pin Validation:**
 * - ถ้าใช้ constant (เช่น PA1) → ตรวจสอบตอนคอมไพล์ (compile-time error)
 * - ถ้าใช้ตัวแปร (เช่น my_pin) → ตรวจสอบตอน runtime (ไม่ทำงาน)
 * 
 * @example
 * // Compile-time validation (constant pins)
 * analogWrite(PA1, 128);  // ✓ OK - 50% duty cycle
 * analogWrite(PA2, 128);  // ✗ Compile Error!
 * 
 * // Runtime validation (variable pins)
 * uint8_t led_pin = get_led_pin();
 * analogWrite(led_pin, 128);  // ตรวจสอบตอน runtime
 * 
 * // ใช้ความถี่เริ่มต้น 1kHz
 * analogWrite(PA1, 128);  // 50% duty cycle @ 1kHz on PA1
 * analogWrite(PC3, 128);  // 50% duty cycle @ 1kHz on PC3
 * 
 * // ใช้ความถี่ที่กำหนดเอง
 * PWM_Init(PWM1_CH3, 25000);  // Init ที่ 25kHz ก่อน
 * PWM_Start(PWM1_CH3);
 * analogWrite(PC3, 128);      // 50% duty cycle @ 25kHz
 */
#define analogWrite(pin, value) __builtin_choose_expr( \
    __builtin_constant_p(pin), \
    ({ \
        _Static_assert(IS_PWM_PIN(pin), \
            "analogWrite: Pin does not support PWM! Supported: PA1, PC0, PC3-4, PD2-4, PD7"); \
        _analogWrite_impl(pin, value); \
    }), \
    _analogWrite_impl(pin, value) \
)

/**
 * @brief เขียนค่า digital ไปหลาย pins พร้อมกัน (internal function)
 * @param pins array ของ pin numbers
 * @param values array ของค่าที่ต้องการเขียน (HIGH/LOW)
 * @param count จำนวน pins
 * 
 * @note ใช้ macro digitalWriteMultiple() แทนการเรียกฟังก์ชันนี้โดยตรง
 */
void _digitalWriteMultiple(const uint8_t* pins, const uint8_t* values, uint8_t count);

/**
 * @brief Macro สำหรับเขียนค่า digital ไปหลาย pins พร้อมกัน
 * @param pins_array array ของ pin numbers
 * @param values_array array ของค่าที่ต้องการเขียน (HIGH/LOW)
 * 
 * @note Macro นี้จะคำนวณจำนวน pins อัตโนมัติ
 * @note ขนาด pins_array และ values_array ต้องเท่ากัน
 * 
 * @example
 * const uint8_t leds[] = {PC0, PC1, PC2};
 * const uint8_t states[] = {HIGH, LOW, HIGH};
 * digitalWriteMultiple(leds, states);  // ตั้งค่าทีเดียว
 */
#define digitalWriteMultiple(pins_array, values_array) \
    _digitalWriteMultiple(pins_array, values_array, sizeof(pins_array)/sizeof(pins_array[0]))

/**
 * @brief วัดความกว้างของ pulse (microseconds)
 * @param pin หมายเลข pin (PA1-PA2, PC0-PC7, PD2-PD7)
 * @param state ชนิดของ pulse ที่ต้องการวัด (HIGH หรือ LOW)
 * @param timeout ระยะเวลา timeout (microseconds), 0 = ไม่มี timeout
 * @return ความกว้างของ pulse (microseconds), 0 = timeout หรือ error
 * 
 * @note ใช้สำหรับวัด pulse จาก ultrasonic sensor, IR receiver
 * @note ความละเอียด ~1us (ขึ้นกับ clock speed)
 * 
 * @example
 * // Ultrasonic HC-SR04
 * digitalWrite(TRIG_PIN, LOW);
 * Delay_Us(2);
 * digitalWrite(TRIG_PIN, HIGH);
 * Delay_Us(10);
 * digitalWrite(TRIG_PIN, LOW);
 * 
 * uint32_t duration = pulseIn(ECHO_PIN, HIGH, 30000);  // timeout 30ms
 * float distance_cm = duration * 0.034 / 2;
 */
uint32_t pulseIn(uint8_t pin, uint8_t state, uint32_t timeout);

/**
 * @brief ส่งข้อมูล 1 byte ผ่าน software SPI (shift out)
 * @param dataPin pin สำหรับส่งข้อมูล
 * @param clockPin pin สำหรับ clock signal
 * @param bitOrder ลำดับการส่ง bit (LSBFIRST หรือ MSBFIRST)
 * @param value ข้อมูลที่ต้องการส่ง (1 byte)
 * 
 * @note ใช้สำหรับควบคุม 74HC595, LED matrix, shift register
 * @note Clock frequency ~100kHz (ขึ้นกับ system clock)
 * 
 * @example
 * // ควบคุม 74HC595 (8-bit shift register)
 * #define DATA_PIN  PC0
 * #define CLOCK_PIN PC1
 * #define LATCH_PIN PC2
 * 
 * digitalWrite(LATCH_PIN, LOW);
 * shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0b10101010);
 * digitalWrite(LATCH_PIN, HIGH);  // Latch data
 */
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t value);

/**
 * @brief รับข้อมูล 1 byte ผ่าน software SPI (shift in)
 * @param dataPin pin สำหรับรับข้อมูล
 * @param clockPin pin สำหรับ clock signal
 * @param bitOrder ลำดับการรับ bit (LSBFIRST หรือ MSBFIRST)
 * @return ข้อมูลที่รับได้ (1 byte)
 * 
 * @note ใช้สำหรับอ่าน 74HC165, keypad matrix, shift register
 * @note Clock frequency ~100kHz (ขึ้นกับ system clock)
 * 
 * @example
 * // อ่าน 74HC165 (8-bit shift register)
 * #define DATA_PIN  PD2
 * #define CLOCK_PIN PD3
 * #define LOAD_PIN  PD4
 * 
 * digitalWrite(LOAD_PIN, LOW);
 * Delay_Us(5);
 * digitalWrite(LOAD_PIN, HIGH);  // Load data
 * 
 * uint8_t data = shiftIn(DATA_PIN, CLOCK_PIN, MSBFIRST);
 */
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_GPIO_H
