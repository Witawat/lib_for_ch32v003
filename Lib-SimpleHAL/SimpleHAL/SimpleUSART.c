/**
 * @file SimpleUSART.c
 * @brief Simple USART Library Implementation
 * @version 1.0
 * @date 2025-12-12
 */

#include "SimpleUSART.h"
#include <string.h>

/* ========== Private Helper Functions ========== */

/**
 * @brief ส่ง 1 character ผ่าน USART (internal)
 */
static void USART_SendChar(char ch) {
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint8_t)ch);
}

/**
 * @brief แปลงตัวเลขเป็น string (internal)
 */
static void Int32ToString(int32_t num, char* str) {
    int i = 0;
    int is_negative = 0;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num = num / 10;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    // Reverse string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

/* ========== Public Functions ========== */

/**
 * @brief เริ่มต้นการใช้งาน USART
 */
void USART_SimpleInit(USART_BaudRate baud, USART_PinConfig pin_config) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    
    // 1. เปิด Clock สำหรับ USART1 และ GPIOD
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);
    
    // 2. ตั้งค่า Pin Remapping และ GPIO
    switch(pin_config) {
        case USART_PINS_DEFAULT:
            // Default: TX=PD5, RX=PD6 (ไม่ต้อง remap)
            
            // TX - Alternate Function Push-Pull
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            
            // RX - Input Floating
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            break;
            
        case USART_PINS_REMAP1:
            // Remap1: TX=PD0, RX=PD1
            GPIO_PinRemapConfig(GPIO_PartialRemap1_USART1, ENABLE);
            
            // TX
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            
            // RX
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            break;
            
        case USART_PINS_REMAP2:
            // Remap2: TX=PD6, RX=PD5
            GPIO_PinRemapConfig(GPIO_PartialRemap2_USART1, ENABLE);
            
            // TX
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            
            // RX
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            break;
    }
    
    // 3. ตั้งค่า USART
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    
    USART_Init(USART1, &USART_InitStructure);
    
    // 4. เปิดใช้งาน USART
    USART_Cmd(USART1, ENABLE);
}

/**
 * @brief ส่งข้อความแบบ string
 */
void USART_Print(const char* str) {
    while(*str) {
        USART_SendChar(*str++);
    }
}

/**
 * @brief ส่งตัวเลขแบบ decimal
 */
void USART_PrintNum(int32_t num) {
    char buffer[12];  // เพียงพอสำหรับ int32_t
    Int32ToString(num, buffer);
    USART_Print(buffer);
}

/**
 * @brief ส่งตัวเลขแบบ hexadecimal
 */
void USART_PrintHex(uint32_t num, uint8_t uppercase) {
    char buffer[11];  // "0x" + 8 hex digits + null
    const char* hex_chars = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    
    buffer[0] = '0';
    buffer[1] = 'x';
    
    // แปลงเป็น hex (8 digits)
    for(int i = 9; i >= 2; i--) {
        buffer[i] = hex_chars[num & 0x0F];
        num >>= 4;
    }
    
    buffer[10] = '\0';
    USART_Print(buffer);
}

/**
 * @brief ส่ง 1 byte
 */
void USART_WriteByte(uint8_t data) {
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, data);
}

/**
 * @brief ตรวจสอบว่ามีข้อมูลรอรับหรือไม่
 */
uint8_t USART_Available(void) {
    return (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) ? 1 : 0;
}

/**
 * @brief อ่านข้อมูล 1 byte (blocking)
 */
uint8_t USART_Read(void) {
    // รอจนกว่าจะมีข้อมูล
    while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    return (uint8_t)USART_ReceiveData(USART1);
}

/**
 * @brief อ่านข้อมูลหลาย bytes
 */
uint16_t USART_ReadBytes(uint8_t* buffer, uint16_t length) {
    uint16_t count = 0;
    
    while(count < length && USART_Available()) {
        buffer[count++] = USART_Read();
    }
    
    return count;
}

/**
 * @brief ล้างข้อมูลใน receive buffer
 */
void USART_Flush(void) {
    // อ่านข้อมูลทิ้งจนกว่าจะหมด
    while(USART_Available()) {
        (void)USART_ReceiveData(USART1);
    }
}
