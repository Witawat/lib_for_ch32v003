/**
 * @file SimpleFlash.h
 * @brief Simple Flash Storage Library สำหรับ CH32V003
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * Library สำหรับจัดเก็บข้อมูล configuration และข้อความใน Flash memory
 * แบบ non-volatile (ข้อมูลไม่หายหลังปิดเครื่อง)
 * 
 * **คุณสมบัติ:**
 * - อ่าน/เขียนข้อมูล byte, half-word (16-bit), word (32-bit)
 * - จัดเก็บ string และ struct
 * - Configuration management พร้อม CRC validation
 * - Wear leveling support
 * - Factory reset capability
 * 
 * **ข้อจำกัดของ Flash Memory:**
 * - CH32V003 มี Flash 16KB (256 pages × 64 bytes/page)
 * - Write endurance: ~10,000-80,000 cycles ต่อ page
 * - ต้อง erase ทั้ง page (64 bytes) ก่อนเขียนข้อมูลใหม่
 * - การเขียนทำได้ครั้งละ 16-bit หรือ 32-bit
 * - สำหรับข้อมูลที่เปลี่ยนบ่อยมาก แนะนำใช้ external EEPROM
 * 
 * **Flash Storage Area:**
 * - ใช้ last 2 pages (254-255) สำหรับเก็บข้อมูล
 * - Address: 0x0803F80 - 0x0803FFF (128 bytes)
 * - Page 254: Configuration storage (64 bytes)
 * - Page 255: General data storage (64 bytes)
 * 
 * @example
 * // ตัวอย่างการใช้งานพื้นฐาน
 * #include "SimpleFlash.h"
 * 
 * typedef struct {
 *     uint32_t magic;      // Magic number สำหรับตรวจสอบ
 *     uint16_t version;
 *     uint16_t brightness;
 *     uint16_t crc;        // CRC16 checksum
 * } Config_t;
 * 
 * int main(void) {
 *     Flash_Init();
 *     
 *     Config_t config;
 *     if (Flash_LoadConfig(&config, sizeof(config))) {
 *         // ใช้ค่า config ที่โหลดมา
 *     } else {
 *         // ตั้งค่า default
 *         config.magic = 0x12345678;
 *         config.version = 1;
 *         config.brightness = 50;
 *         Flash_SaveConfig(&config, sizeof(config));
 *     }
 * }
 * 
 * @warning ต้องแน่ใจว่า linker script ไม่ใช้พื้นที่ 0x0803F80-0x0803FFF
 * @note สำหรับข้อมูลที่เปลี่ยนบ่อย (>100 ครั้ง/วัน) ควรใช้ wear leveling
 */

#ifndef __SIMPLE_FLASH_H
#define __SIMPLE_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch32v00x.h>
#include "ch32v00x_flash.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ========== Flash Memory Configuration ========== */

/**
 * @brief Flash memory constants
 */
#define FLASH_PAGE_SIZE              64        /**< ขนาด 1 page = 64 bytes */
#define FLASH_TOTAL_PAGES            256       /**< จำนวน pages ทั้งหมด */
#define FLASH_BASE_ADDRESS           0x08000000 /**< Flash base address */

/**
 * @brief Flash storage area configuration
 * @note ใช้ last 2 pages (254-255) สำหรับเก็บข้อมูล
 */
#define FLASH_STORAGE_PAGE_START     254       /**< Page เริ่มต้นสำหรับเก็บข้อมูล */
#define FLASH_STORAGE_PAGE_COUNT     2         /**< จำนวน pages สำหรับเก็บข้อมูล */
#define FLASH_STORAGE_START_ADDR     (FLASH_BASE_ADDRESS + (FLASH_STORAGE_PAGE_START * FLASH_PAGE_SIZE))
#define FLASH_STORAGE_SIZE           (FLASH_STORAGE_PAGE_COUNT * FLASH_PAGE_SIZE)  /**< 128 bytes */

/**
 * @brief Flash storage sections
 */
#define FLASH_CONFIG_PAGE            254       /**< Page สำหรับ configuration */
#define FLASH_CONFIG_ADDR            (FLASH_BASE_ADDRESS + (FLASH_CONFIG_PAGE * FLASH_PAGE_SIZE))
#define FLASH_CONFIG_SIZE            FLASH_PAGE_SIZE  /**< 64 bytes */

#define FLASH_DATA_PAGE              255       /**< Page สำหรับ general data */
#define FLASH_DATA_ADDR              (FLASH_BASE_ADDRESS + (FLASH_DATA_PAGE * FLASH_PAGE_SIZE))
#define FLASH_DATA_SIZE              FLASH_PAGE_SIZE  /**< 64 bytes */

/**
 * @brief String and data limits
 */
#define FLASH_MAX_STRING_LENGTH      60        /**< ความยาว string สูงสุด (เหลือที่สำหรับ null + metadata) */

/* ========== Status Codes ========== */

/**
 * @brief Flash operation status codes
 */
typedef enum {
    FLASH_OK = 0,              /**< Operation successful */
    FLASH_ERROR,               /**< General error */
    FLASH_ERROR_BUSY,          /**< Flash is busy */
    FLASH_ERROR_TIMEOUT,       /**< Operation timeout */
    FLASH_ERROR_WRITE,         /**< Write error */
    FLASH_ERROR_ERASE,         /**< Erase error */
    FLASH_ERROR_VERIFY,        /**< Verification failed */
    FLASH_ERROR_ALIGN,         /**< Address alignment error */
    FLASH_ERROR_RANGE,         /**< Address out of range */
    FLASH_ERROR_CRC,           /**< CRC check failed */
    FLASH_ERROR_INVALID        /**< Invalid parameter */
} FlashStatus;

/* ========== Core Functions ========== */

/**
 * @brief เริ่มต้นระบบ Flash storage
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @note ต้องเรียกฟังก์ชันนี้ก่อนใช้งาน Flash storage
 * 
 * @example
 * Flash_Init();
 */
FlashStatus Flash_Init(void);

/**
 * @brief ลบข้อมูลทั้งหมดใน storage area
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @warning จะลบข้อมูลทั้งหมด (config + data)
 * 
 * @example
 * Flash_EraseAll();  // Factory reset
 */
FlashStatus Flash_EraseAll(void);

/**
 * @brief ลบข้อมูลใน page ที่กำหนด
 * @param page_num หมายเลข page (254 หรือ 255)
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @note page_num ต้องเป็น 254 (config) หรือ 255 (data) เท่านั้น
 * 
 * @example
 * Flash_ErasePage(FLASH_CONFIG_PAGE);  // ลบ config page
 */
FlashStatus Flash_ErasePage(uint8_t page_num);

/* ========== Basic Read/Write Functions ========== */

/**
 * @brief อ่านข้อมูล 1 byte จาก Flash
 * @param addr ที่อยู่ใน Flash (ต้องอยู่ใน storage area)
 * @return ค่าที่อ่านได้
 * 
 * @example
 * uint8_t value = Flash_ReadByte(FLASH_CONFIG_ADDR);
 */
uint8_t Flash_ReadByte(uint32_t addr);

/**
 * @brief อ่านข้อมูล 16-bit (half-word) จาก Flash
 * @param addr ที่อยู่ใน Flash (ต้อง align 2 bytes)
 * @return ค่าที่อ่านได้
 * 
 * @note addr ต้องเป็นเลขคู่ (align 2 bytes)
 * 
 * @example
 * uint16_t value = Flash_ReadHalfWord(FLASH_CONFIG_ADDR);
 */
uint16_t Flash_ReadHalfWord(uint32_t addr);

/**
 * @brief อ่านข้อมูล 32-bit (word) จาก Flash
 * @param addr ที่อยู่ใน Flash (ต้อง align 4 bytes)
 * @return ค่าที่อ่านได้
 * 
 * @note addr ต้องหาร 4 ลงตัว (align 4 bytes)
 * 
 * @example
 * uint32_t value = Flash_ReadWord(FLASH_CONFIG_ADDR);
 */
uint32_t Flash_ReadWord(uint32_t addr);

/**
 * @brief เขียนข้อมูล 1 byte ลง Flash
 * @param addr ที่อยู่ใน Flash (ต้องอยู่ใน storage area)
 * @param data ข้อมูลที่ต้องการเขียน
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @warning ต้อง erase page ก่อนเขียนข้อมูลใหม่
 * @note ใช้ Flash_WriteByteWithErase() ถ้าต้องการ auto-erase
 * 
 * @example
 * Flash_ErasePage(FLASH_CONFIG_PAGE);
 * Flash_WriteByte(FLASH_CONFIG_ADDR, 0x55);
 */
FlashStatus Flash_WriteByte(uint32_t addr, uint8_t data);

/**
 * @brief เขียนข้อมูล 16-bit (half-word) ลง Flash
 * @param addr ที่อยู่ใน Flash (ต้อง align 2 bytes)
 * @param data ข้อมูลที่ต้องการเขียน
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @warning ต้อง erase page ก่อนเขียนข้อมูลใหม่
 * @note addr ต้องเป็นเลขคู่ (align 2 bytes)
 * 
 * @example
 * Flash_ErasePage(FLASH_CONFIG_PAGE);
 * Flash_WriteHalfWord(FLASH_CONFIG_ADDR, 0x1234);
 */
FlashStatus Flash_WriteHalfWord(uint32_t addr, uint16_t data);

/**
 * @brief เขียนข้อมูล 32-bit (word) ลง Flash
 * @param addr ที่อยู่ใน Flash (ต้อง align 4 bytes)
 * @param data ข้อมูลที่ต้องการเขียน
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @warning ต้อง erase page ก่อนเขียนข้อมูลใหม่
 * @note addr ต้องหาร 4 ลงตัว (align 4 bytes)
 * 
 * @example
 * Flash_ErasePage(FLASH_CONFIG_PAGE);
 * Flash_WriteWord(FLASH_CONFIG_ADDR, 0x12345678);
 */
FlashStatus Flash_WriteWord(uint32_t addr, uint32_t data);

/* ========== String Functions ========== */

/**
 * @brief อ่าน null-terminated string จาก Flash
 * @param addr ที่อยู่ใน Flash
 * @param buffer buffer สำหรับเก็บ string
 * @param max_len ขนาดสูงสุดของ buffer (รวม null terminator)
 * @return จำนวน bytes ที่อ่านได้ (ไม่รวม null), หรือ 0 ถ้าผิดพลาด
 * 
 * @note buffer ต้องมีขนาดอย่างน้อย max_len bytes
 * 
 * @example
 * char name[32];
 * uint16_t len = Flash_ReadString(FLASH_DATA_ADDR, name, sizeof(name));
 * if (len > 0) {
 *     printf("Name: %s\n", name);
 * }
 */
uint16_t Flash_ReadString(uint32_t addr, char* buffer, uint16_t max_len);

/**
 * @brief เขียน null-terminated string ลง Flash
 * @param addr ที่อยู่ใน Flash
 * @param str string ที่ต้องการเขียน (null-terminated)
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @warning ต้อง erase page ก่อนเขียน
 * @note string ต้องไม่เกิน FLASH_MAX_STRING_LENGTH characters
 * 
 * @example
 * Flash_ErasePage(FLASH_DATA_PAGE);
 * Flash_WriteString(FLASH_DATA_ADDR, "Hello World");
 */
FlashStatus Flash_WriteString(uint32_t addr, const char* str);

/* ========== Struct/Buffer Functions ========== */

/**
 * @brief อ่าน struct/buffer จาก Flash
 * @param addr ที่อยู่ใน Flash
 * @param ptr pointer ไปยัง struct/buffer ปลายทาง
 * @param size ขนาดของ struct/buffer (bytes)
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @example
 * typedef struct {
 *     uint32_t id;
 *     uint16_t value;
 * } MyData_t;
 * 
 * MyData_t data;
 * Flash_ReadStruct(FLASH_DATA_ADDR, &data, sizeof(data));
 */
FlashStatus Flash_ReadStruct(uint32_t addr, void* ptr, uint16_t size);

/**
 * @brief เขียน struct/buffer ลง Flash
 * @param addr ที่อยู่ใน Flash
 * @param ptr pointer ไปยัง struct/buffer ต้นทาง
 * @param size ขนาดของ struct/buffer (bytes)
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @warning ต้อง erase page ก่อนเขียน
 * @note size ต้องไม่เกิน FLASH_PAGE_SIZE
 * 
 * @example
 * MyData_t data = {.id = 123, .value = 456};
 * Flash_ErasePage(FLASH_DATA_PAGE);
 * Flash_WriteStruct(FLASH_DATA_ADDR, &data, sizeof(data));
 */
FlashStatus Flash_WriteStruct(uint32_t addr, const void* ptr, uint16_t size);

/* ========== Configuration Management ========== */

/**
 * @brief บันทึก configuration พร้อม CRC validation
 * @param ptr pointer ไปยัง configuration struct
 * @param size ขนาดของ struct (ไม่รวม CRC field)
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @note CRC16 จะถูกคำนวณและเขียนต่อท้าย struct อัตโนมัติ
 * @note struct ต้องมี field uint16_t crc เป็น field สุดท้าย
 * @note size ต้องไม่เกิน FLASH_CONFIG_SIZE - 2 (เหลือที่สำหรับ CRC)
 * 
 * @example
 * typedef struct {
 *     uint32_t magic;
 *     uint16_t brightness;
 *     uint16_t crc;  // ต้องเป็น field สุดท้าย
 * } Config_t;
 * 
 * Config_t config = {.magic = 0x1234, .brightness = 50};
 * Flash_SaveConfig(&config, sizeof(config) - sizeof(config.crc));
 */
FlashStatus Flash_SaveConfig(const void* ptr, uint16_t size);

/**
 * @brief โหลด configuration และตรวจสอบ CRC
 * @param ptr pointer ไปยัง configuration struct ปลายทาง
 * @param size ขนาดของ struct (ไม่รวม CRC field)
 * @return true ถ้าโหลดสำเร็จและ CRC ถูกต้อง, false ถ้าผิดพลาด
 * 
 * @note struct ต้องมี field uint16_t crc เป็น field สุดท้าย
 * 
 * @example
 * Config_t config;
 * if (Flash_LoadConfig(&config, sizeof(config) - sizeof(config.crc))) {
 *     // ใช้ค่า config ที่โหลดมา
 *     printf("Brightness: %d\n", config.brightness);
 * } else {
 *     // ตั้งค่า default
 *     config.magic = 0x1234;
 *     config.brightness = 50;
 *     Flash_SaveConfig(&config, sizeof(config) - sizeof(config.crc));
 * }
 */
bool Flash_LoadConfig(void* ptr, uint16_t size);

/**
 * @brief ตรวจสอบว่ามี valid configuration ใน Flash หรือไม่
 * @return true ถ้ามี valid config (CRC ถูกต้อง)
 * 
 * @note ใช้สำหรับตรวจสอบก่อนโหลด config
 * 
 * @example
 * if (!Flash_IsConfigValid()) {
 *     // ตั้งค่า default config
 *     Config_t default_config = {...};
 *     Flash_SaveConfig(&default_config, sizeof(default_config) - 2);
 * }
 */
bool Flash_IsConfigValid(void);

/* ========== Utility Functions ========== */

/**
 * @brief คำนวณ CRC16-CCITT checksum
 * @param data pointer ไปยังข้อมูล
 * @param len ความยาวของข้อมูล (bytes)
 * @return CRC16 checksum
 * 
 * @note ใช้ polynomial 0x1021 (CRC16-CCITT)
 * 
 * @example
 * uint8_t data[] = {0x01, 0x02, 0x03};
 * uint16_t crc = Flash_CalculateCRC16(data, sizeof(data));
 */
uint16_t Flash_CalculateCRC16(const uint8_t* data, uint16_t len);

/**
 * @brief ตรวจสอบว่า address อยู่ใน storage area หรือไม่
 * @param addr ที่อยู่ที่ต้องการตรวจสอบ
 * @return true ถ้าอยู่ใน storage area
 * 
 * @example
 * if (Flash_IsAddressValid(addr)) {
 *     // ใช้งาน address ได้
 * }
 */
bool Flash_IsAddressValid(uint32_t addr);

/**
 * @brief แปลง page number เป็น address
 * @param page_num หมายเลข page (254 หรือ 255)
 * @return address ของ page, หรือ 0 ถ้า page_num ไม่ถูกต้อง
 * 
 * @example
 * uint32_t config_addr = Flash_GetPageAddress(FLASH_CONFIG_PAGE);
 */
uint32_t Flash_GetPageAddress(uint8_t page_num);

/* ========== Simplified API (Easy Mode) ========== */

/**
 * @brief มาโครสำหรับคำนวณขนาด config ที่ไม่รวม CRC
 * @note ใช้กับ struct ที่มี crc field เป็น field สุดท้าย
 * 
 * @example
 * Config_t config;
 * Flash_SaveConfig(&config, FLASH_CONFIG_SIZE_NO_CRC(config));
 */
#define FLASH_CONFIG_SIZE_NO_CRC(config) (sizeof(config) - sizeof((config).crc))

/**
 * @brief บันทึก config แบบง่าย (ไม่ต้องคำนวณ size)
 * @param config_ptr pointer ไปยัง config struct
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @note struct ต้องมี uint16_t crc เป็น field สุดท้าย
 * 
 * @example
 * Config_t config = {.magic = 0x1234, .brightness = 50};
 * FLASH_SAVE_CONFIG(&config);
 */
#define FLASH_SAVE_CONFIG(config_ptr) \
    Flash_SaveConfig((config_ptr), sizeof(*(config_ptr)) - sizeof((config_ptr)->crc))

/**
 * @brief โหลด config แบบง่าย (ไม่ต้องคำนวณ size)
 * @param config_ptr pointer ไปยัง config struct
 * @return true ถ้าโหลดสำเร็จ
 * 
 * @example
 * Config_t config;
 * if (FLASH_LOAD_CONFIG(&config)) {
 *     printf("Loaded: brightness=%d\n", config.brightness);
 * }
 */
#define FLASH_LOAD_CONFIG(config_ptr) \
    Flash_LoadConfig((config_ptr), sizeof(*(config_ptr)) - sizeof((config_ptr)->crc))

/**
 * @brief เขียนข้อมูลแบบง่าย (auto-erase)
 * @param addr ที่อยู่ใน Flash
 * @param value ค่าที่ต้องการเขียน (รองรับ uint8_t, uint16_t, uint32_t)
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @note ฟังก์ชันนี้จะ erase page อัตโนมัติ (ช้ากว่าปกติ)
 * 
 * @example
 * FLASH_WRITE_AUTO(FLASH_DATA_ADDR, 0x12345678);  // เขียน word
 * FLASH_WRITE_AUTO(FLASH_DATA_ADDR + 4, 0x1234);  // เขียน half-word
 */
#define FLASH_WRITE_AUTO(addr, value) \
    _Generic((value), \
        uint8_t:  Flash_WriteByteWithErase, \
        uint16_t: Flash_WriteHalfWordWithErase, \
        uint32_t: Flash_WriteWordWithErase, \
        int:      Flash_WriteWordWithErase \
    )((addr), (value))

/**
 * @brief อ่านข้อมูลแบบง่าย (auto-detect type)
 * @param addr ที่อยู่ใน Flash
 * @param var_ptr pointer ไปยังตัวแปรที่จะเก็บค่า
 * 
 * @example
 * uint32_t value;
 * FLASH_READ(FLASH_DATA_ADDR, &value);
 */
#define FLASH_READ(addr, var_ptr) \
    do { \
        if (sizeof(*(var_ptr)) == 1) *(var_ptr) = Flash_ReadByte(addr); \
        else if (sizeof(*(var_ptr)) == 2) *(var_ptr) = Flash_ReadHalfWord(addr); \
        else if (sizeof(*(var_ptr)) == 4) *(var_ptr) = Flash_ReadWord(addr); \
    } while(0)

/* ========== Advanced Functions ========== */

/**
 * @brief เขียน byte พร้อม auto-erase (modify-erase-write)
 * @param addr ที่อยู่ใน Flash
 * @param data ข้อมูลที่ต้องการเขียน
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @note ฟังก์ชันนี้จะ:
 *       1. อ่าน page ทั้งหมดมาเก็บใน buffer
 *       2. แก้ไข byte ที่ต้องการ
 *       3. Erase page
 *       4. เขียน buffer กลับลง Flash
 * @warning ใช้ RAM 64 bytes สำหรับ buffer
 * @warning ช้ากว่าการเขียนปกติ เพราะต้อง erase ทั้ง page
 * 
 * @example
 * Flash_WriteByteWithErase(FLASH_CONFIG_ADDR + 5, 0xAB);
 */
FlashStatus Flash_WriteByteWithErase(uint32_t addr, uint8_t data);

/**
 * @brief เขียน half-word พร้อม auto-erase
 * @param addr ที่อยู่ใน Flash (ต้อง align 2 bytes)
 * @param data ข้อมูลที่ต้องการเขียน
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @note ดูรายละเอียดใน Flash_WriteByteWithErase()
 */
FlashStatus Flash_WriteHalfWordWithErase(uint32_t addr, uint16_t data);

/**
 * @brief เขียน word พร้อม auto-erase
 * @param addr ที่อยู่ใน Flash (ต้อง align 4 bytes)
 * @param data ข้อมูลที่ต้องการเขียน
 * @return FLASH_OK ถ้าสำเร็จ
 * 
 * @note ดูรายละเอียดใน Flash_WriteByteWithErase()
 */
FlashStatus Flash_WriteWordWithErase(uint32_t addr, uint32_t data);

#ifdef __cplusplus
}
#endif

#endif  // __SIMPLE_FLASH_H
