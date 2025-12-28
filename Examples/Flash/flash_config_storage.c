/**
 * @file flash_config_storage.c
 * @brief ตัวอย่างการจัดเก็บ Configuration ใน Flash
 * @date 2025-12-21
 * 
 * @details
 * ตัวอย่างนี้แสดงวิธีการใช้ Flash เพื่อเก็บ configuration ของระบบ
 * เช่น การตั้งค่า brightness, volume, WiFi credentials, calibration values
 * พร้อมระบบ CRC validation เพื่อตรวจสอบความถูกต้อง
 */

#include "debug.h"
#include "SimpleHAL/SimpleFlash.h"

/**
 * @brief Configuration structure
 * @note CRC field ต้องเป็น field สุดท้ายเสมอ
 */
typedef struct {
    uint32_t magic;          // Magic number สำหรับตรวจสอบว่าเป็น valid config
    uint16_t version;        // Config version
    uint16_t brightness;     // ความสว่าง LED (0-100)
    uint16_t volume;         // ระดับเสียง (0-100)
    uint32_t boot_count;     // จำนวนครั้งที่ boot
    uint16_t crc;            // CRC16 checksum (ต้องเป็น field สุดท้าย)
} SystemConfig_t;

// Magic number สำหรับตรวจสอบ valid config
#define CONFIG_MAGIC    0x12345678
#define CONFIG_VERSION  1

// Default configuration
const SystemConfig_t default_config = {
    .magic = CONFIG_MAGIC,
    .version = CONFIG_VERSION,
    .brightness = 50,
    .volume = 75,
    .boot_count = 0,
    .crc = 0  // จะถูกคำนวณตอนบันทึก
};

/**
 * @brief โหลด configuration จาก Flash
 * @param config pointer ไปยัง config struct
 * @return true ถ้าโหลดสำเร็จ, false ถ้าไม่มี config หรือ CRC ผิด
 */
bool load_config(SystemConfig_t* config) {
    printf("Loading configuration from Flash...\n");
    
    // โหลด config (ไม่รวม CRC field)
    bool success = Flash_LoadConfig(config, sizeof(SystemConfig_t) - sizeof(config->crc));
    
    if (success) {
        // ตรวจสอบ magic number
        if (config->magic != CONFIG_MAGIC) {
            printf("Invalid magic number: 0x%08X\n", config->magic);
            return false;
        }
        
        // ตรวจสอบ version
        if (config->version != CONFIG_VERSION) {
            printf("Config version mismatch: %d (expected %d)\n", 
                   config->version, CONFIG_VERSION);
            return false;
        }
        
        printf("✓ Configuration loaded successfully!\n");
        printf("  Brightness: %d%%\n", config->brightness);
        printf("  Volume: %d%%\n", config->volume);
        printf("  Boot count: %d\n", config->boot_count);
        return true;
    } else {
        printf("✗ Failed to load config (CRC error or empty)\n");
        return false;
    }
}

/**
 * @brief บันทึก configuration ลง Flash
 * @param config pointer ไปยัง config struct
 * @return true ถ้าบันทึกสำเร็จ
 */
bool save_config(const SystemConfig_t* config) {
    printf("Saving configuration to Flash...\n");
    
    // บันทึก config (CRC จะถูกคำนวณอัตโนมัติ)
    FlashStatus status = Flash_SaveConfig(config, sizeof(SystemConfig_t) - sizeof(config->crc));
    
    if (status == FLASH_OK) {
        printf("✓ Configuration saved successfully!\n");
        return true;
    } else {
        printf("✗ Failed to save config: %d\n", status);
        return false;
    }
}

/**
 * @brief แสดง configuration ปัจจุบัน
 */
void print_config(const SystemConfig_t* config) {
    printf("\n--- Current Configuration ---\n");
    printf("Magic: 0x%08X\n", config->magic);
    printf("Version: %d\n", config->version);
    printf("Brightness: %d%%\n", config->brightness);
    printf("Volume: %d%%\n", config->volume);
    printf("Boot Count: %d\n", config->boot_count);
    printf("CRC: 0x%04X\n", config->crc);
    printf("-----------------------------\n\n");
}

/**
 * @brief ตัวอย่างการใช้งาน configuration
 */
void example_config_usage(void) {
    SystemConfig_t config;
    
    // พยายามโหลด config จาก Flash
    if (load_config(&config)) {
        // มี config อยู่แล้ว - เพิ่ม boot count
        config.boot_count++;
        printf("System boot #%d\n", config.boot_count);
        
        // บันทึก config ที่อัปเดตแล้ว
        save_config(&config);
    } else {
        // ไม่มี config - ใช้ค่า default
        printf("No valid config found, using defaults\n");
        config = default_config;
        config.boot_count = 1;
        
        // บันทึก default config
        save_config(&config);
    }
    
    // แสดง config ปัจจุบัน
    print_config(&config);
}

/**
 * @brief ตัวอย่างการแก้ไข configuration
 */
void example_modify_config(void) {
    SystemConfig_t config;
    
    printf("\n=== Modifying Configuration ===\n");
    
    // โหลด config ปัจจุบัน
    if (!load_config(&config)) {
        printf("Cannot modify - no config found\n");
        return;
    }
    
    // แก้ไขค่า
    printf("Changing brightness from %d to 80\n", config.brightness);
    config.brightness = 80;
    
    printf("Changing volume from %d to 60\n", config.volume);
    config.volume = 60;
    
    // บันทึกกลับ
    save_config(&config);
    
    // โหลดมาตรวจสอบอีกครั้ง
    SystemConfig_t verify_config;
    if (load_config(&verify_config)) {
        printf("\nVerification:\n");
        printf("  Brightness: %d (expected 80)\n", verify_config.brightness);
        printf("  Volume: %d (expected 60)\n", verify_config.volume);
        
        if (verify_config.brightness == 80 && verify_config.volume == 60) {
            printf("✓ Configuration modified successfully!\n");
        }
    }
}

/**
 * @brief ตัวอย่าง Factory Reset
 */
void example_factory_reset(void) {
    printf("\n=== Factory Reset ===\n");
    
    // ลบ config page
    printf("Erasing configuration...\n");
    FlashStatus status = Flash_ErasePage(FLASH_CONFIG_PAGE);
    
    if (status == FLASH_OK) {
        printf("✓ Configuration erased\n");
        
        // บันทึก default config
        printf("Restoring default configuration...\n");
        SystemConfig_t config = default_config;
        config.boot_count = 1;
        save_config(&config);
        
        printf("✓ Factory reset completed!\n");
    } else {
        printf("✗ Factory reset failed: %d\n", status);
    }
}

/**
 * @brief ตัวอย่างการตรวจสอบ config ก่อนโหลด
 */
void example_check_config(void) {
    printf("\n=== Checking Configuration ===\n");
    
    if (Flash_IsConfigValid()) {
        printf("✓ Valid configuration found in Flash\n");
        
        SystemConfig_t config;
        if (load_config(&config)) {
            print_config(&config);
        }
    } else {
        printf("✗ No valid configuration found\n");
        printf("Flash may be empty or corrupted\n");
    }
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
    printf("  SimpleFlash Configuration Storage\n");
    printf("========================================\n");
    
    // เริ่มต้น Flash
    Flash_Init();
    
    // ตัวอย่างการใช้งาน
    example_check_config();
    Delay_Ms(1000);
    
    example_config_usage();
    Delay_Ms(1000);
    
    example_modify_config();
    Delay_Ms(1000);
    
    // ถ้าต้องการทดสอบ factory reset ให้ uncomment บรรทัดนี้
    // example_factory_reset();
    
    printf("\n========================================\n");
    printf("  Example completed!\n");
    printf("  Reset the MCU to see boot count increase\n");
    printf("========================================\n");
    
    while(1) {
        Delay_Ms(1000);
    }
}
