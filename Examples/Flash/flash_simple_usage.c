/**
 * @file flash_simple_usage.c
 * @brief ตัวอย่างการใช้งาน SimpleFlash แบบง่ายที่สุด
 * @date 2025-12-21
 * 
 * @details
 * ตัวอย่างนี้แสดงการใช้งาน SimpleFlash ด้วย API ที่ง่ายที่สุด
 * ไม่ต้องคำนวณ size หรือจัดการ erase page เอง
 */

#include "debug.h"
#include "SimpleHAL/SimpleHAL.h"

/**
 * @brief Configuration structure
 */
typedef struct {
    uint32_t magic;
    uint16_t brightness;
    uint16_t volume;
    uint16_t crc;  // ต้องเป็น field สุดท้าย!
} Config_t;

#define CONFIG_MAGIC 0xABCD1234

/**
 * @brief ตัวอย่างการใช้งานแบบง่าย - Configuration
 */
void example_simple_config(void) {
    printf("\n=== Simple Config Example ===\n");
    
    Config_t config;
    
    // โหลด config (ใช้ macro ไม่ต้องคำนวณ size)
    if (FLASH_LOAD_CONFIG(&config)) {
        printf("✓ Config loaded!\n");
        printf("  Brightness: %d\n", config.brightness);
        printf("  Volume: %d\n", config.volume);
        
        // แก้ไขค่า
        config.brightness = 80;
        config.volume = 60;
        
        // บันทึกกลับ (ใช้ macro ไม่ต้องคำนวณ size)
        FLASH_SAVE_CONFIG(&config);
        printf("✓ Config updated!\n");
    } else {
        printf("No config found, creating default...\n");
        
        // สร้าง config ใหม่
        config.magic = CONFIG_MAGIC;
        config.brightness = 50;
        config.volume = 75;
        
        // บันทึก (ใช้ macro)
        FLASH_SAVE_CONFIG(&config);
        printf("✓ Default config saved!\n");
    }
}

/**
 * @brief ตัวอย่างการเขียน/อ่านข้อมูลแบบง่าย
 */
void example_simple_read_write(void) {
    printf("\n=== Simple Read/Write Example ===\n");
    
    // เขียนข้อมูลแบบง่าย (auto-erase)
    printf("Writing data with auto-erase...\n");
    FLASH_WRITE_AUTO(FLASH_DATA_ADDR, 0x12345678);
    FLASH_WRITE_AUTO(FLASH_DATA_ADDR + 4, 0xABCD);
    FLASH_WRITE_AUTO(FLASH_DATA_ADDR + 6, 0x55);
    
    // อ่านข้อมูลแบบง่าย
    uint32_t word_val;
    uint16_t half_val;
    uint8_t byte_val;
    
    FLASH_READ(FLASH_DATA_ADDR, &word_val);
    FLASH_READ(FLASH_DATA_ADDR + 4, &half_val);
    FLASH_READ(FLASH_DATA_ADDR + 6, &byte_val);
    
    printf("Read values:\n");
    printf("  Word: 0x%08X\n", word_val);
    printf("  Half: 0x%04X\n", half_val);
    printf("  Byte: 0x%02X\n", byte_val);
}

/**
 * @brief ตัวอย่างการเก็บ string แบบง่าย
 */
void example_simple_string(void) {
    printf("\n=== Simple String Example ===\n");
    
    const char* device_name = "MyDevice-123";
    
    // เขียน string (ต้อง erase page ก่อน)
    Flash_ErasePage(FLASH_DATA_PAGE);
    Flash_WriteString(FLASH_DATA_ADDR, device_name);
    printf("Saved device name: %s\n", device_name);
    
    // อ่าน string
    char loaded_name[32];
    Flash_ReadString(FLASH_DATA_ADDR, loaded_name, sizeof(loaded_name));
    printf("Loaded device name: %s\n", loaded_name);
}

/**
 * @brief ตัวอย่างการใช้งานแบบครบวงจร
 */
void example_complete_workflow(void) {
    printf("\n=== Complete Workflow Example ===\n");
    
    Config_t config;
    
    // 1. ตรวจสอบว่ามี config หรือไม่
    if (Flash_IsConfigValid()) {
        printf("Step 1: Valid config found\n");
        
        // 2. โหลด config
        if (FLASH_LOAD_CONFIG(&config)) {
            printf("Step 2: Config loaded\n");
            printf("  Current brightness: %d\n", config.brightness);
            
            // 3. แก้ไขค่า
            config.brightness += 10;
            if (config.brightness > 100) config.brightness = 100;
            printf("Step 3: Brightness updated to %d\n", config.brightness);
            
            // 4. บันทึกกลับ
            FLASH_SAVE_CONFIG(&config);
            printf("Step 4: Config saved\n");
        }
    } else {
        printf("Step 1: No valid config, using defaults\n");
        
        // สร้าง default config
        config.magic = CONFIG_MAGIC;
        config.brightness = 50;
        config.volume = 75;
        
        FLASH_SAVE_CONFIG(&config);
        printf("Step 2: Default config saved\n");
    }
    
    printf("✓ Workflow completed!\n");
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
    printf("  SimpleFlash - Simple Usage Example\n");
    printf("========================================\n");
    printf("\nใช้งานง่ายด้วย Macro และ Auto-erase!\n");
    
    // เริ่มต้น Flash
    Flash_Init();
    
    // รันตัวอย่างต่างๆ
    example_simple_config();
    Delay_Ms(500);
    
    example_simple_read_write();
    Delay_Ms(500);
    
    example_simple_string();
    Delay_Ms(500);
    
    example_complete_workflow();
    
    printf("\n========================================\n");
    printf("  All examples completed!\n");
    printf("========================================\n");
    
    printf("\n📝 Key Points:\n");
    printf("  ✓ ใช้ FLASH_SAVE_CONFIG() และ FLASH_LOAD_CONFIG()\n");
    printf("  ✓ ไม่ต้องคำนวณ size เอง\n");
    printf("  ✓ ใช้ FLASH_WRITE_AUTO() สำหรับเขียนแบบง่าย\n");
    printf("  ✓ ใช้ FLASH_READ() สำหรับอ่านแบบง่าย\n");
    
    while(1) {
        Delay_Ms(1000);
    }
}
