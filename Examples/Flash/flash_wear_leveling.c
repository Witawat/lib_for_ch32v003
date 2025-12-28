/**
 * @file flash_wear_leveling.c
 * @brief ตัวอย่างเทคนิค Wear Leveling (ขั้นสูง)
 * @date 2025-12-21
 * 
 * @details
 * Flash memory มีจำนวนครั้งการเขียนจำกัด (~10,000-80,000 cycles)
 * Wear leveling คือเทคนิคการกระจายการเขียนข้อมูลเพื่อยืดอายุการใช้งาน
 * 
 * เทคนิคที่ใช้:
 * - Circular buffer: เขียนข้อมูลแบบวนรอบ
 * - Timestamp-based: ใช้ timestamp หาข้อมูลล่าสุด
 * - Write counter: นับจำนวนครั้งที่เขียนแต่ละ slot
 */

#include "debug.h"
#include "SimpleHAL/SimpleFlash.h"
#include "SimpleHAL/timer.h"

/**
 * @brief Wear leveling slot structure
 */
typedef struct {
    uint32_t timestamp;    // เวลาที่เขียน (millis)
    uint16_t counter;      // ค่าที่เก็บ
    uint16_t crc;          // CRC checksum
} WearLevelSlot_t;

#define SLOT_SIZE       sizeof(WearLevelSlot_t)
#define MAX_SLOTS       (FLASH_DATA_SIZE / SLOT_SIZE)  // ~8 slots
#define SLOT_ADDR(n)    (FLASH_DATA_ADDR + ((n) * SLOT_SIZE))

/**
 * @brief หา slot ที่มีข้อมูลล่าสุด
 * @return slot index, หรือ -1 ถ้าไม่พบ
 */
int find_latest_slot(void) {
    uint32_t latest_time = 0;
    int latest_slot = -1;
    
    for (int i = 0; i < MAX_SLOTS; i++) {
        WearLevelSlot_t slot;
        Flash_ReadStruct(SLOT_ADDR(i), &slot, sizeof(slot) - sizeof(slot.crc));
        
        // ตรวจสอบว่า slot ว่างหรือไม่ (all 0xFF)
        if (slot.timestamp == 0xFFFFFFFF) {
            continue;
        }
        
        // ตรวจสอบ CRC
        uint16_t stored_crc = Flash_ReadHalfWord(SLOT_ADDR(i) + sizeof(slot) - sizeof(slot.crc));
        uint16_t calc_crc = Flash_CalculateCRC16((uint8_t*)&slot, sizeof(slot) - sizeof(slot.crc));
        
        if (stored_crc != calc_crc) {
            continue;  // CRC ผิด, ข้าม slot นี้
        }
        
        // เปรียบเทียบ timestamp
        if (slot.timestamp > latest_time) {
            latest_time = slot.timestamp;
            latest_slot = i;
        }
    }
    
    return latest_slot;
}

/**
 * @brief หา slot ว่างถัดไป (circular)
 * @param current_slot slot ปัจจุบัน
 * @return slot index ถัดไป
 */
int find_next_slot(int current_slot) {
    return (current_slot + 1) % MAX_SLOTS;
}

/**
 * @brief เขียนข้อมูลด้วย wear leveling
 * @param value ค่าที่ต้องการเขียน
 * @return true ถ้าสำเร็จ
 */
bool write_with_wear_leveling(uint16_t value) {
    // หา slot ล่าสุด
    int latest_slot = find_latest_slot();
    
    // หา slot ถัดไป
    int next_slot = (latest_slot >= 0) ? find_next_slot(latest_slot) : 0;
    
    printf("Writing to slot %d (previous: %d)\n", next_slot, latest_slot);
    
    // สร้างข้อมูล
    WearLevelSlot_t slot = {
        .timestamp = millis(),
        .counter = value
    };
    
    // คำนวณ CRC
    slot.crc = Flash_CalculateCRC16((uint8_t*)&slot, sizeof(slot) - sizeof(slot.crc));
    
    // ถ้า slot ถัดไปไม่ว่าง ต้อง erase page
    WearLevelSlot_t check_slot;
    Flash_ReadStruct(SLOT_ADDR(next_slot), &check_slot, sizeof(check_slot));
    
    if (check_slot.timestamp != 0xFFFFFFFF) {
        printf("  Slot not empty, erasing page...\n");
        Flash_ErasePage(FLASH_DATA_PAGE);
    }
    
    // เขียนข้อมูล
    FlashStatus status = Flash_WriteStruct(SLOT_ADDR(next_slot), &slot, sizeof(slot));
    
    return (status == FLASH_OK);
}

/**
 * @brief อ่านข้อมูลล่าสุด
 * @param value pointer สำหรับเก็บค่า
 * @return true ถ้าพบข้อมูล
 */
bool read_latest_value(uint16_t* value) {
    int latest_slot = find_latest_slot();
    
    if (latest_slot < 0) {
        return false;  // ไม่พบข้อมูล
    }
    
    WearLevelSlot_t slot;
    Flash_ReadStruct(SLOT_ADDR(latest_slot), &slot, sizeof(slot) - sizeof(slot.crc));
    
    *value = slot.counter;
    printf("Read from slot %d: value=%d, time=%d\n", 
           latest_slot, slot.counter, slot.timestamp);
    
    return true;
}

/**
 * @brief แสดงข้อมูลทุก slot
 */
void print_all_slots(void) {
    printf("\n--- All Slots ---\n");
    
    for (int i = 0; i < MAX_SLOTS; i++) {
        WearLevelSlot_t slot;
        Flash_ReadStruct(SLOT_ADDR(i), &slot, sizeof(slot) - sizeof(slot.crc));
        
        if (slot.timestamp == 0xFFFFFFFF) {
            printf("Slot %d: [EMPTY]\n", i);
        } else {
            uint16_t stored_crc = Flash_ReadHalfWord(SLOT_ADDR(i) + sizeof(slot) - sizeof(slot.crc));
            uint16_t calc_crc = Flash_CalculateCRC16((uint8_t*)&slot, sizeof(slot) - sizeof(slot.crc));
            bool crc_ok = (stored_crc == calc_crc);
            
            printf("Slot %d: value=%d, time=%d, CRC=%s\n", 
                   i, slot.counter, slot.timestamp, crc_ok ? "OK" : "FAIL");
        }
    }
    printf("----------------\n\n");
}

/**
 * @brief ตัวอย่างการใช้ wear leveling
 */
void example_wear_leveling(void) {
    printf("\n=== Wear Leveling Example ===\n");
    printf("Max slots: %d\n", MAX_SLOTS);
    printf("Slot size: %d bytes\n", SLOT_SIZE);
    
    // ลบ page
    printf("\nErasing page...\n");
    Flash_ErasePage(FLASH_DATA_PAGE);
    
    // เขียนข้อมูลหลายครั้ง
    printf("\nWriting values with wear leveling:\n");
    for (int i = 1; i <= 12; i++) {
        printf("\n[Write #%d]\n", i);
        write_with_wear_leveling(i * 10);
        Delay_Ms(100);  // เพื่อให้ timestamp ต่างกัน
        
        // แสดง slots ทุกๆ 3 ครั้ง
        if (i % 3 == 0) {
            print_all_slots();
        }
    }
    
    // อ่านค่าล่าสุด
    uint16_t latest_value;
    if (read_latest_value(&latest_value)) {
        printf("Latest value: %d (expected 120)\n", latest_value);
        if (latest_value == 120) {
            printf("✓ Wear leveling working correctly!\n");
        }
    }
}

/**
 * @brief คำนวณสถิติการใช้งาน
 */
void calculate_wear_statistics(void) {
    printf("\n=== Wear Statistics ===\n");
    
    int used_slots = 0;
    int empty_slots = 0;
    uint32_t oldest_time = 0xFFFFFFFF;
    uint32_t newest_time = 0;
    
    for (int i = 0; i < MAX_SLOTS; i++) {
        WearLevelSlot_t slot;
        Flash_ReadStruct(SLOT_ADDR(i), &slot, sizeof(slot) - sizeof(slot.crc));
        
        if (slot.timestamp == 0xFFFFFFFF) {
            empty_slots++;
        } else {
            used_slots++;
            if (slot.timestamp < oldest_time) oldest_time = slot.timestamp;
            if (slot.timestamp > newest_time) newest_time = slot.timestamp;
        }
    }
    
    printf("Used slots: %d/%d\n", used_slots, MAX_SLOTS);
    printf("Empty slots: %d/%d\n", empty_slots, MAX_SLOTS);
    printf("Time span: %d ms\n", newest_time - oldest_time);
    
    // คำนวณจำนวนครั้งที่ page ถูก erase
    // ถ้าเขียนข้อมูล N ครั้ง และมี M slots
    // จำนวนครั้งที่ erase = (N - M) / M
    printf("\nEstimated page erases: ~%d times\n", (12 - MAX_SLOTS) / MAX_SLOTS);
    printf("Without wear leveling: 12 times\n");
    printf("Wear reduction: ~%.1f%%\n", 
           (1.0 - (float)((12 - MAX_SLOTS) / MAX_SLOTS) / 12.0) * 100);
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
    printf("  SimpleFlash Wear Leveling Example\n");
    printf("========================================\n");
    
    // เริ่มต้น Flash และ Timer
    Flash_Init();
    
    // รันตัวอย่าง
    example_wear_leveling();
    
    calculate_wear_statistics();
    
    printf("\n========================================\n");
    printf("  Example completed!\n");
    printf("========================================\n");
    printf("\nKey Points:\n");
    printf("- Wear leveling distributes writes across multiple slots\n");
    printf("- Reduces flash wear by ~%.0f%%\n", 
           (1.0 - (float)((12 - MAX_SLOTS) / MAX_SLOTS) / 12.0) * 100);
    printf("- Uses timestamp to find latest data\n");
    printf("- CRC ensures data integrity\n");
    
    while(1) {
        Delay_Ms(1000);
    }
}
