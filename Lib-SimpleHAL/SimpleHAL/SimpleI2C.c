/**
 * @file SimpleI2C.c
 * @brief Simple I2C Library Implementation
 * @version 1.0
 * @date 2025-12-12
 */

#include "SimpleI2C.h"
#include "SimpleDelay.h"

/* ========== Private Helper Functions ========== */

/**
 * @brief รอ event flag พร้อม timeout
 */
static I2C_Status I2C_WaitEvent(uint32_t event, uint32_t timeout_ms) {
    uint32_t timeout = timeout_ms * 1000;  // แปลงเป็น microseconds
    
    while(!I2C_CheckEvent(I2C1, event)) {
        if(timeout-- == 0) {
            return I2C_ERROR_TIMEOUT;
        }
        Delay_Us(1);
    }
    return I2C_OK;
}

/* ========== Public Functions ========== */

/**
 * @brief เริ่มต้นการใช้งาน I2C
 */
void I2C_SimpleInit(I2C_Speed speed, I2C_PinConfig pin_config) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef I2C_InitStructure = {0};
    
    // 1. เปิด Clock
    if(pin_config == I2C_PINS_DEFAULT) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB1Periph_I2C1, ENABLE);
    } else {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB1Periph_I2C1, ENABLE);
    }
    
    // 2. ตั้งค่า Pin Remapping และ GPIO
    switch(pin_config) {
        case I2C_PINS_DEFAULT:
            // Default: SCL=PC2, SDA=PC1
            
            // SCL และ SDA - Alternate Function Open-Drain
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_1;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            break;
            
        case I2C_PINS_REMAP:
            // Remap: SCL=PD0, SDA=PD1
            GPIO_PinRemapConfig(GPIO_FullRemap_I2C1, ENABLE);
            
            // SCL และ SDA
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            break;
    }
    
    // 3. ตั้งค่า I2C
    I2C_InitStructure.I2C_ClockSpeed = speed;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;  // Master mode
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    
    I2C_Init(I2C1, &I2C_InitStructure);
    
    // 4. เปิดใช้งาน I2C
    I2C_Cmd(I2C1, ENABLE);
}

/**
 * @brief เขียนข้อมูลไปยัง I2C device
 */
I2C_Status I2C_Write(uint8_t addr, uint8_t* data, uint16_t len) {
    I2C_Status status;
    
    // 1. ส่ง START condition
    I2C_GenerateSTART(I2C1, ENABLE);
    status = I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, I2C_TIMEOUT_MS);
    if(status != I2C_OK) return status;
    
    // 2. ส่ง address + Write bit
    I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
    status = I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_TIMEOUT_MS);
    if(status != I2C_OK) return status;
    
    // 3. ส่งข้อมูล
    for(uint16_t i = 0; i < len; i++) {
        I2C_SendData(I2C1, data[i]);
        status = I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_TIMEOUT_MS);
        if(status != I2C_OK) return status;
    }
    
    // 4. ส่ง STOP condition
    I2C_GenerateSTOP(I2C1, ENABLE);
    
    return I2C_OK;
}

/**
 * @brief อ่านข้อมูลจาก I2C device
 */
I2C_Status I2C_Read(uint8_t addr, uint8_t* data, uint16_t len) {
    I2C_Status status;
    
    // 1. ส่ง START condition
    I2C_GenerateSTART(I2C1, ENABLE);
    status = I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, I2C_TIMEOUT_MS);
    if(status != I2C_OK) return status;
    
    // 2. ส่ง address + Read bit
    I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Receiver);
    status = I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, I2C_TIMEOUT_MS);
    if(status != I2C_OK) return status;
    
    // 3. อ่านข้อมูล
    for(uint16_t i = 0; i < len; i++) {
        if(i == len - 1) {
            // Byte สุดท้าย: ส่ง NACK
            I2C_AcknowledgeConfig(I2C1, DISABLE);
        }
        
        status = I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED, I2C_TIMEOUT_MS);
        if(status != I2C_OK) return status;
        
        data[i] = I2C_ReceiveData(I2C1);
    }
    
    // 4. ส่ง STOP condition
    I2C_GenerateSTOP(I2C1, ENABLE);
    
    // 5. เปิด ACK กลับ
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    
    return I2C_OK;
}

/**
 * @brief เขียนข้อมูลไปยัง register ของ device
 */
I2C_Status I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};
    return I2C_Write(addr, buffer, 2);
}

/**
 * @brief อ่านข้อมูลจาก register ของ device
 */
uint8_t I2C_ReadReg(uint8_t addr, uint8_t reg) {
    uint8_t data = 0xFF;
    
    // 1. เขียน register address
    if(I2C_Write(addr, &reg, 1) != I2C_OK) {
        return 0xFF;
    }
    
    // 2. อ่านข้อมูล
    if(I2C_Read(addr, &data, 1) != I2C_OK) {
        return 0xFF;
    }
    
    return data;
}

/**
 * @brief เขียนหลาย bytes ไปยัง register
 */
I2C_Status I2C_WriteRegMulti(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len) {
    I2C_Status status;
    
    // 1. ส่ง START condition
    I2C_GenerateSTART(I2C1, ENABLE);
    status = I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, I2C_TIMEOUT_MS);
    if(status != I2C_OK) return status;
    
    // 2. ส่ง address + Write bit
    I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
    status = I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_TIMEOUT_MS);
    if(status != I2C_OK) return status;
    
    // 3. ส่ง register address
    I2C_SendData(I2C1, reg);
    status = I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_TIMEOUT_MS);
    if(status != I2C_OK) return status;
    
    // 4. ส่งข้อมูล
    for(uint16_t i = 0; i < len; i++) {
        I2C_SendData(I2C1, data[i]);
        status = I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_TIMEOUT_MS);
        if(status != I2C_OK) return status;
    }
    
    // 5. ส่ง STOP condition
    I2C_GenerateSTOP(I2C1, ENABLE);
    
    return I2C_OK;
}

/**
 * @brief อ่านหลาย bytes จาก register
 */
I2C_Status I2C_ReadRegMulti(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len) {
    // 1. เขียน register address
    I2C_Status status = I2C_Write(addr, &reg, 1);
    if(status != I2C_OK) return status;
    
    // 2. อ่านข้อมูล
    return I2C_Read(addr, data, len);
}

/**
 * @brief สแกนหา I2C devices บน bus
 */
uint8_t I2C_Scan(uint8_t* found_devices, uint8_t max_devices) {
    uint8_t count = 0;
    
    for(uint8_t addr = 0x08; addr < 0x78; addr++) {
        if(I2C_IsDeviceReady(addr)) {
            if(count < max_devices) {
                found_devices[count++] = addr;
            }
        }
    }
    
    return count;
}

/**
 * @brief ตรวจสอบว่า device ตอบสนองหรือไม่
 */
uint8_t I2C_IsDeviceReady(uint8_t addr) {
    I2C_Status status;
    
    // ส่ง START condition
    I2C_GenerateSTART(I2C1, ENABLE);
    status = I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, I2C_TIMEOUT_MS);
    if(status != I2C_OK) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return 0;
    }
    
    // ส่ง address
    I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
    status = I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_TIMEOUT_MS);
    
    // ส่ง STOP condition
    I2C_GenerateSTOP(I2C1, ENABLE);
    
    return (status == I2C_OK) ? 1 : 0;
}
