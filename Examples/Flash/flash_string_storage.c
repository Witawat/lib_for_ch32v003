/**
 * @file flash_string_storage.c
 * @brief ตัวอย่างการจัดเก็บ String/Text ใน Flash
 * @date 2025-12-21
 * 
 * @details
 * ตัวอย่างนี้แสดงวิธีการเก็บข้อความต่างๆ ใน Flash เช่น:
 * - ชื่อผู้ใช้
 * - Device name
 * - WiFi SSID/Password
 * - Log messages
 */

#include "debug.h"
#include "SimpleHAL/SimpleFlash.h"
#include <stdio.h>

// Address offsets สำหรับเก็บ string ต่างๆ
#define ADDR_DEVICE_NAME    (FLASH_DATA_ADDR + 0)
#define ADDR_USER_NAME      (FLASH_DATA_ADDR + 32)

/**
 * @brief ตัวอย่างการเขียนและอ่าน string
 */
void example_basic_string(void) {
    printf("\n=== Basic String Storage ===\n");
    
    // ลบ page
    Flash_ErasePage(FLASH_DATA_PAGE);
    
    // เขียน string
    const char* device_name = "CH32V003-Device";
    printf("Writing device name: \"%s\"\n", device_name);
    
    FlashStatus status = Flash_WriteString(ADDR_DEVICE_NAME, device_name);
    if (status != FLASH_OK) {
        printf("Error writing string: %d\n", status);
        return;
    }
    
    // อ่าน string กลับมา
    char read_buffer[32];
    uint16_t len = Flash_ReadString(ADDR_DEVICE_NAME, read_buffer, sizeof(read_buffer));
    
    printf("Read device name: \"%s\" (length: %d)\n", read_buffer, len);
    
    // ตรวจสอบ
    if (strcmp(read_buffer, device_name) == 0) {
        printf("✓ String storage successful!\n");
    } else {
        printf("✗ String verification failed!\n");
    }
}

/**
 * @brief ตัวอย่างการเก็บหลาย string
 */
void example_multiple_strings(void) {
    printf("\n=== Multiple String Storage ===\n");
    
    // ลบ page
    Flash_ErasePage(FLASH_DATA_PAGE);
    
    // เขียนหลาย string
    const char* device_name = "MyDevice";
    const char* user_name = "Admin";
    
    printf("Writing multiple strings...\n");
    Flash_WriteString(ADDR_DEVICE_NAME, device_name);
    Flash_WriteString(ADDR_USER_NAME, user_name);
    
    // อ่านกลับมา
    char dev_buffer[32];
    char user_buffer[32];
    
    Flash_ReadString(ADDR_DEVICE_NAME, dev_buffer, sizeof(dev_buffer));
    Flash_ReadString(ADDR_USER_NAME, user_buffer, sizeof(user_buffer));
    
    printf("Device Name: \"%s\"\n", dev_buffer);
    printf("User Name: \"%s\"\n", user_buffer);
    
    // ตรวจสอบ
    if (strcmp(dev_buffer, device_name) == 0 && strcmp(user_buffer, user_name) == 0) {
        printf("✓ Multiple strings stored successfully!\n");
    }
}

/**
 * @brief ตัวอย่างการจัดเก็บ WiFi credentials
 */
void example_wifi_credentials(void) {
    printf("\n=== WiFi Credentials Storage ===\n");
    
    // Structure สำหรับ WiFi credentials
    typedef struct {
        char ssid[32];
        char password[32];
    } WiFiCredentials_t;
    
    WiFiCredentials_t wifi = {
        .ssid = "MyWiFiNetwork",
        .password = "SecurePassword123"
    };
    
    // ลบ page
    Flash_ErasePage(FLASH_DATA_PAGE);
    
    // บันทึก WiFi credentials
    printf("Saving WiFi credentials...\n");
    printf("  SSID: %s\n", wifi.ssid);
    printf("  Password: %s\n", wifi.password);
    
    FlashStatus status = Flash_WriteStruct(FLASH_DATA_ADDR, &wifi, sizeof(wifi));
    if (status != FLASH_OK) {
        printf("Error saving credentials: %d\n", status);
        return;
    }
    
    // โหลด WiFi credentials
    WiFiCredentials_t loaded_wifi;
    status = Flash_ReadStruct(FLASH_DATA_ADDR, &loaded_wifi, sizeof(loaded_wifi));
    
    if (status == FLASH_OK) {
        printf("\nLoaded WiFi credentials:\n");
        printf("  SSID: %s\n", loaded_wifi.ssid);
        printf("  Password: %s\n", loaded_wifi.password);
        
        // ตรวจสอบ
        if (strcmp(loaded_wifi.ssid, wifi.ssid) == 0 && 
            strcmp(loaded_wifi.password, wifi.password) == 0) {
            printf("✓ WiFi credentials stored successfully!\n");
        }
    }
}

/**
 * @brief ตัวอย่างการแก้ไข string ที่มีอยู่
 */
void example_modify_string(void) {
    printf("\n=== Modifying Stored String ===\n");
    
    // เขียน string เริ่มต้น
    Flash_ErasePage(FLASH_DATA_PAGE);
    const char* original = "Version 1.0";
    Flash_WriteString(FLASH_DATA_ADDR, original);
    
    printf("Original string: \"%s\"\n", original);
    
    // อ่านกลับมา
    char buffer[32];
    Flash_ReadString(FLASH_DATA_ADDR, buffer, sizeof(buffer));
    printf("Read: \"%s\"\n", buffer);
    
    // แก้ไข string (ต้อง erase page ก่อน)
    const char* updated = "Version 2.0";
    printf("\nUpdating to: \"%s\"\n", updated);
    
    Flash_ErasePage(FLASH_DATA_PAGE);
    Flash_WriteString(FLASH_DATA_ADDR, updated);
    
    // อ่านกลับมาอีกครั้ง
    Flash_ReadString(FLASH_DATA_ADDR, buffer, sizeof(buffer));
    printf("Updated string: \"%s\"\n", buffer);
    
    if (strcmp(buffer, updated) == 0) {
        printf("✓ String updated successfully!\n");
    }
}

/**
 * @brief ตัวอย่างการตรวจสอบความยาว string
 */
void example_string_length_check(void) {
    printf("\n=== String Length Check ===\n");
    
    // ทดสอบ string ที่ยาวเกินไป
    char long_string[100];
    memset(long_string, 'A', sizeof(long_string) - 1);
    long_string[sizeof(long_string) - 1] = '\0';
    
    printf("Attempting to write %d character string...\n", strlen(long_string));
    
    Flash_ErasePage(FLASH_DATA_PAGE);
    FlashStatus status = Flash_WriteString(FLASH_DATA_ADDR, long_string);
    
    if (status == FLASH_ERROR_RANGE) {
        printf("✓ Correctly rejected string longer than %d characters\n", 
               FLASH_MAX_STRING_LENGTH);
    } else if (status == FLASH_OK) {
        printf("String written (truncated to max length)\n");
    }
    
    // ทดสอบ string ที่ความยาวพอดี
    char valid_string[FLASH_MAX_STRING_LENGTH];
    memset(valid_string, 'B', FLASH_MAX_STRING_LENGTH - 1);
    valid_string[FLASH_MAX_STRING_LENGTH - 1] = '\0';
    
    printf("\nWriting %d character string...\n", strlen(valid_string));
    Flash_ErasePage(FLASH_DATA_PAGE);
    status = Flash_WriteString(FLASH_DATA_ADDR, valid_string);
    
    if (status == FLASH_OK) {
        char read_buffer[FLASH_MAX_STRING_LENGTH + 10];
        Flash_ReadString(FLASH_DATA_ADDR, read_buffer, sizeof(read_buffer));
        printf("✓ Max length string stored successfully (%d chars)\n", strlen(read_buffer));
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
    printf("  SimpleFlash String Storage Example\n");
    printf("========================================\n");
    
    // เริ่มต้น Flash
    Flash_Init();
    
    printf("\nMax string length: %d characters\n", FLASH_MAX_STRING_LENGTH);
    
    // รันตัวอย่างต่างๆ
    example_basic_string();
    Delay_Ms(500);
    
    example_multiple_strings();
    Delay_Ms(500);
    
    example_wifi_credentials();
    Delay_Ms(500);
    
    example_modify_string();
    Delay_Ms(500);
    
    example_string_length_check();
    
    printf("\n========================================\n");
    printf("  All examples completed!\n");
    printf("========================================\n");
    
    while(1) {
        Delay_Ms(1000);
    }
}
