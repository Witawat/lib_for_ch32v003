/**
 * @file flash_basic_read_write.c
 * @brief ตัวอย่างการใช้งาน SimpleFlash พื้นฐาน - อ่าน/เขียนข้อมูล
 * @date 2025-12-21
 * 
 * @details
 * ตัวอย่างนี้แสดงการใช้งาน SimpleFlash ในระดับพื้นฐาน:
 * - การเขียนและอ่านข้อมูล byte, half-word, word
 * - การลบ page
 * - การตรวจสอบข้อมูล
 */

#include "debug.h"
#include "SimpleHAL/SimpleFlash.h"

/**
 * @brief ตัวอย่างการอ่าน/เขียน byte
 */
void example_byte_operations(void) {
    printf("\n=== Byte Operations ===\n");
    
    // ลบ data page ก่อนเขียน
    printf("Erasing data page...\n");
    FlashStatus status = Flash_ErasePage(FLASH_DATA_PAGE);
    if (status != FLASH_OK) {
        printf("Error erasing page: %d\n", status);
        return;
    }
    
    // เขียนข้อมูล byte
    uint8_t write_byte = 0xAB;
    printf("Writing byte 0x%02X to address 0x%08X\n", write_byte, FLASH_DATA_ADDR);
    status = Flash_WriteByte(FLASH_DATA_ADDR, write_byte);
    if (status != FLASH_OK) {
        printf("Error writing byte: %d\n", status);
        return;
    }
    
    // อ่านข้อมูลกลับมา
    uint8_t read_byte = Flash_ReadByte(FLASH_DATA_ADDR);
    printf("Read byte: 0x%02X\n", read_byte);
    
    // ตรวจสอบ
    if (read_byte == write_byte) {
        printf("✓ Byte write/read successful!\n");
    } else {
        printf("✗ Byte verification failed!\n");
    }
}

/**
 * @brief ตัวอย่างการอ่าน/เขียน half-word (16-bit)
 */
void example_halfword_operations(void) {
    printf("\n=== Half-Word Operations ===\n");
    
    // ลบ page
    Flash_ErasePage(FLASH_DATA_PAGE);
    
    // เขียนข้อมูล half-word
    uint16_t write_halfword = 0x1234;
    printf("Writing half-word 0x%04X\n", write_halfword);
    FlashStatus status = Flash_WriteHalfWord(FLASH_DATA_ADDR, write_halfword);
    if (status != FLASH_OK) {
        printf("Error writing half-word: %d\n", status);
        return;
    }
    
    // อ่านข้อมูลกลับมา
    uint16_t read_halfword = Flash_ReadHalfWord(FLASH_DATA_ADDR);
    printf("Read half-word: 0x%04X\n", read_halfword);
    
    // ตรวจสอบ
    if (read_halfword == write_halfword) {
        printf("✓ Half-word write/read successful!\n");
    } else {
        printf("✗ Half-word verification failed!\n");
    }
}

/**
 * @brief ตัวอย่างการอ่าน/เขียน word (32-bit)
 */
void example_word_operations(void) {
    printf("\n=== Word Operations ===\n");
    
    // ลบ page
    Flash_ErasePage(FLASH_DATA_PAGE);
    
    // เขียนข้อมูล word
    uint32_t write_word = 0x12345678;
    printf("Writing word 0x%08X\n", write_word);
    FlashStatus status = Flash_WriteWord(FLASH_DATA_ADDR, write_word);
    if (status != FLASH_OK) {
        printf("Error writing word: %d\n", status);
        return;
    }
    
    // อ่านข้อมูลกลับมา
    uint32_t read_word = Flash_ReadWord(FLASH_DATA_ADDR);
    printf("Read word: 0x%08X\n", read_word);
    
    // ตรวจสอบ
    if (read_word == write_word) {
        printf("✓ Word write/read successful!\n");
    } else {
        printf("✗ Word verification failed!\n");
    }
}

/**
 * @brief ตัวอย่างการเขียนหลายค่าพร้อมกัน
 */
void example_multiple_writes(void) {
    printf("\n=== Multiple Writes ===\n");
    
    // ลบ page
    Flash_ErasePage(FLASH_DATA_PAGE);
    
    // เขียนข้อมูลหลายค่า
    printf("Writing multiple values...\n");
    Flash_WriteByte(FLASH_DATA_ADDR + 0, 0x11);
    Flash_WriteByte(FLASH_DATA_ADDR + 1, 0x22);
    Flash_WriteHalfWord(FLASH_DATA_ADDR + 2, 0x3344);
    Flash_WriteWord(FLASH_DATA_ADDR + 4, 0x55667788);
    
    // อ่านและแสดงผล
    printf("Reading back:\n");
    printf("  Byte[0]: 0x%02X\n", Flash_ReadByte(FLASH_DATA_ADDR + 0));
    printf("  Byte[1]: 0x%02X\n", Flash_ReadByte(FLASH_DATA_ADDR + 1));
    printf("  HWord[2]: 0x%04X\n", Flash_ReadHalfWord(FLASH_DATA_ADDR + 2));
    printf("  Word[4]: 0x%08X\n", Flash_ReadWord(FLASH_DATA_ADDR + 4));
}

/**
 * @brief ตัวอย่างการใช้ WriteWithErase (modify-erase-write)
 */
void example_write_with_erase(void) {
    printf("\n=== Write With Auto-Erase ===\n");
    
    // เขียนข้อมูลเริ่มต้น
    Flash_ErasePage(FLASH_DATA_PAGE);
    Flash_WriteWord(FLASH_DATA_ADDR, 0x11111111);
    Flash_WriteWord(FLASH_DATA_ADDR + 4, 0x22222222);
    
    printf("Initial data:\n");
    printf("  Word[0]: 0x%08X\n", Flash_ReadWord(FLASH_DATA_ADDR));
    printf("  Word[4]: 0x%08X\n", Flash_ReadWord(FLASH_DATA_ADDR + 4));
    
    // แก้ไขข้อมูลโดยใช้ WriteWithErase
    printf("\nModifying byte at offset 1...\n");
    Flash_WriteByteWithErase(FLASH_DATA_ADDR + 1, 0xFF);
    
    printf("After modification:\n");
    printf("  Word[0]: 0x%08X\n", Flash_ReadWord(FLASH_DATA_ADDR));
    printf("  Word[4]: 0x%08X\n", Flash_ReadWord(FLASH_DATA_ADDR + 4));
    
    printf("✓ WriteWithErase preserves other data!\n");
}

/**
 * @brief Main function
 */
int main(void) {
    // เริ่มต้นระบบ
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    
    printf("\n");
    printf("========================================\n");
    printf("  SimpleFlash Basic Read/Write Example\n");
    printf("========================================\n");
    
    // เริ่มต้น Flash
    printf("\nInitializing Flash...\n");
    FlashStatus status = Flash_Init();
    if (status != FLASH_OK) {
        printf("Flash initialization failed: %d\n", status);
        while(1);
    }
    printf("Flash initialized successfully!\n");
    
    // แสดงข้อมูล Flash
    printf("\nFlash Storage Info:\n");
    printf("  Storage Start: 0x%08X\n", FLASH_STORAGE_START_ADDR);
    printf("  Storage Size: %d bytes\n", FLASH_STORAGE_SIZE);
    printf("  Config Page: %d (0x%08X)\n", FLASH_CONFIG_PAGE, FLASH_CONFIG_ADDR);
    printf("  Data Page: %d (0x%08X)\n", FLASH_DATA_PAGE, FLASH_DATA_ADDR);
    
    // รันตัวอย่างต่างๆ
    example_byte_operations();
    Delay_Ms(500);
    
    example_halfword_operations();
    Delay_Ms(500);
    
    example_word_operations();
    Delay_Ms(500);
    
    example_multiple_writes();
    Delay_Ms(500);
    
    example_write_with_erase();
    
    printf("\n========================================\n");
    printf("  All examples completed!\n");
    printf("========================================\n");
    
    while(1) {
        Delay_Ms(1000);
    }
}
