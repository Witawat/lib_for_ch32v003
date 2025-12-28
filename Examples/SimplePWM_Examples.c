/**
 * @file SimplePWM_Examples.c
 * @brief ตัวอย่างการใช้งาน SimplePWM Library
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * ไฟล์นี้มีตัวอย่างการใช้งาน SimplePWM แบบต่างๆ
 * คัดลอกโค้ดไปใส่ใน main.c เพื่อทดสอบ
 */

#include "../SimplePWM.h"
#include "../SimpleGPIO.h"
#include "../Lib/Timer/timer.h"
#include <stdio.h>

/* ========== Example 1: Basic LED Fade ========== */

/**
 * @brief ตัวอย่างการทำ LED fade in/out
 */
void example_led_fade(void) {
    printf("LED Fade Example\r\n");
    
    // เริ่มต้น PWM ที่ 1kHz
    PWM_Init(PWM1_CH1, 1000);
    PWM_Start(PWM1_CH1);
    
    while(1) {
        // Fade in
        for (uint8_t i = 0; i <= 100; i++) {
            PWM_SetDutyCycle(PWM1_CH1, i);
            Delay_Ms(10);
        }
        
        // Fade out
        for (uint8_t i = 100; i > 0; i--) {
            PWM_SetDutyCycle(PWM1_CH1, i);
            Delay_Ms(10);
        }
    }
}

/* ========== Example 2: Arduino-style analogWrite ========== */

/**
 * @brief ตัวอย่างการใช้ PWM_Write (analogWrite)
 */
void example_analog_write(void) {
    printf("Arduino analogWrite Example\r\n");
    
    while(1) {
        // Fade in (0-255)
        for (uint16_t i = 0; i <= 255; i++) {
            PWM_Write(PWM1_CH1, i);
            Delay_Ms(5);
        }
        
        // Fade out (255-0)
        for (uint16_t i = 255; i > 0; i--) {
            PWM_Write(PWM1_CH1, i);
            Delay_Ms(5);
        }
    }
}

/* ========== Example 3: Multiple PWM Channels ========== */

/**
 * @brief ตัวอย่างการใช้หลาย PWM channels
 */
void example_multiple_pwm(void) {
    printf("Multiple PWM Channels Example\r\n");
    
    // เริ่มต้นหลาย channels
    PWM_Init(PWM1_CH1, 1000);
    PWM_Init(PWM1_CH2, 1000);
    PWM_Init(PWM1_CH3, 1000);
    
    PWM_Start(PWM1_CH1);
    PWM_Start(PWM1_CH2);
    PWM_Start(PWM1_CH3);
    
    while(1) {
        // แต่ละ channel มี duty cycle ต่างกัน
        for (uint8_t i = 0; i <= 100; i++) {
            PWM_SetDutyCycle(PWM1_CH1, i);
            PWM_SetDutyCycle(PWM1_CH2, 100 - i);
            PWM_SetDutyCycle(PWM1_CH3, (i * 2) % 100);
            Delay_Ms(20);
        }
    }
}

/* ========== Example 4: Servo Motor Control ========== */

/**
 * @brief ตัวอย่างการควบคุม servo motor
 * @note Servo ใช้ PWM 50Hz, pulse width 1-2ms (5-10% duty)
 */
void example_servo_control(void) {
    printf("Servo Motor Control Example\r\n");
    
    // Servo ต้องใช้ 50Hz
    PWM_Init(PWM1_CH1, 50);
    PWM_Start(PWM1_CH1);
    
    while(1) {
        // 0 degrees (1ms pulse = 5% duty @ 50Hz)
        printf("Servo: 0 degrees\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 5);
        Delay_Ms(1000);
        
        // 90 degrees (1.5ms pulse = 7.5% duty)
        printf("Servo: 90 degrees\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 7);
        Delay_Ms(1000);
        
        // 180 degrees (2ms pulse = 10% duty)
        printf("Servo: 180 degrees\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 10);
        Delay_Ms(1000);
        
        // กลับไป 90
        printf("Servo: 90 degrees\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 7);
        Delay_Ms(1000);
    }
}

/* ========== Example 5: Frequency Change ========== */

/**
 * @brief ตัวอย่างการเปลี่ยนความถี่ PWM
 */
void example_frequency_change(void) {
    printf("PWM Frequency Change Example\r\n");
    
    PWM_Init(PWM1_CH1, 1000);
    PWM_SetDutyCycle(PWM1_CH1, 50);  // 50% duty
    PWM_Start(PWM1_CH1);
    
    uint32_t frequencies[] = {100, 500, 1000, 2000, 5000, 10000};
    
    while(1) {
        for (uint8_t i = 0; i < 6; i++) {
            printf("Frequency: %lu Hz\r\n", frequencies[i]);
            PWM_SetFrequency(PWM1_CH1, frequencies[i]);
            PWM_SetDutyCycle(PWM1_CH1, 50);
            PWM_Start(PWM1_CH1);
            Delay_Ms(2000);
        }
    }
}

/* ========== Example 6: RGB LED Control ========== */

/**
 * @brief ตัวอย่างการควบคุม RGB LED
 */
void example_rgb_led(void) {
    printf("RGB LED Control Example\r\n");
    
    // R, G, B channels
    PWM_Init(PWM1_CH1, 1000);  // Red
    PWM_Init(PWM1_CH2, 1000);  // Green
    PWM_Init(PWM1_CH3, 1000);  // Blue
    
    PWM_Start(PWM1_CH1);
    PWM_Start(PWM1_CH2);
    PWM_Start(PWM1_CH3);
    
    while(1) {
        // Red
        printf("Color: Red\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 100);
        PWM_SetDutyCycle(PWM1_CH2, 0);
        PWM_SetDutyCycle(PWM1_CH3, 0);
        Delay_Ms(1000);
        
        // Green
        printf("Color: Green\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 0);
        PWM_SetDutyCycle(PWM1_CH2, 100);
        PWM_SetDutyCycle(PWM1_CH3, 0);
        Delay_Ms(1000);
        
        // Blue
        printf("Color: Blue\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 0);
        PWM_SetDutyCycle(PWM1_CH2, 0);
        PWM_SetDutyCycle(PWM1_CH3, 100);
        Delay_Ms(1000);
        
        // Yellow (R+G)
        printf("Color: Yellow\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 100);
        PWM_SetDutyCycle(PWM1_CH2, 100);
        PWM_SetDutyCycle(PWM1_CH3, 0);
        Delay_Ms(1000);
        
        // Cyan (G+B)
        printf("Color: Cyan\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 0);
        PWM_SetDutyCycle(PWM1_CH2, 100);
        PWM_SetDutyCycle(PWM1_CH3, 100);
        Delay_Ms(1000);
        
        // Magenta (R+B)
        printf("Color: Magenta\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 100);
        PWM_SetDutyCycle(PWM1_CH2, 0);
        PWM_SetDutyCycle(PWM1_CH3, 100);
        Delay_Ms(1000);
        
        // White (R+G+B)
        printf("Color: White\r\n");
        PWM_SetDutyCycle(PWM1_CH1, 100);
        PWM_SetDutyCycle(PWM1_CH2, 100);
        PWM_SetDutyCycle(PWM1_CH3, 100);
        Delay_Ms(1000);
    }
}

/* ========== Example 7: Motor Speed Control ========== */

/**
 * @brief ตัวอย่างการควบคุมความเร็วมอเตอร์
 */
void example_motor_speed(void) {
    printf("Motor Speed Control Example\r\n");
    
    PWM_Init(PWM1_CH1, 1000);  // Motor PWM
    PWM_Start(PWM1_CH1);
    
    while(1) {
        // Accelerate
        printf("Accelerating...\r\n");
        for (uint8_t speed = 0; speed <= 100; speed += 5) {
            PWM_SetDutyCycle(PWM1_CH1, speed);
            printf("Speed: %u%%\r\n", speed);
            Delay_Ms(200);
        }
        
        Delay_Ms(2000);
        
        // Decelerate
        printf("Decelerating...\r\n");
        for (uint8_t speed = 100; speed > 0; speed -= 5) {
            PWM_SetDutyCycle(PWM1_CH1, speed);
            printf("Speed: %u%%\r\n", speed);
            Delay_Ms(200);
        }
        
        Delay_Ms(2000);
    }
}

/* ========== Example 8: Breathing LED ========== */

/**
 * @brief ตัวอย่างการทำ LED breathing effect
 */
void example_breathing_led(void) {
    printf("Breathing LED Example\r\n");
    
    PWM_Init(PWM1_CH1, 1000);
    PWM_Start(PWM1_CH1);
    
    while(1) {
        // Breathe in (exponential curve)
        for (uint8_t i = 0; i <= 100; i++) {
            uint8_t brightness = (i * i) / 100;  // Quadratic curve
            PWM_SetDutyCycle(PWM1_CH1, brightness);
            Delay_Ms(15);
        }
        
        // Breathe out
        for (uint8_t i = 100; i > 0; i--) {
            uint8_t brightness = (i * i) / 100;
            PWM_SetDutyCycle(PWM1_CH1, brightness);
            Delay_Ms(15);
        }
        
        Delay_Ms(500);
    }
}

/* ========== Example 9: Advanced PWM Setup ========== */

/**
 * @brief ตัวอย่างการตั้งค่า PWM แบบละเอียด
 */
void example_advanced_pwm(void) {
    printf("Advanced PWM Setup Example\r\n");
    
    // ตั้งค่าแบบละเอียด
    // SystemCoreClock = 48MHz
    // PWM freq = 48MHz / ((prescaler+1) * (period+1))
    // Example: 1kHz = 48MHz / (48 * 1000)
    
    uint16_t prescaler = 47;
    uint16_t period = 999;
    uint16_t duty = 500;  // 50%
    
    PWM_AdvancedInit(PWM1_CH1, prescaler, period, duty);
    PWM_Start(PWM1_CH1);
    
    printf("PWM configured:\r\n");
    printf("  Prescaler: %u\r\n", prescaler);
    printf("  Period: %u\r\n", period);
    printf("  Duty: %u\r\n", duty);
    printf("  Frequency: %lu Hz\r\n", 
           SystemCoreClock / ((prescaler+1) * (period+1)));
    
    while(1) {
        Delay_Ms(1000);
    }
}

/* ========== Example 10: PWM with Button Control ========== */

volatile uint8_t brightness = 50;

void button_up_isr(void) {
    if (brightness < 100) brightness += 10;
    PWM_SetDutyCycle(PWM1_CH1, brightness);
    printf("Brightness: %u%%\r\n", brightness);
}

void button_down_isr(void) {
    if (brightness > 0) brightness -= 10;
    PWM_SetDutyCycle(PWM1_CH1, brightness);
    printf("Brightness: %u%%\r\n", brightness);
}

/**
 * @brief ตัวอย่างการควบคุม PWM ด้วยปุ่มกด
 */
void example_pwm_button_control(void) {
    printf("PWM Button Control Example\r\n");
    
    // Setup PWM
    PWM_Init(PWM1_CH1, 1000);
    PWM_SetDutyCycle(PWM1_CH1, brightness);
    PWM_Start(PWM1_CH1);
    
    // Setup buttons
    pinMode(PC1, PIN_MODE_INPUT_PULLUP);  // Up button
    pinMode(PC2, PIN_MODE_INPUT_PULLUP);  // Down button
    
    attachInterrupt(PC1, button_up_isr, FALLING);
    attachInterrupt(PC2, button_down_isr, FALLING);
    
    printf("Press PC1 to increase, PC2 to decrease brightness\r\n");
    printf("Initial brightness: %u%%\r\n", brightness);
    
    while(1) {
        Delay_Ms(100);
    }
}

/* ========== Main Function Template ========== */

#if 0  // เปลี่ยนเป็น 1 เพื่อ compile

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Timer_Init();
    
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#endif
    
    printf("SimplePWM Examples\r\n");
    printf("SystemClk: %d Hz\r\n", SystemCoreClock);
    
    // เลือก example ที่ต้องการทดสอบ
    // example_led_fade();
    // example_analog_write();
    // example_multiple_pwm();
    // example_servo_control();
    // example_frequency_change();
    // example_rgb_led();
    // example_motor_speed();
    // example_breathing_led();
    // example_advanced_pwm();
    example_pwm_button_control();
    
    return 0;
}

#endif
