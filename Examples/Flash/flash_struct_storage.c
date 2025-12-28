/**
 * @file flash_struct_storage.c
 * @brief ตัวอย่างการจัดเก็บ Struct ใน Flash (ขั้นกลาง)
 * @date 2025-12-21
 * 
 * @details
 * ตัวอย่างนี้แสดงเทคนิคการจัดเก็บ struct ที่ซับซ้อนใน Flash
 * รวมถึงการจัดการ nested structs, arrays, และ data validation
 */

#include "debug.h"
#include "SimpleHAL/SimpleFlash.h"
#include <stdio.h>

/**
 * @brief Sensor calibration data
 */
typedef struct {
    float offset;
    float scale;
    uint16_t zero_point;
} SensorCalibration_t;

/**
 * @brief Device settings
 */
typedef struct {
    uint32_t magic;                    // Magic number
    uint16_t version;                  // Version
    char device_name[20];              // Device name
    SensorCalibration_t temp_cal;      // Temperature sensor calibration
    SensorCalibration_t pressure_cal;  // Pressure sensor calibration
    uint8_t sensor_count;              // Number of sensors
    uint16_t sample_rate;              // Sample rate (Hz)
    uint16_t crc;                      // CRC checksum
} DeviceSettings_t;

#define SETTINGS_MAGIC   0xABCD1234
#define SETTINGS_VERSION 1

/**
 * @brief ตัวอย่างการบันทึก struct ที่ซับซ้อน
 */
void example_complex_struct(void) {
    printf("\n=== Complex Struct Storage ===\n");
    
    // สร้าง settings
    DeviceSettings_t settings = {
        .magic = SETTINGS_MAGIC,
        .version = SETTINGS_VERSION,
        .device_name = "TempSensor-01",
        .temp_cal = {
            .offset = -2.5f,
            .scale = 1.02f,
            .zero_point = 512
        },
        .pressure_cal = {
            .offset = 0.1f,
            .scale = 0.98f,
            .zero_point = 1024
        },
        .sensor_count = 2,
        .sample_rate = 100
    };
    
    printf("Saving device settings:\n");
    printf("  Device: %s\n", settings.device_name);
    printf("  Temp cal: offset=%.2f, scale=%.2f\n", 
           settings.temp_cal.offset, settings.temp_cal.scale);
    printf("  Pressure cal: offset=%.2f, scale=%.2f\n", 
           settings.pressure_cal.offset, settings.pressure_cal.scale);
    printf("  Sample rate: %d Hz\n", settings.sample_rate);
    
    // บันทึก (ไม่รวม CRC)
    FlashStatus status = Flash_SaveConfig(&settings, sizeof(settings) - sizeof(settings.crc));
    
    if (status == FLASH_OK) {
        printf("✓ Settings saved successfully!\n");
        
        // โหลดกลับมา
        DeviceSettings_t loaded;
        if (Flash_LoadConfig(&loaded, sizeof(loaded) - sizeof(loaded.crc))) {
            printf("\nLoaded settings:\n");
            printf("  Device: %s\n", loaded.device_name);
            printf("  Temp cal: offset=%.2f, scale=%.2f\n", 
                   loaded.temp_cal.offset, loaded.temp_cal.scale);
            printf("  Sample rate: %d Hz\n", loaded.sample_rate);
            
            // ตรวจสอบ
            if (loaded.magic == SETTINGS_MAGIC && 
                strcmp(loaded.device_name, settings.device_name) == 0) {
                printf("✓ Settings verified successfully!\n");
            }
        }
    }
}

/**
 * @brief ตัวอย่างการจัดเก็บ array of structs
 */
void example_struct_array(void) {
    printf("\n=== Struct Array Storage ===\n");
    
    // Sensor reading structure
    typedef struct {
        uint32_t timestamp;
        float temperature;
        float humidity;
        uint8_t status;
    } SensorReading_t;
    
    // Array of readings
    #define MAX_READINGS 5
    SensorReading_t readings[MAX_READINGS] = {
        {.timestamp = 1000, .temperature = 25.5f, .humidity = 60.0f, .status = 1},
        {.timestamp = 2000, .temperature = 26.0f, .humidity = 61.5f, .status = 1},
        {.timestamp = 3000, .temperature = 26.5f, .humidity = 62.0f, .status = 1},
        {.timestamp = 4000, .temperature = 27.0f, .humidity = 63.0f, .status = 1},
        {.timestamp = 5000, .temperature = 27.5f, .humidity = 64.0f, .status = 1}
    };
    
    printf("Saving %d sensor readings...\n", MAX_READINGS);
    
    // ลบ page
    Flash_ErasePage(FLASH_DATA_PAGE);
    
    // บันทึก array
    FlashStatus status = Flash_WriteStruct(FLASH_DATA_ADDR, readings, sizeof(readings));
    
    if (status == FLASH_OK) {
        printf("✓ Readings saved\n");
        
        // โหลดกลับมา
        SensorReading_t loaded_readings[MAX_READINGS];
        Flash_ReadStruct(FLASH_DATA_ADDR, loaded_readings, sizeof(loaded_readings));
        
        printf("\nLoaded readings:\n");
        for (int i = 0; i < MAX_READINGS; i++) {
            printf("  [%d] Time:%d, Temp:%.1f°C, Humidity:%.1f%%\n",
                   i, loaded_readings[i].timestamp,
                   loaded_readings[i].temperature,
                   loaded_readings[i].humidity);
        }
        
        // ตรวจสอบ
        bool match = true;
        for (int i = 0; i < MAX_READINGS; i++) {
            if (loaded_readings[i].timestamp != readings[i].timestamp) {
                match = false;
                break;
            }
        }
        
        if (match) {
            printf("✓ Array verified successfully!\n");
        }
    }
}

/**
 * @brief ตัวอย่างการ update บางส่วนของ struct
 */
void example_partial_update(void) {
    printf("\n=== Partial Struct Update ===\n");
    
    typedef struct {
        uint32_t id;
        uint16_t counter;
        uint8_t flags;
        uint8_t reserved;
    } SimpleData_t;
    
    // เขียนข้อมูลเริ่มต้น
    Flash_ErasePage(FLASH_DATA_PAGE);
    
    SimpleData_t data = {
        .id = 0x12345678,
        .counter = 0,
        .flags = 0x01,
        .reserved = 0
    };
    
    Flash_WriteStruct(FLASH_DATA_ADDR, &data, sizeof(data));
    printf("Initial data: ID=0x%08X, Counter=%d, Flags=0x%02X\n",
           data.id, data.counter, data.flags);
    
    // Update เฉพาะ counter (ใช้ WriteWithErase)
    for (int i = 1; i <= 3; i++) {
        printf("\nIncrementing counter to %d...\n", i);
        
        // อ่าน struct ปัจจุบัน
        SimpleData_t current;
        Flash_ReadStruct(FLASH_DATA_ADDR, &current, sizeof(current));
        
        // แก้ไข counter
        current.counter = i;
        
        // เขียนกลับ (ต้อง erase ก่อน)
        Flash_ErasePage(FLASH_DATA_PAGE);
        Flash_WriteStruct(FLASH_DATA_ADDR, &current, sizeof(current));
        
        // ตรวจสอบ
        SimpleData_t verify;
        Flash_ReadStruct(FLASH_DATA_ADDR, &verify, sizeof(verify));
        printf("  Verified: ID=0x%08X, Counter=%d, Flags=0x%02X\n",
               verify.id, verify.counter, verify.flags);
    }
    
    printf("✓ Partial updates successful!\n");
}

/**
 * @brief ตัวอย่างการใช้ struct กับ CRC validation
 */
void example_struct_with_crc(void) {
    printf("\n=== Struct with CRC Validation ===\n");
    
    typedef struct {
        uint32_t serial_number;
        uint16_t production_year;
        uint8_t hardware_version;
        uint8_t firmware_version;
        uint16_t crc;  // ต้องเป็น field สุดท้าย
    } ProductInfo_t;
    
    ProductInfo_t info = {
        .serial_number = 123456,
        .production_year = 2025,
        .hardware_version = 1,
        .firmware_version = 2
    };
    
    printf("Saving product info with CRC...\n");
    printf("  Serial: %d\n", info.serial_number);
    printf("  Year: %d\n", info.production_year);
    printf("  HW Ver: %d, FW Ver: %d\n", info.hardware_version, info.firmware_version);
    
    // บันทึกพร้อม CRC
    FlashStatus status = Flash_SaveConfig(&info, sizeof(info) - sizeof(info.crc));
    
    if (status == FLASH_OK) {
        printf("✓ Product info saved with CRC\n");
        
        // โหลดและตรวจสอบ CRC
        ProductInfo_t loaded;
        if (Flash_LoadConfig(&loaded, sizeof(loaded) - sizeof(loaded.crc))) {
            printf("\n✓ CRC validation passed!\n");
            printf("Loaded product info:\n");
            printf("  Serial: %d\n", loaded.serial_number);
            printf("  Year: %d\n", loaded.production_year);
        } else {
            printf("✗ CRC validation failed!\n");
        }
        
        // ทดสอบการตรวจจับข้อมูลเสียหาย
        printf("\nSimulating data corruption...\n");
        Flash_WriteByteWithErase(FLASH_CONFIG_ADDR + 2, 0xFF);
        
        ProductInfo_t corrupted;
        if (!Flash_LoadConfig(&corrupted, sizeof(corrupted) - sizeof(corrupted.crc))) {
            printf("✓ Corruption detected by CRC!\n");
        }
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
    printf("  SimpleFlash Struct Storage Example\n");
    printf("========================================\n");
    
    // เริ่มต้น Flash
    Flash_Init();
    
    // รันตัวอย่างต่างๆ
    example_complex_struct();
    Delay_Ms(1000);
    
    example_struct_array();
    Delay_Ms(1000);
    
    example_partial_update();
    Delay_Ms(1000);
    
    example_struct_with_crc();
    
    printf("\n========================================\n");
    printf("  All examples completed!\n");
    printf("========================================\n");
    
    while(1) {
        Delay_Ms(1000);
    }
}
