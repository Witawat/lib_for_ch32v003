/**
 * @file SimpleSPI.c
 * @brief Simple SPI Library Implementation
 * @version 1.0
 * @date 2025-12-12
 */

#include "SimpleSPI.h"

/* ========== Private Variables ========== */

static GPIO_TypeDef* cs_port = GPIOC;
static uint16_t cs_pin = GPIO_Pin_4;

/* ========== Public Functions ========== */

/**
 * @brief เริ่มต้นการใช้งาน SPI
 */
void SPI_SimpleInit(SPI_Mode mode, SPI_Speed speed, SPI_PinConfig pin_config) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef SPI_InitStructure = {0};
    
    // 1. เปิด Clock
    if(pin_config == SPI_PINS_DEFAULT) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1, ENABLE);
    } else {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1, ENABLE);
    }
    
    // 2. ตั้งค่า Pin Remapping และ GPIO
    switch(pin_config) {
        case SPI_PINS_DEFAULT:
            // Default: SCK=PC5, MISO=PC7, MOSI=PC6, NSS=PC4
            
            // SCK และ MOSI - Alternate Function Push-Pull
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            
            // MISO - Input Floating
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            
            // NSS (CS) - Output Push-Pull (software control)
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            
            cs_port = GPIOC;
            cs_pin = GPIO_Pin_4;
            break;
            
        case SPI_PINS_REMAP:
            // Remap: SCK=PC6, MISO=PC8, MOSI=PC7, NSS=PC5
            GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
            
            // SCK และ MOSI
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            
            // MISO (PC8 อาจต้องใช้ GPIOC หรือ port อื่น ตาม datasheet)
            // สำหรับ CH32V003 อาจมีข้อจำกัด ให้ตรวจสอบ datasheet
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            
            // NSS (CS)
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            
            cs_port = GPIOC;
            cs_pin = GPIO_Pin_5;
            break;
    }
    
    // 3. ตั้งค่า SPI
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    
    // ตั้งค่า CPOL และ CPHA ตาม SPI Mode
    switch(mode) {
        case SPI_MODE0:
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
            break;
        case SPI_MODE1:
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
            break;
        case SPI_MODE2:
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
            break;
        case SPI_MODE3:
            SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
            SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
            break;
    }
    
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = (speed << 3);  // Convert enum to prescaler
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    
    SPI_Init(SPI1, &SPI_InitStructure);
    
    // 4. เปิดใช้งาน SPI
    SPI_Cmd(SPI1, ENABLE);
    
    // 5. ตั้งค่า CS เป็น HIGH (inactive)
    GPIO_SetBits(cs_port, cs_pin);
}

/**
 * @brief ส่งและรับข้อมูล 1 byte
 */
uint8_t SPI_Transfer(uint8_t data) {
    // รอจนกว่า TX buffer ว่าง
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    
    // ส่งข้อมูล
    SPI_I2S_SendData(SPI1, data);
    
    // รอจนกว่า RX buffer มีข้อมูล
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    
    // รับข้อมูล
    return (uint8_t)SPI_I2S_ReceiveData(SPI1);
}

/**
 * @brief ส่งและรับข้อมูลหลาย bytes
 */
void SPI_TransferBuffer(uint8_t* tx_data, uint8_t* rx_data, uint16_t len) {
    for(uint16_t i = 0; i < len; i++) {
        uint8_t tx_byte = (tx_data != NULL) ? tx_data[i] : 0x00;
        uint8_t rx_byte = SPI_Transfer(tx_byte);
        
        if(rx_data != NULL) {
            rx_data[i] = rx_byte;
        }
    }
}

/**
 * @brief ส่งข้อมูลอย่างเดียว
 */
void SPI_Write(uint8_t* data, uint16_t len) {
    SPI_TransferBuffer(data, NULL, len);
}

/**
 * @brief รับข้อมูลอย่างเดียว
 */
void SPI_Read(uint8_t* data, uint16_t len, uint8_t dummy_byte) {
    for(uint16_t i = 0; i < len; i++) {
        data[i] = SPI_Transfer(dummy_byte);
    }
}

/**
 * @brief ควบคุม CS (Chip Select) pin
 */
void SPI_SetCS(uint8_t state) {
    if(state) {
        GPIO_SetBits(cs_port, cs_pin);    // CS = HIGH (inactive)
    } else {
        GPIO_ResetBits(cs_port, cs_pin);  // CS = LOW (active)
    }
}

/**
 * @brief ตั้งค่า bit order
 */
void SPI_SetBitOrder(SPI_BitOrder order) {
    SPI_Cmd(SPI1, DISABLE);
    
    if(order == SPI_LSB_FIRST) {
        SPI1->CTLR1 |= SPI_FirstBit_LSB;
    } else {
        SPI1->CTLR1 &= ~SPI_FirstBit_LSB;
    }
    
    SPI_Cmd(SPI1, ENABLE);
}

/**
 * @brief เปลี่ยนความเร็ว SPI
 */
void SPI_SetSpeed(SPI_Speed speed) {
    SPI_Cmd(SPI1, DISABLE);
    
    // Clear prescaler bits
    SPI1->CTLR1 &= ~(0x07 << 3);
    // Set new prescaler
    SPI1->CTLR1 |= (speed << 3);
    
    SPI_Cmd(SPI1, ENABLE);
}
