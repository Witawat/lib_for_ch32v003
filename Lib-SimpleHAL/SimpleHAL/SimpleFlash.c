/**
 * @file SimpleFlash.c
 * @brief Simple Flash Storage Library Implementation
 * @version 1.0
 * @date 2025-12-21
 */

#include "SimpleFlash.h"


/* ========== Private Variables ========== */

static bool flash_initialized = false;

/* ========== Private Function Prototypes ========== */

static FlashStatus Flash_UnlockInternal(void);
static void Flash_LockInternal(void);
static FlashStatus Flash_WaitForOperation(uint32_t timeout_ms);
static FlashStatus Flash_ConvertStatus(FLASH_Status status);

/* ========== Core Functions Implementation ========== */

/**
 * @brief เริ่มต้นระบบ Flash storage
 */
FlashStatus Flash_Init(void) {
    if (flash_initialized) {
        return FLASH_OK;
    }
    
    // Enable flash clock (already enabled by default)
    // Set flash latency based on system clock
    FLASH_SetLatency(FLASH_Latency_0);  // For 24MHz or less
    
    flash_initialized = true;
    return FLASH_OK;
}

/**
 * @brief ลบข้อมูลทั้งหมดใน storage area
 */
FlashStatus Flash_EraseAll(void) {
    FlashStatus status;
    
    // Erase config page
    status = Flash_ErasePage(FLASH_CONFIG_PAGE);
    if (status != FLASH_OK) {
        return status;
    }
    
    // Erase data page
    status = Flash_ErasePage(FLASH_DATA_PAGE);
    return status;
}

/**
 * @brief ลบข้อมูลใน page ที่กำหนด
 */
FlashStatus Flash_ErasePage(uint8_t page_num) {
    // Validate page number (254 or 255 only)
    if (page_num != FLASH_CONFIG_PAGE && page_num != FLASH_DATA_PAGE) {
        return FLASH_ERROR_RANGE;
    }
    
    uint32_t page_addr = Flash_GetPageAddress(page_num);
    if (page_addr == 0) {
        return FLASH_ERROR_RANGE;
    }
    
    // Unlock flash
    FlashStatus status = Flash_UnlockInternal();
    if (status != FLASH_OK) {
        return status;
    }
    
    // Erase page
    FLASH_Status flash_status = FLASH_ErasePage(page_addr);
    
    // Lock flash
    Flash_LockInternal();
    
    return Flash_ConvertStatus(flash_status);
}

/* ========== Basic Read Functions ========== */

/**
 * @brief อ่านข้อมูล 1 byte จาก Flash
 */
uint8_t Flash_ReadByte(uint32_t addr) {
    if (!Flash_IsAddressValid(addr)) {
        return 0;
    }
    return *(volatile uint8_t*)addr;
}

/**
 * @brief อ่านข้อมูล 16-bit จาก Flash
 */
uint16_t Flash_ReadHalfWord(uint32_t addr) {
    if (!Flash_IsAddressValid(addr) || (addr & 0x01)) {  // Check alignment
        return 0;
    }
    return *(volatile uint16_t*)addr;
}

/**
 * @brief อ่านข้อมูล 32-bit จาก Flash
 */
uint32_t Flash_ReadWord(uint32_t addr) {
    if (!Flash_IsAddressValid(addr) || (addr & 0x03)) {  // Check alignment
        return 0;
    }
    return *(volatile uint32_t*)addr;
}

/* ========== Basic Write Functions ========== */

/**
 * @brief เขียนข้อมูล 1 byte ลง Flash
 */
FlashStatus Flash_WriteByte(uint32_t addr, uint8_t data) {
    // Validate address
    if (!Flash_IsAddressValid(addr)) {
        return FLASH_ERROR_RANGE;
    }
    
    // For byte write, we need to use half-word programming
    // Read the existing half-word, modify the byte, and write back
    uint32_t aligned_addr = addr & ~0x01;  // Align to half-word
    uint16_t half_word;
    
    if (addr & 0x01) {
        // Odd address - modify high byte
        half_word = Flash_ReadByte(aligned_addr) | ((uint16_t)data << 8);
    } else {
        // Even address - modify low byte
        half_word = ((uint16_t)Flash_ReadByte(aligned_addr + 1) << 8) | data;
    }
    
    return Flash_WriteHalfWord(aligned_addr, half_word);
}

/**
 * @brief เขียนข้อมูล 16-bit ลง Flash
 */
FlashStatus Flash_WriteHalfWord(uint32_t addr, uint16_t data) {
    // Validate address and alignment
    if (!Flash_IsAddressValid(addr)) {
        return FLASH_ERROR_RANGE;
    }
    if (addr & 0x01) {
        return FLASH_ERROR_ALIGN;
    }
    
    // Unlock flash
    FlashStatus status = Flash_UnlockInternal();
    if (status != FLASH_OK) {
        return status;
    }
    
    // Program half-word
    FLASH_Status flash_status = FLASH_ProgramHalfWord(addr, data);
    
    // Lock flash
    Flash_LockInternal();
    
    // Verify
    if (flash_status == FLASH_COMPLETE) {
        if (Flash_ReadHalfWord(addr) != data) {
            return FLASH_ERROR_VERIFY;
        }
    }
    
    return Flash_ConvertStatus(flash_status);
}

/**
 * @brief เขียนข้อมูล 32-bit ลง Flash
 */
FlashStatus Flash_WriteWord(uint32_t addr, uint32_t data) {
    // Validate address and alignment
    if (!Flash_IsAddressValid(addr)) {
        return FLASH_ERROR_RANGE;
    }
    if (addr & 0x03) {
        return FLASH_ERROR_ALIGN;
    }
    
    // Unlock flash
    FlashStatus status = Flash_UnlockInternal();
    if (status != FLASH_OK) {
        return status;
    }
    
    // Program word
    FLASH_Status flash_status = FLASH_ProgramWord(addr, data);
    
    // Lock flash
    Flash_LockInternal();
    
    // Verify
    if (flash_status == FLASH_COMPLETE) {
        if (Flash_ReadWord(addr) != data) {
            return FLASH_ERROR_VERIFY;
        }
    }
    
    return Flash_ConvertStatus(flash_status);
}

/* ========== String Functions ========== */

/**
 * @brief อ่าน null-terminated string จาก Flash
 */
uint16_t Flash_ReadString(uint32_t addr, char* buffer, uint16_t max_len) {
    if (!Flash_IsAddressValid(addr) || buffer == NULL || max_len == 0) {
        return 0;
    }
    
    uint16_t i;
    for (i = 0; i < max_len - 1; i++) {
        char c = (char)Flash_ReadByte(addr + i);
        buffer[i] = c;
        if (c == '\0') {
            return i;  // Return length without null terminator
        }
    }
    
    buffer[max_len - 1] = '\0';  // Ensure null termination
    return i;
}

/**
 * @brief เขียน null-terminated string ลง Flash
 */
FlashStatus Flash_WriteString(uint32_t addr, const char* str) {
    if (!Flash_IsAddressValid(addr) || str == NULL) {
        return FLASH_ERROR_INVALID;
    }
    
    uint16_t len = strlen(str);
    if (len > FLASH_MAX_STRING_LENGTH) {
        return FLASH_ERROR_RANGE;
    }
    
    // Write string including null terminator
    FlashStatus status;
    for (uint16_t i = 0; i <= len; i++) {
        status = Flash_WriteByte(addr + i, (uint8_t)str[i]);
        if (status != FLASH_OK) {
            return status;
        }
    }
    
    return FLASH_OK;
}

/* ========== Struct/Buffer Functions ========== */

/**
 * @brief อ่าน struct/buffer จาก Flash
 */
FlashStatus Flash_ReadStruct(uint32_t addr, void* ptr, uint16_t size) {
    if (!Flash_IsAddressValid(addr) || ptr == NULL || size == 0) {
        return FLASH_ERROR_INVALID;
    }
    
    if (size > FLASH_PAGE_SIZE) {
        return FLASH_ERROR_RANGE;
    }
    
    uint8_t* dest = (uint8_t*)ptr;
    for (uint16_t i = 0; i < size; i++) {
        dest[i] = Flash_ReadByte(addr + i);
    }
    
    return FLASH_OK;
}

/**
 * @brief เขียน struct/buffer ลง Flash
 */
FlashStatus Flash_WriteStruct(uint32_t addr, const void* ptr, uint16_t size) {
    if (!Flash_IsAddressValid(addr) || ptr == NULL || size == 0) {
        return FLASH_ERROR_INVALID;
    }
    
    if (size > FLASH_PAGE_SIZE) {
        return FLASH_ERROR_RANGE;
    }
    
    const uint8_t* src = (const uint8_t*)ptr;
    FlashStatus status;
    
    for (uint16_t i = 0; i < size; i++) {
        status = Flash_WriteByte(addr + i, src[i]);
        if (status != FLASH_OK) {
            return status;
        }
    }
    
    return FLASH_OK;
}

/* ========== Configuration Management ========== */

/**
 * @brief บันทึก configuration พร้อม CRC validation
 */
FlashStatus Flash_SaveConfig(const void* ptr, uint16_t size) {
    if (ptr == NULL || size == 0 || size > (FLASH_CONFIG_SIZE - 2)) {
        return FLASH_ERROR_INVALID;
    }
    
    // Calculate CRC
    uint16_t crc = Flash_CalculateCRC16((const uint8_t*)ptr, size);
    
    // Erase config page
    FlashStatus status = Flash_ErasePage(FLASH_CONFIG_PAGE);
    if (status != FLASH_OK) {
        return status;
    }
    
    // Write config data
    status = Flash_WriteStruct(FLASH_CONFIG_ADDR, ptr, size);
    if (status != FLASH_OK) {
        return status;
    }
    
    // Write CRC at the end
    status = Flash_WriteHalfWord(FLASH_CONFIG_ADDR + size, crc);
    return status;
}

/**
 * @brief โหลด configuration และตรวจสอบ CRC
 */
bool Flash_LoadConfig(void* ptr, uint16_t size) {
    if (ptr == NULL || size == 0 || size > (FLASH_CONFIG_SIZE - 2)) {
        return false;
    }
    
    // Read config data
    if (Flash_ReadStruct(FLASH_CONFIG_ADDR, ptr, size) != FLASH_OK) {
        return false;
    }
    
    // Read stored CRC
    uint16_t stored_crc = Flash_ReadHalfWord(FLASH_CONFIG_ADDR + size);
    
    // Calculate CRC of read data
    uint16_t calculated_crc = Flash_CalculateCRC16((const uint8_t*)ptr, size);
    
    // Compare CRCs
    return (stored_crc == calculated_crc);
}

/**
 * @brief ตรวจสอบว่ามี valid configuration ใน Flash หรือไม่
 */
bool Flash_IsConfigValid(void) {
    // Read first 4 bytes to check if page is erased (all 0xFF)
    uint32_t first_word = Flash_ReadWord(FLASH_CONFIG_ADDR);
    if (first_word == 0xFFFFFFFF) {
        return false;  // Page is erased, no config
    }
    
    // For a more thorough check, we would need to know the config size
    // This is a basic check
    return true;
}

/* ========== Utility Functions ========== */

/**
 * @brief คำนวณ CRC16-CCITT checksum
 */
uint16_t Flash_CalculateCRC16(const uint8_t* data, uint16_t len) {
    uint16_t crc = 0xFFFF;  // Initial value
    
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;  // Polynomial
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

/**
 * @brief ตรวจสอบว่า address อยู่ใน storage area หรือไม่
 */
bool Flash_IsAddressValid(uint32_t addr) {
    return (addr >= FLASH_STORAGE_START_ADDR && 
            addr < (FLASH_STORAGE_START_ADDR + FLASH_STORAGE_SIZE));
}

/**
 * @brief แปลง page number เป็น address
 */
uint32_t Flash_GetPageAddress(uint8_t page_num) {
    // Validate page number (254 or 255 only)
    if (page_num != FLASH_CONFIG_PAGE && page_num != FLASH_DATA_PAGE) {
        return 0;
    }
    return FLASH_BASE_ADDRESS + (page_num * FLASH_PAGE_SIZE);
}

/* ========== Advanced Functions ========== */

/**
 * @brief เขียน byte พร้อม auto-erase (modify-erase-write)
 */
FlashStatus Flash_WriteByteWithErase(uint32_t addr, uint8_t data) {
    if (!Flash_IsAddressValid(addr)) {
        return FLASH_ERROR_RANGE;
    }
    
    // Determine which page this address belongs to
    uint32_t page_addr = addr & ~(FLASH_PAGE_SIZE - 1);  // Align to page boundary
    uint8_t page_num = (addr - FLASH_BASE_ADDRESS) / FLASH_PAGE_SIZE;
    
    // Read entire page into buffer
    uint8_t page_buffer[FLASH_PAGE_SIZE];
    for (uint16_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        page_buffer[i] = Flash_ReadByte(page_addr + i);
    }
    
    // Modify the byte
    uint16_t offset = addr - page_addr;
    page_buffer[offset] = data;
    
    // Erase page
    FlashStatus status = Flash_ErasePage(page_num);
    if (status != FLASH_OK) {
        return status;
    }
    
    // Write buffer back
    for (uint16_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        status = Flash_WriteByte(page_addr + i, page_buffer[i]);
        if (status != FLASH_OK) {
            return status;
        }
    }
    
    return FLASH_OK;
}

/**
 * @brief เขียน half-word พร้อม auto-erase
 */
FlashStatus Flash_WriteHalfWordWithErase(uint32_t addr, uint16_t data) {
    if (!Flash_IsAddressValid(addr) || (addr & 0x01)) {
        return FLASH_ERROR_ALIGN;
    }
    
    // Use byte write for each byte
    FlashStatus status = Flash_WriteByteWithErase(addr, (uint8_t)(data & 0xFF));
    if (status != FLASH_OK) {
        return status;
    }
    
    return Flash_WriteByteWithErase(addr + 1, (uint8_t)(data >> 8));
}

/**
 * @brief เขียน word พร้อม auto-erase
 */
FlashStatus Flash_WriteWordWithErase(uint32_t addr, uint32_t data) {
    if (!Flash_IsAddressValid(addr) || (addr & 0x03)) {
        return FLASH_ERROR_ALIGN;
    }
    
    // Write as two half-words
    FlashStatus status = Flash_WriteHalfWordWithErase(addr, (uint16_t)(data & 0xFFFF));
    if (status != FLASH_OK) {
        return status;
    }
    
    return Flash_WriteHalfWordWithErase(addr + 2, (uint16_t)(data >> 16));
}

/* ========== Private Functions ========== */

/**
 * @brief Unlock flash for programming
 */
static FlashStatus Flash_UnlockInternal(void) {
    FLASH_Unlock();
    return FLASH_OK;
}

/**
 * @brief Lock flash after programming
 */
static void Flash_LockInternal(void) {
    FLASH_Lock();
}

/**
 * @brief Wait for flash operation to complete
 */
static FlashStatus Flash_WaitForOperation(uint32_t timeout_ms) __attribute__((unused));
static FlashStatus Flash_WaitForOperation(uint32_t timeout_ms) {
    FLASH_Status status = FLASH_WaitForLastOperation(timeout_ms * 1000);  // Convert to us
    return Flash_ConvertStatus(status);
}

/**
 * @brief Convert FLASH_Status to FlashStatus
 */
static FlashStatus Flash_ConvertStatus(FLASH_Status status) {
    switch (status) {
        case FLASH_COMPLETE:
            return FLASH_OK;
        case FLASH_BUSY:
            return FLASH_ERROR_BUSY;
        case FLASH_TIMEOUT:
            return FLASH_ERROR_TIMEOUT;
        case FLASH_ERROR_PG:
            return FLASH_ERROR_WRITE;
        case FLASH_ERROR_WRP:
            return FLASH_ERROR_WRITE;
        case FLASH_ALIGN_ERROR:
            return FLASH_ERROR_ALIGN;
        case FLASH_ADR_RANGE_ERROR:
        case FLASH_OP_RANGE_ERROR:
            return FLASH_ERROR_RANGE;
        default:
            return FLASH_ERROR;
    }
}
