/**
 * @file SimpleHAL_Examples.c
 * @brief ตัวอย่างการใช้งาน Simple HAL Library
 * @version 1.0
 * @date 2025-12-12
 * 
 * @details
 * ไฟล์นี้แสดงตัวอย่างการใช้งาน SimpleHAL Library ทั้งหมด
 * - SimpleUSART: การสื่อสาร serial
 * - SimpleI2C: การใช้งานกับ EEPROM และ sensors
 * - SimpleSPI: การใช้งานกับ SD card และ sensors
 */

#include "../SimpleHAL.h"

/* ========== ตัวอย่างที่ 1: SimpleUSART ========== */

/**
 * @brief ตัวอย่างการใช้งาน USART พื้นฐาน
 */
void Example_USART_Basic(void) {
    // เริ่มต้น USART ที่ 115200 baud, default pins
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    // ส่งข้อความ
    USART_Print("=== SimpleUSART Example ===\r\n");
    USART_Print("Hello from CH32V003!\r\n");
    
    // ส่งตัวเลข
    USART_Print("Number: ");
    USART_PrintNum(12345);
    USART_Print("\r\n");
    
    // ส่ง hex
    USART_Print("Hex: ");
    USART_PrintHex(0xDEADBEEF, 1);
    USART_Print("\r\n");
}

/**
 * @brief ตัวอย่างการอ่านข้อมูลจาก USART
 */
void Example_USART_Read(void) {
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    USART_Print("Type something: ");
    
    while(1) {
        if(USART_Available()) {
            uint8_t data = USART_Read();
            
            // Echo กลับ
            USART_WriteByte(data);
            
            if(data == '\r') {
                USART_Print("\r\nReceived!\r\n");
                break;
            }
        }
    }
}

/**
 * @brief ตัวอย่างการใช้ pin remap
 */
void Example_USART_Remap(void) {
    // ใช้ remap1: TX=PD0, RX=PD1
    USART_SimpleInit(BAUD_115200, USART_PINS_REMAP1);
    
    USART_Print("Using remapped pins!\r\n");
}

/* ========== ตัวอย่างที่ 2: SimpleI2C ========== */

/**
 * @brief ตัวอย่างการใช้งาน I2C กับ EEPROM 24C02
 */
void Example_I2C_EEPROM(void) {
    #define EEPROM_ADDR 0x50  // 24C02 address (7-bit)
    
    // เริ่มต้น I2C ที่ 100kHz
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
    
    USART_Print("=== I2C EEPROM Example ===\r\n");
    
    // เขียนข้อมูล
    uint8_t write_data = 0x55;
    if(I2C_WriteReg(EEPROM_ADDR, 0x00, write_data) == I2C_OK) {
        USART_Print("Write OK\r\n");
    }
    
    Delay_Ms(5);  // EEPROM write cycle time
    
    // อ่านข้อมูล
    uint8_t read_data = I2C_ReadReg(EEPROM_ADDR, 0x00);
    
    USART_Print("Read: 0x");
    USART_PrintHex(read_data, 1);
    USART_Print("\r\n");
    
    if(read_data == write_data) {
        USART_Print("Verify OK!\r\n");
    }
}

/**
 * @brief ตัวอย่างการ scan I2C devices
 */
void Example_I2C_Scan(void) {
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
    
    USART_Print("=== I2C Scanner ===\r\n");
    USART_Print("Scanning...\r\n\r\n");
    
    uint8_t devices[20];
    uint8_t count = I2C_Scan(devices, 20);
    
    USART_Print("Found ");
    USART_PrintNum(count);
    USART_Print(" device(s):\r\n");
    
    for(uint8_t i = 0; i < count; i++) {
        USART_Print("  0x");
        USART_PrintHex(devices[i], 1);
        USART_Print("\r\n");
    }
}

/**
 * @brief ตัวอย่างการเขียน/อ่านหลาย bytes
 */
void Example_I2C_MultiBytes(void) {
    #define EEPROM_ADDR 0x50
    
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
    
    // เขียนหลาย bytes
    uint8_t write_buf[] = {0x11, 0x22, 0x33, 0x44};
    I2C_WriteRegMulti(EEPROM_ADDR, 0x00, write_buf, 4);
    Delay_Ms(5);
    
    // อ่านหลาย bytes
    uint8_t read_buf[4];
    I2C_ReadRegMulti(EEPROM_ADDR, 0x00, read_buf, 4);
    
    USART_Print("Read data: ");
    for(int i = 0; i < 4; i++) {
        USART_PrintHex(read_buf[i], 1);
        USART_Print(" ");
    }
    USART_Print("\r\n");
}

/* ========== ตัวอย่างที่ 3: SimpleSPI ========== */

/**
 * @brief ตัวอย่างการใช้งาน SPI พื้นฐาน
 */
void Example_SPI_Basic(void) {
    // เริ่มต้น SPI mode 0, 1MHz
    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
    
    USART_Print("=== SimpleSPI Example ===\r\n");
    
    // ส่ง/รับข้อมูล
    SPI_SetCS(0);  // เลือก device (CS = LOW)
    
    uint8_t tx = 0xAA;
    uint8_t rx = SPI_Transfer(tx);
    
    SPI_SetCS(1);  // ยกเลิกการเลือก (CS = HIGH)
    
    USART_Print("TX: 0x");
    USART_PrintHex(tx, 1);
    USART_Print(", RX: 0x");
    USART_PrintHex(rx, 1);
    USART_Print("\r\n");
}

/**
 * @brief ตัวอย่างการใช้งาน SPI buffer transfer
 */
void Example_SPI_Buffer(void) {
    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
    
    uint8_t tx_buf[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t rx_buf[4];
    
    SPI_SetCS(0);
    SPI_TransferBuffer(tx_buf, rx_buf, 4);
    SPI_SetCS(1);
    
    USART_Print("RX Buffer: ");
    for(int i = 0; i < 4; i++) {
        USART_PrintHex(rx_buf[i], 1);
        USART_Print(" ");
    }
    USART_Print("\r\n");
}

/**
 * @brief ตัวอย่างการเปลี่ยน SPI mode
 */
void Example_SPI_Modes(void) {
    // ทดสอบ SPI modes ต่างๆ
    const char* mode_names[] = {"Mode 0", "Mode 1", "Mode 2", "Mode 3"};
    
    for(int mode = 0; mode < 4; mode++) {
        SPI_SimpleInit((SPI_Mode)mode, SPI_1MHZ, SPI_PINS_DEFAULT);
        
        USART_Print("Testing ");
        USART_Print(mode_names[mode]);
        USART_Print("\r\n");
        
        SPI_SetCS(0);
        uint8_t rx = SPI_Transfer(0x55);
        SPI_SetCS(1);
        
        Delay_Ms(100);
    }
}

/* ========== ตัวอย่างที่ 4: ใช้งานรวมกัน ========== */

/**
 * @brief ตัวอย่างการใช้งาน USART + I2C + SPI พร้อมกัน
 */
void Example_Combined(void) {
    // เริ่มต้นทั้งหมด
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    I2C_SimpleInit(I2C_100KHZ, I2C_PINS_DEFAULT);
    SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
    
    USART_Print("\r\n=== Combined Example ===\r\n");
    
    // ใช้ I2C
    USART_Print("I2C: ");
    if(I2C_IsDeviceReady(0x50)) {
        USART_Print("EEPROM found!\r\n");
    } else {
        USART_Print("EEPROM not found\r\n");
    }
    
    // ใช้ SPI
    USART_Print("SPI: Sending data...\r\n");
    SPI_SetCS(0);
    SPI_Transfer(0xAA);
    SPI_SetCS(1);
    
    USART_Print("Done!\r\n");
}

/* ========== Main Function ========== */

/**
 * @brief ฟังก์ชัน main แสดงตัวอย่างทั้งหมด
 */
void SimpleHAL_Examples_Main(void) {
    // เริ่มต้นระบบ
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เลือกตัวอย่างที่ต้องการทดสอบ
    
    // USART Examples
    Example_USART_Basic();
    Delay_Ms(1000);
    
    // I2C Examples
    Example_I2C_Scan();
    Delay_Ms(1000);
    
    Example_I2C_EEPROM();
    Delay_Ms(1000);
    
    // SPI Examples
    Example_SPI_Basic();
    Delay_Ms(1000);
    
    // Combined
    Example_Combined();
    
    while(1) {
        // Main loop
        Delay_Ms(1000);
    }
}
