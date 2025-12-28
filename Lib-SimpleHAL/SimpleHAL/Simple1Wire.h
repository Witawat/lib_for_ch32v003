/**
 * @file Simple1Wire.h
 * @brief Simple 1-Wire Protocol Library สำหรับ CH32V003
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * Library นี้ใช้สำหรับการสื่อสารด้วย 1-Wire protocol
 * รองรับ Dallas/Maxim 1-Wire devices เช่น DS18B20, DS2431, iButton
 * 
 * **คุณสมบัติ:**
 * - 1-Wire protocol implementation (standard timing)
 * - ROM commands (Skip, Read, Match, Search, Alarm Search)
 * - ROM search algorithm สำหรับ multi-device bus
 * - CRC8 calculation และ validation
 * - รองรับทุก GPIO pins ของ CH32V003
 * - Interrupt-safe critical sections
 * 
 * **Hardware Requirements:**
 * - Pull-up resistor 4.7kΩ ระหว่าง data line และ VCC
 * - รองรับทั้ง normal power และ parasite power mode
 * 
 * **1-Wire Protocol:**
 * ```
 * VCC (3.3V)
 *     |
 *   [4.7kΩ]
 *     |
 *     +---- Data Line (GPIO) ----+---- Device 1
 *     |                          |
 *     |                          +---- Device 2
 *     |                          |
 *     |                          +---- Device N
 *    GND
 * ```
 * 
 * @example
 * #include "Simple1Wire.h"
 * 
 * int main(void) {
 *     SystemCoreClockUpdate();
 *     
 *     // เริ่มต้น 1-Wire bus บน PD2
 *     OneWire_Bus* bus = OneWire_Init(PD2);
 *     
 *     // ตรวจสอบว่ามี device หรือไม่
 *     if (OneWire_Reset(bus)) {
 *         // มี device บน bus
 *         OneWire_SkipROM(bus);  // Skip ROM (single device)
 *         OneWire_WriteByte(bus, 0x44);  // Send command
 *     }
 * }
 * 
 * @note ต้องเรียก SystemCoreClockUpdate() ก่อนใช้งาน
 * @note ต้องมี pull-up resistor 4.7kΩ ภายนอก
 */

#ifndef __SIMPLE_1WIRE_H
#define __SIMPLE_1WIRE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include "SimpleGPIO.h"
#include "SimpleDelay.h"
#include <stdint.h>
#include <stdbool.h>

/* ========== Configuration ========== */

#define ONEWIRE_MAX_BUSES  4  /**< จำนวน 1-Wire buses สูงสุด */

/* ========== 1-Wire Timing (microseconds) ========== */

/**
 * @brief 1-Wire Timing Constants (Standard Speed)
 * @note ค่าเหล่านี้ตาม Dallas/Maxim 1-Wire specification
 */
#define ONEWIRE_RESET_PULSE       480   /**< Reset pulse duration (480-960 µs) */
#define ONEWIRE_PRESENCE_WAIT     70    /**< Wait before sampling presence (60-75 µs) */
#define ONEWIRE_PRESENCE_TIMEOUT  240   /**< Presence detect timeout (60-240 µs) */
#define ONEWIRE_WRITE_0_LOW       60    /**< Write 0 low time (60-120 µs) */
#define ONEWIRE_WRITE_1_LOW       10    /**< Write 1 low time (1-15 µs) */
#define ONEWIRE_WRITE_RECOVERY    1     /**< Recovery time after write (1 µs min) */
#define ONEWIRE_READ_LOW          3     /**< Read initiation low time (1-15 µs) */
#define ONEWIRE_READ_WAIT         10    /**< Wait before sampling read (9-15 µs) */
#define ONEWIRE_READ_RECOVERY     55    /**< Recovery time after read (55 µs) */
#define ONEWIRE_SLOT_TIME         65    /**< Total time slot (60-120 µs) */

/* ========== ROM Commands ========== */

/**
 * @brief 1-Wire ROM Commands
 */
#define ONEWIRE_CMD_SKIP_ROM      0xCC  /**< Skip ROM (single device) */
#define ONEWIRE_CMD_READ_ROM      0x33  /**< Read ROM (single device only) */
#define ONEWIRE_CMD_MATCH_ROM     0x55  /**< Match ROM (select specific device) */
#define ONEWIRE_CMD_SEARCH_ROM    0xF0  /**< Search ROM (find all devices) */
#define ONEWIRE_CMD_ALARM_SEARCH  0xEC  /**< Alarm search (find devices with alarm) */

/* ========== Structures ========== */

/**
 * @brief 1-Wire Bus Instance Structure
 */
typedef struct {
    uint8_t pin;                        /**< GPIO pin number */
    uint8_t rom[8];                     /**< Current ROM address (64-bit) */
    uint8_t last_discrepancy;           /**< Last discrepancy position (search state) */
    uint8_t last_family_discrepancy;    /**< Last family discrepancy (search state) */
    bool last_device_flag;              /**< Last device found flag */
    bool initialized;                   /**< Initialization flag */
} OneWire_Bus;

/* ========== Function Prototypes ========== */

/* === Initialization === */

/**
 * @brief เริ่มต้น 1-Wire bus บน GPIO pin
 * @param pin GPIO pin number (PA1-PA2, PC0-PC7, PD2-PD7)
 * @return ตัวชี้ไปยัง OneWire_Bus instance หรือ NULL ถ้าเต็ม
 * 
 * @note ต้องมี pull-up resistor 4.7kΩ ภายนอก
 * 
 * @example
 * OneWire_Bus* bus = OneWire_Init(PD2);
 * if (bus == NULL) {
 *     // Error: ไม่สามารถสร้าง bus ได้
 * }
 */
OneWire_Bus* OneWire_Init(uint8_t pin);

/* === Low-Level Protocol Functions === */

/**
 * @brief ส่ง reset pulse และตรวจสอบ presence pulse
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @return true = มี device ตอบกลับ, false = ไม่มี device
 * 
 * @note ฟังก์ชันนี้ใช้เวลาประมาณ 960 µs
 * 
 * @example
 * if (OneWire_Reset(bus)) {
 *     // มี device บน bus
 * } else {
 *     // ไม่มี device หรือ short circuit
 * }
 */
bool OneWire_Reset(OneWire_Bus* bus);

/**
 * @brief เขียน 1 bit ไปยัง 1-Wire bus
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @param bit ค่า bit ที่ต้องการเขียน (0 หรือ 1)
 * 
 * @note ฟังก์ชันนี้ใช้เวลาประมาณ 65 µs
 * 
 * @example
 * OneWire_WriteBit(bus, 1);  // เขียน bit 1
 * OneWire_WriteBit(bus, 0);  // เขียน bit 0
 */
void OneWire_WriteBit(OneWire_Bus* bus, uint8_t bit);

/**
 * @brief อ่าน 1 bit จาก 1-Wire bus
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @return ค่า bit ที่อ่านได้ (0 หรือ 1)
 * 
 * @note ฟังก์ชันนี้ใช้เวลาประมาณ 65 µs
 * 
 * @example
 * uint8_t bit = OneWire_ReadBit(bus);
 */
uint8_t OneWire_ReadBit(OneWire_Bus* bus);

/**
 * @brief เขียน 1 byte ไปยัง 1-Wire bus
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @param data ข้อมูล 1 byte ที่ต้องการเขียน
 * 
 * @note ฟังก์ชันนี้ใช้เวลาประมาณ 520 µs (8 bits × 65 µs)
 * 
 * @example
 * OneWire_WriteByte(bus, 0x44);  // Send Convert T command
 */
void OneWire_WriteByte(OneWire_Bus* bus, uint8_t data);

/**
 * @brief อ่าน 1 byte จาก 1-Wire bus
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @return ข้อมูล 1 byte ที่อ่านได้
 * 
 * @note ฟังก์ชันนี้ใช้เวลาประมาณ 520 µs (8 bits × 65 µs)
 * 
 * @example
 * uint8_t data = OneWire_ReadByte(bus);
 */
uint8_t OneWire_ReadByte(OneWire_Bus* bus);

/**
 * @brief เขียนหลาย bytes ไปยัง 1-Wire bus
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @param data ตัวชี้ไปยัง buffer ที่เก็บข้อมูล
 * @param len จำนวน bytes ที่ต้องการเขียน
 * 
 * @example
 * uint8_t cmd[] = {0x4E, 0x4B, 0x46, 0x7F};  // Write scratchpad
 * OneWire_WriteBytes(bus, cmd, 4);
 */
void OneWire_WriteBytes(OneWire_Bus* bus, const uint8_t* data, uint8_t len);

/**
 * @brief อ่านหลาย bytes จาก 1-Wire bus
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @param buffer ตัวชี้ไปยัง buffer สำหรับเก็บข้อมูล
 * @param len จำนวน bytes ที่ต้องการอ่าน
 * 
 * @example
 * uint8_t scratchpad[9];
 * OneWire_ReadBytes(bus, scratchpad, 9);
 */
void OneWire_ReadBytes(OneWire_Bus* bus, uint8_t* buffer, uint8_t len);

/* === ROM Command Functions === */

/**
 * @brief ส่ง Skip ROM command (0xCC)
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * 
 * @note ใช้เมื่อมี device เดียวบน bus หรือต้องการส่ง command ไปยังทุก devices
 * 
 * @example
 * OneWire_Reset(bus);
 * OneWire_SkipROM(bus);
 * OneWire_WriteByte(bus, 0x44);  // Convert T (all devices)
 */
void OneWire_SkipROM(OneWire_Bus* bus);

/**
 * @brief ส่ง Read ROM command (0x33) และอ่าน ROM address
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @param rom ตัวชี้ไปยัง buffer 8 bytes สำหรับเก็บ ROM address
 * @return true = สำเร็จ, false = ผิดพลาด
 * 
 * @note ใช้ได้เฉพาะเมื่อมี device เดียวบน bus
 * @note ROM address: [Family Code][Serial Number 6 bytes][CRC]
 * 
 * @example
 * uint8_t rom[8];
 * if (OneWire_ReadROM(bus, rom)) {
 *     printf("Family: 0x%02X\r\n", rom[0]);
 *     printf("Serial: %02X%02X%02X%02X%02X%02X\r\n",
 *            rom[6], rom[5], rom[4], rom[3], rom[2], rom[1]);
 *     printf("CRC: 0x%02X\r\n", rom[7]);
 * }
 */
bool OneWire_ReadROM(OneWire_Bus* bus, uint8_t* rom);

/**
 * @brief ส่ง Match ROM command (0x55) และเลือก device
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @param rom ตัวชี้ไปยัง ROM address 8 bytes
 * 
 * @note ใช้สำหรับเลือก device เฉพาะบน multi-device bus
 * 
 * @example
 * uint8_t rom[8] = {0x28, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE};
 * OneWire_Reset(bus);
 * OneWire_MatchROM(bus, rom);
 * OneWire_WriteByte(bus, 0x44);  // Convert T (selected device only)
 */
void OneWire_MatchROM(OneWire_Bus* bus, const uint8_t* rom);

/**
 * @brief เลือก device ด้วย ROM address (รวม Reset + Match ROM)
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @param rom ตัวชี้ไปยัง ROM address 8 bytes
 * @return true = สำเร็จ, false = ไม่มี device
 * 
 * @note Convenience function ที่รวม Reset และ Match ROM เข้าด้วยกัน
 * 
 * @example
 * uint8_t rom[8] = {...};
 * if (OneWire_Select(bus, rom)) {
 *     OneWire_WriteByte(bus, 0x44);  // Send command
 * }
 */
bool OneWire_Select(OneWire_Bus* bus, const uint8_t* rom);

/* === Search Functions === */

/**
 * @brief รีเซ็ต search state
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * 
 * @note เรียกฟังก์ชันนี้ก่อนเริ่ม search ใหม่
 * 
 * @example
 * OneWire_ResetSearch(bus);
 * while (OneWire_Search(bus)) {
 *     // Process found device
 * }
 */
void OneWire_ResetSearch(OneWire_Bus* bus);

/**
 * @brief ค้นหา device ถัดไปบน bus
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @return true = พบ device, false = ไม่มี device เพิ่มเติม
 * 
 * @note ROM address ที่พบจะถูกเก็บใน bus->rom
 * @note ใช้ร่วมกับ OneWire_ResetSearch() และ OneWire_GetAddress()
 * 
 * @example
 * OneWire_ResetSearch(bus);
 * while (OneWire_Search(bus)) {
 *     uint8_t rom[8];
 *     OneWire_GetAddress(bus, rom);
 *     printf("Found device: %02X...\r\n", rom[0]);
 * }
 */
bool OneWire_Search(OneWire_Bus* bus);

/**
 * @brief ค้นหา devices ที่มี alarm condition
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @return true = พบ device ที่มี alarm, false = ไม่มี
 * 
 * @note ใช้ Alarm Search command (0xEC) แทน Search ROM (0xF0)
 * @note ROM address ที่พบจะถูกเก็บใน bus->rom
 * 
 * @example
 * OneWire_ResetSearch(bus);
 * while (OneWire_AlarmSearch(bus)) {
 *     uint8_t rom[8];
 *     OneWire_GetAddress(bus, rom);
 *     printf("Alarm device: %02X...\r\n", rom[0]);
 * }
 */
bool OneWire_AlarmSearch(OneWire_Bus* bus);

/**
 * @brief ดึง ROM address ที่พบจาก search
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * @param rom ตัวชี้ไปยัง buffer 8 bytes สำหรับเก็บ ROM address
 * 
 * @example
 * if (OneWire_Search(bus)) {
 *     uint8_t rom[8];
 *     OneWire_GetAddress(bus, rom);
 * }
 */
void OneWire_GetAddress(OneWire_Bus* bus, uint8_t* rom);

/* === CRC Functions === */

/**
 * @brief คำนวณ CRC8 (Dallas/Maxim)
 * @param data ตัวชี้ไปยังข้อมูล
 * @param len จำนวน bytes
 * @return ค่า CRC8
 * 
 * @note Polynomial: x^8 + x^5 + x^4 + 1 (0x8C)
 * 
 * @example
 * uint8_t rom[8] = {...};
 * uint8_t crc = OneWire_CRC8(rom, 7);
 * if (crc == rom[7]) {
 *     // CRC ถูกต้อง
 * }
 */
uint8_t OneWire_CRC8(const uint8_t* data, uint8_t len);

/**
 * @brief ตรวจสอบ CRC8
 * @param data ตัวชี้ไปยังข้อมูล (รวม CRC byte ท้ายสุด)
 * @param len จำนวน bytes ทั้งหมด (รวม CRC)
 * @return true = CRC ถูกต้อง, false = CRC ผิด
 * 
 * @note CRC byte ต้องอยู่ท้ายสุดของข้อมูล
 * 
 * @example
 * uint8_t rom[8];
 * OneWire_ReadROM(bus, rom);
 * if (OneWire_VerifyCRC(rom, 8)) {
 *     // ROM address ถูกต้อง
 * } else {
 *     // CRC error
 * }
 */
bool OneWire_VerifyCRC(const uint8_t* data, uint8_t len);

/* === Utility Functions === */

/**
 * @brief ปิด strong pull-up (สำหรับ parasite power)
 * @param bus ตัวชี้ไปยัง OneWire_Bus instance
 * 
 * @note ใช้เมื่อต้องการปิด strong pull-up หลังจาก power-intensive operation
 * 
 * @example
 * // Start conversion with strong pull-up
 * OneWire_Reset(bus);
 * OneWire_SkipROM(bus);
 * OneWire_WriteByte(bus, 0x44);
 * // Enable strong pull-up here (external circuit)
 * Delay_Ms(750);
 * OneWire_Depower(bus);  // Disable strong pull-up
 */
void OneWire_Depower(OneWire_Bus* bus);

/**
 * @brief หา OneWire_Bus instance จาก pin number
 * @param pin GPIO pin number
 * @return ตัวชี้ไปยัง OneWire_Bus instance หรือ NULL ถ้าไม่พบ
 * 
 * @example
 * OneWire_Bus* bus = OneWire_GetBusByPin(PD2);
 */
OneWire_Bus* OneWire_GetBusByPin(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_1WIRE_H
