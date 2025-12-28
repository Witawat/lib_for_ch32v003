/**
 * @file SimpleI2C_Soft.c
 * @brief Software I2C (Bit-bang) Implementation
 */

#include "SimpleI2C_Soft.h"
#include "SimpleDelay.h"

/* ลำดับ Pin ส่วนตัว */
static uint8_t _SCL_PIN;
static uint8_t _SDA_PIN;
static uint16_t _DELAY_US;

/* Helper Macros */
#define SCL_H()   digitalWrite(_SCL_PIN, HIGH)
#define SCL_L()   digitalWrite(_SCL_PIN, LOW)
#define SDA_H()   digitalWrite(_SDA_PIN, HIGH)
#define SDA_L()   digitalWrite(_SDA_PIN, LOW)
#define SDA_READ() digitalRead(_SDA_PIN)

/**
 * @brief เริ่มต้น Software I2C
 */
void I2C_Soft_Init(uint8_t scl_pin, uint8_t sda_pin, I2C_Soft_Speed speed) {
    _SCL_PIN = scl_pin;
    _SDA_PIN = sda_pin;
    
    // ตั้งค่า delay ตามความเร็วมาตรฐาน
    if (speed == I2C_SOFT_400KHZ) {
        _DELAY_US = 1; // ประมาณ 400kHz+ (ขึ้นกับ loop overhead)
    } else {
        _DELAY_US = 5; // ประมาณ 100kHz
    }

    // ตั้งค่าเป็น Open-Drain Output
    pinMode(_SCL_PIN, PIN_MODE_OUTPUT_OD);
    pinMode(_SDA_PIN, PIN_MODE_OUTPUT_OD);

    // Idle state
    SCL_H();
    SDA_H();
    Delay_Us(_DELAY_US);
}

/**
 * @brief สัญญาณ Start
 */
void I2C_Soft_Start(void) {
    SDA_H();
    SCL_H();
    Delay_Us(_DELAY_US);
    SDA_L();
    Delay_Us(_DELAY_US);
    SCL_L();
    Delay_Us(_DELAY_US);
}

/**
 * @brief สัญญาณ Stop
 */
void I2C_Soft_Stop(void) {
    SDA_L();
    SCL_H();
    Delay_Us(_DELAY_US);
    SDA_H();
    Delay_Us(_DELAY_US);
}

/**
 * @brief เขียน 1 Byte
 * @return 0 = ACK, 1 = NACK
 */
uint8_t I2C_Soft_WriteByte(uint8_t byte) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (byte & 0x80) SDA_H();
        else SDA_L();
        byte <<= 1;
        Delay_Us(_DELAY_US);
        SCL_H();
        Delay_Us(_DELAY_US);
        SCL_L();
    }
    
    // Check ACK
    SDA_H(); // Release SDA
    Delay_Us(_DELAY_US);
    SCL_H();
    Delay_Us(_DELAY_US);
    uint8_t ack = SDA_READ();
    SCL_L();
    
    return ack;
}

/**
 * @brief อ่าน 1 Byte
 */
uint8_t I2C_Soft_ReadByte(uint8_t ack) {
    uint8_t i, byte = 0;
    SDA_H(); // Ensure SDA is high (input)
    for (i = 0; i < 8; i++) {
        byte <<= 1;
        SCL_H();
        Delay_Us(_DELAY_US);
        if (SDA_READ()) byte |= 0x01;
        SCL_L();
        Delay_Us(_DELAY_US);
    }
    
    // Send ACK/NACK
    if (ack) SDA_L();
    else SDA_H();
    SCL_H();
    Delay_Us(_DELAY_US);
    SCL_L();
    SDA_H();
    
    return byte;
}

/**
 * @brief เขียนข้อมูล (ข้าม ACK ได้)
 */
I2C_Soft_Status I2C_Soft_Write(uint8_t addr, uint8_t* data, uint16_t len, uint8_t ignore_ack) {
    I2C_Soft_Start();
    
    // Send Address + W
    if (I2C_Soft_WriteByte(addr << 1) && !ignore_ack) {
        I2C_Soft_Stop();
        return I2C_SOFT_ERROR_NACK;
    }
    
    for (uint16_t i = 0; i < len; i++) {
        if (I2C_Soft_WriteByte(data[i]) && !ignore_ack) {
            I2C_Soft_Stop();
            return I2C_SOFT_ERROR_NACK;
        }
    }
    
    I2C_Soft_Stop();
    return I2C_SOFT_OK;
}

/**
 * @brief อ่านข้อมูล
 */
I2C_Soft_Status I2C_Soft_Read(uint8_t addr, uint8_t* data, uint16_t len) {
    I2C_Soft_Start();
    
    // Send Address + R
    if (I2C_Soft_WriteByte((addr << 1) | 0x01)) {
        I2C_Soft_Stop();
        return I2C_SOFT_ERROR_NACK;
    }
    
    for (uint16_t i = 0; i < len; i++) {
        data[i] = I2C_Soft_ReadByte(i < (len - 1));
    }
    
    I2C_Soft_Stop();
    return I2C_SOFT_OK;
}
