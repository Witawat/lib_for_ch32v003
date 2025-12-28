/**
 * @file Simple1Wire.c
 * @brief Simple 1-Wire Protocol Library Implementation
 * @version 1.0
 * @date 2025-12-22
 */

#include "Simple1Wire.h"
#include <string.h>

/* ========== Private Variables ========== */

static OneWire_Bus onewire_buses[ONEWIRE_MAX_BUSES];
static uint8_t onewire_bus_count = 0;

/* ========== Private Function Prototypes ========== */

static bool OneWire_SearchInternal(OneWire_Bus* bus, uint8_t command);
static void OneWire_DisableInterrupts(void);
static void OneWire_EnableInterrupts(void);

/* ========== Initialization ========== */

/**
 * @brief เริ่มต้น 1-Wire bus บน GPIO pin
 */
OneWire_Bus* OneWire_Init(uint8_t pin) {
    // ตรวจสอบว่าเต็มหรือไม่
    if (onewire_bus_count >= ONEWIRE_MAX_BUSES) {
        return NULL;
    }
    
    // ตรวจสอบว่า pin นี้ถูกใช้แล้วหรือไม่
    for (uint8_t i = 0; i < onewire_bus_count; i++) {
        if (onewire_buses[i].pin == pin) {
            return &onewire_buses[i];  // Return existing instance
        }
    }
    
    // สร้าง instance ใหม่
    OneWire_Bus* bus = &onewire_buses[onewire_bus_count++];
    
    // Initialize structure
    memset(bus, 0, sizeof(OneWire_Bus));
    bus->pin = pin;
    bus->initialized = true;
    
    // ตั้งค่า GPIO เป็น input (pull-up ภายนอก)
    pinMode(pin, PIN_MODE_INPUT);
    
    return bus;
}

/* ========== Low-Level Protocol Functions ========== */

/**
 * @brief ส่ง reset pulse และตรวจสอบ presence pulse
 */
bool OneWire_Reset(OneWire_Bus* bus) {
    if (!bus || !bus->initialized) return false;
    
    bool presence;
    
    OneWire_DisableInterrupts();
    
    // ส่ง reset pulse (480 µs low)
    pinMode(bus->pin, PIN_MODE_OUTPUT);
    digitalWrite(bus->pin, LOW);
    Delay_Us(ONEWIRE_RESET_PULSE);
    
    // Release bus (pull-up จะดึงขึ้น)
    pinMode(bus->pin, PIN_MODE_INPUT);
    Delay_Us(ONEWIRE_PRESENCE_WAIT);
    
    // อ่าน presence pulse (device จะดึงลง)
    presence = !digitalRead(bus->pin);
    
    OneWire_EnableInterrupts();
    
    // รอให้ presence pulse จบ
    Delay_Us(ONEWIRE_PRESENCE_TIMEOUT);
    
    return presence;
}

/**
 * @brief เขียน 1 bit ไปยัง 1-Wire bus
 */
void OneWire_WriteBit(OneWire_Bus* bus, uint8_t bit) {
    if (!bus || !bus->initialized) return;
    
    OneWire_DisableInterrupts();
    
    if (bit & 1) {
        // Write 1: ดึงลง 10 µs แล้วปล่อย
        pinMode(bus->pin, PIN_MODE_OUTPUT);
        digitalWrite(bus->pin, LOW);
        Delay_Us(ONEWIRE_WRITE_1_LOW);
        pinMode(bus->pin, PIN_MODE_INPUT);
        Delay_Us(ONEWIRE_SLOT_TIME - ONEWIRE_WRITE_1_LOW);
    } else {
        // Write 0: ดึงลง 60 µs แล้วปล่อย
        pinMode(bus->pin, PIN_MODE_OUTPUT);
        digitalWrite(bus->pin, LOW);
        Delay_Us(ONEWIRE_WRITE_0_LOW);
        pinMode(bus->pin, PIN_MODE_INPUT);
        Delay_Us(ONEWIRE_SLOT_TIME - ONEWIRE_WRITE_0_LOW);
    }
    
    OneWire_EnableInterrupts();
    
    // Recovery time
    Delay_Us(ONEWIRE_WRITE_RECOVERY);
}

/**
 * @brief อ่าน 1 bit จาก 1-Wire bus
 */
uint8_t OneWire_ReadBit(OneWire_Bus* bus) {
    if (!bus || !bus->initialized) return 0;
    
    uint8_t bit;
    
    OneWire_DisableInterrupts();
    
    // ดึงลง 3 µs เพื่อเริ่ม read slot
    pinMode(bus->pin, PIN_MODE_OUTPUT);
    digitalWrite(bus->pin, LOW);
    Delay_Us(ONEWIRE_READ_LOW);
    
    // ปล่อยและรอ 10 µs ก่อนอ่าน
    pinMode(bus->pin, PIN_MODE_INPUT);
    Delay_Us(ONEWIRE_READ_WAIT);
    
    // อ่านค่า
    bit = digitalRead(bus->pin);
    
    OneWire_EnableInterrupts();
    
    // รอให้ slot จบ
    Delay_Us(ONEWIRE_READ_RECOVERY);
    
    return bit;
}

/**
 * @brief เขียน 1 byte ไปยัง 1-Wire bus
 */
void OneWire_WriteByte(OneWire_Bus* bus, uint8_t data) {
    if (!bus || !bus->initialized) return;
    
    // เขียนทีละ bit (LSB first)
    for (uint8_t i = 0; i < 8; i++) {
        OneWire_WriteBit(bus, data & 0x01);
        data >>= 1;
    }
}

/**
 * @brief อ่าน 1 byte จาก 1-Wire bus
 */
uint8_t OneWire_ReadByte(OneWire_Bus* bus) {
    if (!bus || !bus->initialized) return 0;
    
    uint8_t data = 0;
    
    // อ่านทีละ bit (LSB first)
    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;
        if (OneWire_ReadBit(bus)) {
            data |= 0x80;
        }
    }
    
    return data;
}

/**
 * @brief เขียนหลาย bytes ไปยัง 1-Wire bus
 */
void OneWire_WriteBytes(OneWire_Bus* bus, const uint8_t* data, uint8_t len) {
    if (!bus || !bus->initialized || !data) return;
    
    for (uint8_t i = 0; i < len; i++) {
        OneWire_WriteByte(bus, data[i]);
    }
}

/**
 * @brief อ่านหลาย bytes จาก 1-Wire bus
 */
void OneWire_ReadBytes(OneWire_Bus* bus, uint8_t* buffer, uint8_t len) {
    if (!bus || !bus->initialized || !buffer) return;
    
    for (uint8_t i = 0; i < len; i++) {
        buffer[i] = OneWire_ReadByte(bus);
    }
}

/* ========== ROM Command Functions ========== */

/**
 * @brief ส่ง Skip ROM command (0xCC)
 */
void OneWire_SkipROM(OneWire_Bus* bus) {
    if (!bus || !bus->initialized) return;
    OneWire_WriteByte(bus, ONEWIRE_CMD_SKIP_ROM);
}

/**
 * @brief ส่ง Read ROM command (0x33) และอ่าน ROM address
 */
bool OneWire_ReadROM(OneWire_Bus* bus, uint8_t* rom) {
    if (!bus || !bus->initialized || !rom) return false;
    
    // Reset และตรวจสอบ presence
    if (!OneWire_Reset(bus)) {
        return false;
    }
    
    // ส่ง Read ROM command
    OneWire_WriteByte(bus, ONEWIRE_CMD_READ_ROM);
    
    // อ่าน ROM address (8 bytes)
    OneWire_ReadBytes(bus, rom, 8);
    
    // ตรวจสอบ CRC
    return OneWire_VerifyCRC(rom, 8);
}

/**
 * @brief ส่ง Match ROM command (0x55) และเลือก device
 */
void OneWire_MatchROM(OneWire_Bus* bus, const uint8_t* rom) {
    if (!bus || !bus->initialized || !rom) return;
    
    // ส่ง Match ROM command
    OneWire_WriteByte(bus, ONEWIRE_CMD_MATCH_ROM);
    
    // ส่ง ROM address (8 bytes)
    OneWire_WriteBytes(bus, rom, 8);
}

/**
 * @brief เลือก device ด้วย ROM address (รวม Reset + Match ROM)
 */
bool OneWire_Select(OneWire_Bus* bus, const uint8_t* rom) {
    if (!bus || !bus->initialized || !rom) return false;
    
    // Reset และตรวจสอบ presence
    if (!OneWire_Reset(bus)) {
        return false;
    }
    
    // Match ROM
    OneWire_MatchROM(bus, rom);
    
    return true;
}

/* ========== Search Functions ========== */

/**
 * @brief รีเซ็ต search state
 */
void OneWire_ResetSearch(OneWire_Bus* bus) {
    if (!bus || !bus->initialized) return;
    
    bus->last_discrepancy = 0;
    bus->last_family_discrepancy = 0;
    bus->last_device_flag = false;
    memset(bus->rom, 0, 8);
}

/**
 * @brief ค้นหา device ถัดไปบน bus
 */
bool OneWire_Search(OneWire_Bus* bus) {
    return OneWire_SearchInternal(bus, ONEWIRE_CMD_SEARCH_ROM);
}

/**
 * @brief ค้นหา devices ที่มี alarm condition
 */
bool OneWire_AlarmSearch(OneWire_Bus* bus) {
    return OneWire_SearchInternal(bus, ONEWIRE_CMD_ALARM_SEARCH);
}

/**
 * @brief ดึง ROM address ที่พบจาก search
 */
void OneWire_GetAddress(OneWire_Bus* bus, uint8_t* rom) {
    if (!bus || !bus->initialized || !rom) return;
    memcpy(rom, bus->rom, 8);
}

/* ========== CRC Functions ========== */

/**
 * @brief คำนวณ CRC8 (Dallas/Maxim)
 * @note Polynomial: x^8 + x^5 + x^4 + 1 (0x8C)
 */
uint8_t OneWire_CRC8(const uint8_t* data, uint8_t len) {
    if (!data) return 0;
    
    uint8_t crc = 0;
    
    for (uint8_t i = 0; i < len; i++) {
        uint8_t inbyte = data[i];
        for (uint8_t j = 0; j < 8; j++) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) {
                crc ^= 0x8C;
            }
            inbyte >>= 1;
        }
    }
    
    return crc;
}

/**
 * @brief ตรวจสอบ CRC8
 */
bool OneWire_VerifyCRC(const uint8_t* data, uint8_t len) {
    if (!data || len == 0) return false;
    
    // คำนวณ CRC ของข้อมูลทั้งหมด (รวม CRC byte)
    // ถ้าถูกต้อง ผลลัพธ์ต้องเป็น 0
    return (OneWire_CRC8(data, len) == 0);
}

/* ========== Utility Functions ========== */

/**
 * @brief ปิด strong pull-up
 */
void OneWire_Depower(OneWire_Bus* bus) {
    if (!bus || !bus->initialized) return;
    pinMode(bus->pin, PIN_MODE_INPUT);
}

/**
 * @brief หา OneWire_Bus instance จาก pin number
 */
OneWire_Bus* OneWire_GetBusByPin(uint8_t pin) {
    for (uint8_t i = 0; i < onewire_bus_count; i++) {
        if (onewire_buses[i].pin == pin) {
            return &onewire_buses[i];
        }
    }
    return NULL;
}

/* ========== Private Functions ========== */

/**
 * @brief ROM Search Algorithm Implementation
 * @note Based on Maxim Application Note 187
 */
static bool OneWire_SearchInternal(OneWire_Bus* bus, uint8_t command) {
    if (!bus || !bus->initialized) return false;
    
    // ถ้าพบ device สุดท้ายแล้ว
    if (bus->last_device_flag) {
        return false;
    }
    
    // Reset และตรวจสอบ presence
    if (!OneWire_Reset(bus)) {
        OneWire_ResetSearch(bus);
        return false;
    }
    
    // ส่ง search command
    OneWire_WriteByte(bus, command);
    
    uint8_t id_bit_number = 1;
    uint8_t last_zero = 0;
    uint8_t rom_byte_number = 0;
    uint8_t rom_byte_mask = 1;
    bool search_result = false;
    
    // Loop through all 64 bits
    while (rom_byte_number < 8) {
        // อ่าน bit และ complement bit
        uint8_t id_bit = OneWire_ReadBit(bus);
        uint8_t cmp_id_bit = OneWire_ReadBit(bus);
        
        // ตรวจสอบว่ามี device หรือไม่
        if (id_bit && cmp_id_bit) {
            // ไม่มี device ตอบกลับ
            break;
        }
        
        uint8_t search_direction;
        
        if (id_bit != cmp_id_bit) {
            // ทุก devices มีค่า bit เหมือนกัน
            search_direction = id_bit;
        } else {
            // มี discrepancy (devices มีค่า bit ต่างกัน)
            if (id_bit_number < bus->last_discrepancy) {
                // ใช้ค่าเดิมจาก ROM
                search_direction = ((bus->rom[rom_byte_number] & rom_byte_mask) > 0);
            } else {
                // เลือก 1 ถ้าเป็น last_discrepancy, ไม่งั้นเลือก 0
                search_direction = (id_bit_number == bus->last_discrepancy);
            }
            
            // บันทึก last zero position
            if (search_direction == 0) {
                last_zero = id_bit_number;
                
                // Check for last family discrepancy
                if (last_zero < 9) {
                    bus->last_family_discrepancy = last_zero;
                }
            }
        }
        
        // บันทึก bit ใน ROM
        if (search_direction) {
            bus->rom[rom_byte_number] |= rom_byte_mask;
        } else {
            bus->rom[rom_byte_number] &= ~rom_byte_mask;
        }
        
        // ส่ง search direction bit
        OneWire_WriteBit(bus, search_direction);
        
        // เพิ่ม bit counter
        id_bit_number++;
        rom_byte_mask <<= 1;
        
        // ถ้าครบ byte แล้ว
        if (rom_byte_mask == 0) {
            rom_byte_number++;
            rom_byte_mask = 1;
        }
    }
    
    // ตรวจสอบว่า search สำเร็จหรือไม่
    if (id_bit_number >= 65) {
        // ตรวจสอบ CRC
        if (OneWire_VerifyCRC(bus->rom, 8)) {
            // อัพเดท search state
            bus->last_discrepancy = last_zero;
            
            // ตรวจสอบว่าเป็น device สุดท้ายหรือไม่
            if (bus->last_discrepancy == 0) {
                bus->last_device_flag = true;
            }
            
            search_result = true;
        }
    }
    
    // ถ้าไม่พบ device
    if (!search_result || !bus->rom[0]) {
        OneWire_ResetSearch(bus);
        search_result = false;
    }
    
    return search_result;
}

/**
 * @brief ปิด interrupts (critical section)
 */
static void OneWire_DisableInterrupts(void) {
    __disable_irq();
}

/**
 * @brief เปิด interrupts
 */
static void OneWire_EnableInterrupts(void) {
    __enable_irq();
}
