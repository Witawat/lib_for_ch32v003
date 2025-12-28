/**
 * @file SimpleGPIO_Examples.c
 * @brief ตัวอย่างการใช้งาน SimpleGPIO Library
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * ไฟล์นี้มีตัวอย่างการใช้งาน SimpleGPIO แบบต่างๆ
 * คัดลอกโค้ดไปใส่ใน main.c เพื่อทดสอบ
 */

#include "../SimpleGPIO.h"
#include "../Lib/Timer/timer.h"
#include <stdio.h>

/* ========== Example 1: Basic LED Blink ========== */

/**
 * @brief ตัวอย่างการกระพริบ LED แบบง่าย
 */
void example_led_blink(void) {
    // ตั้งค่า PC0 เป็น output
    pinMode(PC0, PIN_MODE_OUTPUT);
    
    while(1) {
        digitalWrite(PC0, HIGH);  // เปิด LED
        Delay_Ms(500);
        digitalWrite(PC0, LOW);   // ปิด LED
        Delay_Ms(500);
    }
}

/* ========== Example 2: LED Blink with Toggle ========== */

/**
 * @brief ตัวอย่างการกระพริบ LED ด้วย digitalToggle
 */
void example_led_toggle(void) {
    pinMode(PC0, PIN_MODE_OUTPUT);
    
    while(1) {
        digitalToggle(PC0);  // สลับสถานะ LED
        Delay_Ms(500);
    }
}

/* ========== Example 3: Button Reading ========== */

/**
 * @brief ตัวอย่างการอ่านค่าปุ่มกด
 */
void example_button_read(void) {
    pinMode(PC0, PIN_MODE_OUTPUT);           // LED
    pinMode(PC1, PIN_MODE_INPUT_PULLUP);     // Button (active low)
    
    while(1) {
        uint8_t button_state = digitalRead(PC1);
        
        if (button_state == LOW) {
            // ปุ่มถูกกด -> เปิด LED
            digitalWrite(PC0, HIGH);
        } else {
            // ปุ่มไม่ถูกกด -> ปิด LED
            digitalWrite(PC0, LOW);
        }
        
        Delay_Ms(10);  // Debounce delay
    }
}

/* ========== Example 4: Multiple LEDs ========== */

/**
 * @brief ตัวอย่างการควบคุมหลาย LEDs
 */
void example_multiple_leds(void) {
    // ตั้งค่า PC0-PC3 เป็น outputs
    pinMode(PC0, PIN_MODE_OUTPUT);
    pinMode(PC1, PIN_MODE_OUTPUT);
    pinMode(PC2, PIN_MODE_OUTPUT);
    pinMode(PC3, PIN_MODE_OUTPUT);
    
    while(1) {
        // Running light pattern
        digitalWrite(PC0, HIGH);
        Delay_Ms(200);
        digitalWrite(PC0, LOW);
        
        digitalWrite(PC1, HIGH);
        Delay_Ms(200);
        digitalWrite(PC1, LOW);
        
        digitalWrite(PC2, HIGH);
        Delay_Ms(200);
        digitalWrite(PC2, LOW);
        
        digitalWrite(PC3, HIGH);
        Delay_Ms(200);
        digitalWrite(PC3, LOW);
    }
}

/* ========== Example 5: Button Interrupt ========== */

volatile uint8_t led_state = 0;

/**
 * @brief Interrupt callback สำหรับปุ่มกด
 */
void button_isr(void) {
    // สลับสถานะ LED เมื่อกดปุ่ม
    led_state = !led_state;
    digitalWrite(PC0, led_state);
}

/**
 * @brief ตัวอย่างการใช้งาน interrupt
 */
void example_button_interrupt(void) {
    pinMode(PC0, PIN_MODE_OUTPUT);
    pinMode(PC1, PIN_MODE_INPUT_PULLUP);
    
    // ตั้งค่า interrupt ให้ trigger เมื่อปุ่มกด (falling edge)
    attachInterrupt(PC1, button_isr, FALLING);
    
    printf("Button interrupt example running...\r\n");
    printf("Press button on PC1 to toggle LED on PC0\r\n");
    
    while(1) {
        // Main loop ว่าง - ทุกอย่างทำงานใน interrupt
        Delay_Ms(100);
    }
}

/* ========== Example 6: Multiple Interrupts ========== */

volatile uint32_t button1_count = 0;
volatile uint32_t button2_count = 0;

void button1_isr(void) {
    button1_count++;
    printf("Button 1 pressed: %lu times\r\n", button1_count);
}

void button2_isr(void) {
    button2_count++;
    printf("Button 2 pressed: %lu times\r\n", button2_count);
}

/**
 * @brief ตัวอย่างการใช้งานหลาย interrupts
 */
void example_multiple_interrupts(void) {
    pinMode(PC1, PIN_MODE_INPUT_PULLUP);
    pinMode(PC2, PIN_MODE_INPUT_PULLUP);
    
    attachInterrupt(PC1, button1_isr, FALLING);
    attachInterrupt(PC2, button2_isr, FALLING);
    
    printf("Multiple interrupts example\r\n");
    
    while(1) {
        Delay_Ms(100);
    }
}

/* ========== Example 7: Port Write ========== */

/**
 * @brief ตัวอย่างการเขียนค่าไปยัง port ทั้งหมด
 */
void example_port_write(void) {
    // ตั้งค่า PC0-PC7 เป็น outputs
    for (uint8_t i = PC0; i <= PC7; i++) {
        pinMode(i, PIN_MODE_OUTPUT);
    }
    
    uint8_t pattern = 0x01;
    
    while(1) {
        // เขียน pattern ไปยัง GPIOC (PC0-PC7)
        portWrite(GPIOC, pattern);
        
        // Shift pattern
        pattern <<= 1;
        if (pattern == 0) pattern = 0x01;
        
        Delay_Ms(100);
    }
}

/* ========== Example 8: Binary Counter Display ========== */

/**
 * @brief ตัวอย่างการแสดงผล binary counter ด้วย LEDs
 */
void example_binary_counter(void) {
    // ตั้งค่า PC0-PC7 เป็น outputs
    for (uint8_t i = PC0; i <= PC7; i++) {
        pinMode(i, PIN_MODE_OUTPUT);
    }
    
    uint8_t counter = 0;
    
    while(1) {
        // แสดงค่า counter เป็น binary บน LEDs
        portWrite(GPIOC, counter);
        
        printf("Counter: %d (0x%02X)\r\n", counter, counter);
        
        counter++;
        Delay_Ms(500);
    }
}

/* ========== Example 9: Debounced Button ========== */

/**
 * @brief ตัวอย่างการอ่านปุ่มพร้อม debounce
 */
uint8_t readButtonDebounced(uint8_t pin) {
    static uint8_t last_state = HIGH;
    static uint32_t last_change = 0;
    const uint32_t debounce_delay = 50;  // ms
    
    uint8_t current_state = digitalRead(pin);
    uint32_t current_time = Get_CurrentMs();
    
    if (current_state != last_state) {
        last_change = current_time;
    }
    
    if ((current_time - last_change) > debounce_delay) {
        last_state = current_state;
        return current_state;
    }
    
    return last_state;
}

void example_debounced_button(void) {
    pinMode(PC0, PIN_MODE_OUTPUT);
    pinMode(PC1, PIN_MODE_INPUT_PULLUP);
    
    uint8_t last_button = HIGH;
    
    while(1) {
        uint8_t button = readButtonDebounced(PC1);
        
        // ตรวจจับ falling edge (ปุ่มกด)
        if (button == LOW && last_button == HIGH) {
            digitalToggle(PC0);
            printf("Button pressed!\r\n");
        }
        
        last_button = button;
        Delay_Ms(10);
    }
}

/* ========== Example 10: Complete Application ========== */

/**
 * @brief ตัวอย่างแอปพลิเคชันสมบูรณ์
 * @details รวม LED, Button, และ Interrupt
 */
void example_complete_app(void) {
    // Setup
    pinMode(PC0, PIN_MODE_OUTPUT);  // Status LED
    pinMode(PC1, PIN_MODE_OUTPUT);  // Activity LED
    pinMode(PC2, PIN_MODE_INPUT_PULLUP);  // Button
    
    uint32_t last_blink = 0;
    uint8_t blink_state = 0;
    
    // Interrupt สำหรับปุ่ม
    attachInterrupt(PC2, button_isr, FALLING);
    
    printf("Complete application example\r\n");
    
    while(1) {
        uint32_t now = Get_CurrentMs();
        
        // Blink status LED ทุก 500ms
        if (now - last_blink >= 500) {
            blink_state = !blink_state;
            digitalWrite(PC0, blink_state);
            last_blink = now;
        }
        
        Delay_Ms(10);
    }
}

/* ========== Main Function Template ========== */

#if 0  // เปลี่ยนเป็น 1 เพื่อ compile

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Timer_Init();  // ใช้ timer จาก SimpleHAL
    
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#endif
    
    printf("SimpleGPIO Examples\r\n");
    printf("SystemClk: %d Hz\r\n", SystemCoreClock);
    
    // เลือก example ที่ต้องการทดสอบ
    // example_led_blink();
    // example_led_toggle();
    // example_button_read();
    // example_multiple_leds();
    // example_button_interrupt();
    // example_multiple_interrupts();
    // example_port_write();
    // example_binary_counter();
    // example_debounced_button();
    example_complete_app();
    
    return 0;
}

#endif
