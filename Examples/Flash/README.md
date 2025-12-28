# SimpleFlash Library - คู่มือการใช้งาน

> **Simple Flash Storage Library สำหรับ CH32V003**  
> จัดเก็บข้อมูล configuration และข้อความใน Flash memory แบบ non-volatile

---

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [การเริ่มต้นใช้งาน](#การเริ่มต้นใช้งาน)
3. [การใช้งานขั้นพื้นฐาน](#การใช้งานขั้นพื้นฐาน)
4. [การใช้งานขั้นกลาง](#การใช้งานขั้นกลาง)
5. [เทคนิคขั้นสูง](#เทคนิคขั้นสูง)
6. [Best Practices](#best-practices)
7. [API Reference](#api-reference)
8. [Troubleshooting](#troubleshooting)

---

## ภาพรวม

### คุณสมบัติหลัก

- ✅ อ่าน/เขียนข้อมูล byte, half-word (16-bit), word (32-bit)
- ✅ จัดเก็บ string และ struct
- ✅ Configuration management พร้อม CRC validation
- ✅ Wear leveling support
- ✅ Factory reset capability
- ✅ API ใช้งานง่ายแบบ Arduino-style

### ข้อจำกัดของ Flash Memory

> [!IMPORTANT]
> **Flash Memory Characteristics**
> - CH32V003 มี Flash 16KB (256 pages × 64 bytes/page)
> - Write endurance: ~10,000-80,000 cycles ต่อ page
> - ต้อง **erase ทั้ง page** (64 bytes) ก่อนเขียนข้อมูลใหม่
> - การเขียนทำได้ครั้งละ 16-bit หรือ 32-bit เท่านั้น

> [!WARNING]
> **สำหรับข้อมูลที่เปลี่ยนบ่อยมาก (>100 ครั้ง/วัน)**  
> แนะนำให้ใช้ external EEPROM แทน หรือใช้เทคนิค wear leveling

### Flash Storage Area

Library ใช้ **last 2 pages** (pages 254-255) สำหรับเก็บข้อมูล:

| Page | Address Range | Size | การใช้งาน |
|------|--------------|------|-----------|
| 254 | `0x0803F80 - 0x0803FBF` | 64 bytes | Configuration storage |
| 255 | `0x0803FC0 - 0x0803FFF` | 64 bytes | General data storage |

---

## การเริ่มต้นใช้งาน

### 1. การติดตั้ง

Include header file ในโปรแกรม:

```c
#include "SimpleHAL/SimpleFlash.h"
```

### 2. การเริ่มต้น Flash

```c
int main(void) {
    // เริ่มต้นระบบ
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น Flash
    Flash_Init();
    
    // พร้อมใช้งาน!
}
```

### 3. ตัวอย่างโค้ดพื้นฐาน

```c
// เขียนข้อมูล
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteWord(FLASH_DATA_ADDR, 0x12345678);

// อ่านข้อมูล
uint32_t value = Flash_ReadWord(FLASH_DATA_ADDR);
printf("Value: 0x%08X\n", value);
```

---

## การใช้งานแบบง่าย (Simplified API) ⭐

SimpleFlash มี **macros พิเศษ** ที่ทำให้ใช้งานง่ายขึ้นมาก ไม่ต้องคำนวณ size เอง!

### บันทึก/โหลด Configuration

```c
typedef struct {
    uint32_t magic;
    uint16_t brightness;
    uint16_t volume;
    uint16_t crc;  // ต้องเป็น field สุดท้าย!
} Config_t;

Config_t config;

// ❌ แบบเดิม (ซับซ้อน)
Flash_SaveConfig(&config, sizeof(config) - sizeof(config.crc));
Flash_LoadConfig(&config, sizeof(config) - sizeof(config.crc));

// ✅ แบบใหม่ (ง่าย) - ใช้ macro
FLASH_SAVE_CONFIG(&config);
FLASH_LOAD_CONFIG(&config);
```

### เขียน/อ่านข้อมูลแบบง่าย

```c
// ❌ แบบเดิม (ต้อง erase ก่อน)
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteWord(FLASH_DATA_ADDR, 0x12345678);

// ✅ แบบใหม่ (auto-erase)
FLASH_WRITE_AUTO(FLASH_DATA_ADDR, 0x12345678);

// อ่านข้อมูล (auto-detect type)
uint32_t value;
FLASH_READ(FLASH_DATA_ADDR, &value);
```

### ตัวอย่างการใช้งานจริง

```c
#include "SimpleHAL/SimpleHAL.h"

int main(void) {
    Flash_Init();
    
    Config_t config;
    
    // โหลด config (ง่ายมาก!)
    if (FLASH_LOAD_CONFIG(&config)) {
        printf("Loaded: brightness=%d\n", config.brightness);
        
        // แก้ไขค่า
        config.brightness = 80;
        
        // บันทึกกลับ (ง่ายมาก!)
        FLASH_SAVE_CONFIG(&config);
    } else {
        // ตั้งค่า default
        config.magic = 0x12345678;
        config.brightness = 50;
        config.volume = 75;
        
        FLASH_SAVE_CONFIG(&config);
    }
}
```

> [!TIP]
> **Simplified API Macros**
> - `FLASH_SAVE_CONFIG(ptr)` - บันทึก config ไม่ต้องคำนวณ size
> - `FLASH_LOAD_CONFIG(ptr)` - โหลด config ไม่ต้องคำนวณ size
> - `FLASH_WRITE_AUTO(addr, value)` - เขียนแบบ auto-erase (รองรับ uint8_t, uint16_t, uint32_t)
> - `FLASH_READ(addr, ptr)` - อ่านแบบ auto-detect type

---

## การใช้งานขั้นพื้นฐาน

### 1. การอ่าน/เขียนข้อมูลพื้นฐาน

#### เขียน/อ่าน Byte

```c
// ลบ page ก่อนเขียน
Flash_ErasePage(FLASH_DATA_PAGE);

// เขียน byte
Flash_WriteByte(FLASH_DATA_ADDR, 0xAB);

// อ่าน byte
uint8_t value = Flash_ReadByte(FLASH_DATA_ADDR);
```

#### เขียน/อ่าน Half-Word (16-bit)

```c
Flash_ErasePage(FLASH_DATA_PAGE);

// เขียน half-word (address ต้องเป็นเลขคู่)
Flash_WriteHalfWord(FLASH_DATA_ADDR, 0x1234);

// อ่าน half-word
uint16_t value = Flash_ReadHalfWord(FLASH_DATA_ADDR);
```

#### เขียน/อ่าน Word (32-bit)

```c
Flash_ErasePage(FLASH_DATA_PAGE);

// เขียน word (address ต้องหาร 4 ลงตัว)
Flash_WriteWord(FLASH_DATA_ADDR, 0x12345678);

// อ่าน word
uint32_t value = Flash_ReadWord(FLASH_DATA_ADDR);
```

> [!NOTE]
> **Address Alignment**
> - Byte: ไม่ต้อง align
> - Half-word: ต้อง align 2 bytes (เลขคู่)
> - Word: ต้อง align 4 bytes (หาร 4 ลงตัว)

### 2. การจัดเก็บ String

```c
// เขียน string
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteString(FLASH_DATA_ADDR, "Hello World");

// อ่าน string
char buffer[32];
uint16_t len = Flash_ReadString(FLASH_DATA_ADDR, buffer, sizeof(buffer));
printf("String: %s (length: %d)\n", buffer, len);
```

**ข้อจำกัด:**
- String ต้องไม่เกิน `FLASH_MAX_STRING_LENGTH` (60 characters)
- String ต้องเป็น null-terminated

### 3. การจัดเก็บ Configuration

```c
// กำหนด config structure
typedef struct {
    uint32_t magic;
    uint16_t brightness;
    uint16_t volume;
    uint16_t crc;  // ต้องเป็น field สุดท้าย!
} Config_t;

// บันทึก config
Config_t config = {
    .magic = 0x12345678,
    .brightness = 50,
    .volume = 75
};

Flash_SaveConfig(&config, sizeof(config) - sizeof(config.crc));

// โหลด config
Config_t loaded_config;
if (Flash_LoadConfig(&loaded_config, sizeof(loaded_config) - sizeof(loaded_config.crc))) {
    printf("Brightness: %d\n", loaded_config.brightness);
} else {
    printf("No valid config found\n");
}
```

> [!IMPORTANT]
> **CRC Field**
> - Struct ต้องมี `uint16_t crc` เป็น field **สุดท้าย**
> - ตอนบันทึก/โหลด ต้องระบุ size **ไม่รวม** CRC field
> - CRC จะถูกคำนวณและตรวจสอบอัตโนมัติ

---

## การใช้งานขั้นกลาง

### 1. การจัดเก็บ Struct

```c
typedef struct {
    uint32_t id;
    float temperature;
    float humidity;
    uint8_t status;
} SensorData_t;

// บันทึก struct
SensorData_t data = {
    .id = 123,
    .temperature = 25.5f,
    .humidity = 60.0f,
    .status = 1
};

Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteStruct(FLASH_DATA_ADDR, &data, sizeof(data));

// อ่าน struct
SensorData_t loaded_data;
Flash_ReadStruct(FLASH_DATA_ADDR, &loaded_data, sizeof(loaded_data));
```

### 2. การจัดเก็บ Array of Structs

```c
#define MAX_READINGS 5

typedef struct {
    uint32_t timestamp;
    float value;
} Reading_t;

Reading_t readings[MAX_READINGS];

// เติมข้อมูล
for (int i = 0; i < MAX_READINGS; i++) {
    readings[i].timestamp = millis();
    readings[i].value = 25.0f + i;
}

// บันทึก array
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteStruct(FLASH_DATA_ADDR, readings, sizeof(readings));

// โหลด array
Reading_t loaded_readings[MAX_READINGS];
Flash_ReadStruct(FLASH_DATA_ADDR, loaded_readings, sizeof(loaded_readings));
```

### 3. การแก้ไขข้อมูลบางส่วน (Modify-Erase-Write)

ถ้าต้องการแก้ไขข้อมูลบางส่วนโดยไม่ลบทั้ง page:

```c
// ใช้ WriteWithErase (ช้ากว่าปกติ แต่สะดวก)
Flash_WriteByteWithErase(FLASH_DATA_ADDR + 5, 0xAB);
Flash_WriteHalfWordWithErase(FLASH_DATA_ADDR + 10, 0x1234);
Flash_WriteWordWithErase(FLASH_DATA_ADDR + 20, 0x12345678);
```

> [!WARNING]
> **WriteWithErase ช้ากว่าปกติ**
> - ต้องอ่าน page ทั้งหมดมาเก็บใน RAM (64 bytes)
> - Erase page
> - เขียน page ทั้งหมดกลับ
> - ใช้เฉพาะเมื่อจำเป็น!

### 4. การตรวจสอบ Configuration

```c
// ตรวจสอบว่ามี valid config หรือไม่
if (Flash_IsConfigValid()) {
    // มี config - โหลดมาใช้
    Config_t config;
    Flash_LoadConfig(&config, sizeof(config) - 2);
} else {
    // ไม่มี config - ใช้ค่า default
    Config_t default_config = {...};
    Flash_SaveConfig(&default_config, sizeof(default_config) - 2);
}
```

---

## เทคนิคขั้นสูง

### 1. Wear Leveling

Flash มีจำนวนครั้งการเขียนจำกัด Wear leveling ช่วยกระจายการเขียนเพื่อยืดอายุการใช้งาน

#### เทคนิค Circular Buffer

```c
typedef struct {
    uint32_t timestamp;
    uint16_t value;
    uint16_t crc;
} Slot_t;

#define SLOT_SIZE sizeof(Slot_t)
#define MAX_SLOTS (FLASH_DATA_SIZE / SLOT_SIZE)  // ~8 slots

// หา slot ล่าสุด
int find_latest_slot(void) {
    uint32_t latest_time = 0;
    int latest_slot = -1;
    
    for (int i = 0; i < MAX_SLOTS; i++) {
        Slot_t slot;
        Flash_ReadStruct(FLASH_DATA_ADDR + (i * SLOT_SIZE), &slot, sizeof(slot) - 2);
        
        if (slot.timestamp == 0xFFFFFFFF) continue;  // ว่าง
        
        // ตรวจสอบ CRC
        uint16_t crc = Flash_ReadHalfWord(FLASH_DATA_ADDR + (i * SLOT_SIZE) + sizeof(slot) - 2);
        if (crc != Flash_CalculateCRC16((uint8_t*)&slot, sizeof(slot) - 2)) continue;
        
        if (slot.timestamp > latest_time) {
            latest_time = slot.timestamp;
            latest_slot = i;
        }
    }
    
    return latest_slot;
}

// เขียนข้อมูลแบบ wear leveling
void write_with_wear_leveling(uint16_t value) {
    int latest = find_latest_slot();
    int next = (latest + 1) % MAX_SLOTS;
    
    Slot_t slot = {
        .timestamp = millis(),
        .value = value
    };
    slot.crc = Flash_CalculateCRC16((uint8_t*)&slot, sizeof(slot) - 2);
    
    // ตรวจสอบว่า slot ถัดไปว่างหรือไม่
    Slot_t check;
    Flash_ReadStruct(FLASH_DATA_ADDR + (next * SLOT_SIZE), &check, sizeof(check));
    if (check.timestamp != 0xFFFFFFFF) {
        Flash_ErasePage(FLASH_DATA_PAGE);  // ไม่ว่าง - erase
    }
    
    Flash_WriteStruct(FLASH_DATA_ADDR + (next * SLOT_SIZE), &slot, sizeof(slot));
}
```

**ประโยชน์:**
- ลดการ erase page จาก N ครั้ง เหลือ ~N/M ครั้ง (M = จำนวน slots)
- ยืดอายุ Flash ได้หลายเท่า

### 2. Data Validation ด้วย CRC

#### การใช้ CRC16-CCITT

```c
// คำนวณ CRC
uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
uint16_t crc = Flash_CalculateCRC16(data, sizeof(data));

// เก็บข้อมูลพร้อม CRC
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteStruct(FLASH_DATA_ADDR, data, sizeof(data));
Flash_WriteHalfWord(FLASH_DATA_ADDR + sizeof(data), crc);

// ตรวจสอบข้อมูล
uint8_t loaded_data[4];
Flash_ReadStruct(FLASH_DATA_ADDR, loaded_data, sizeof(loaded_data));
uint16_t loaded_crc = Flash_ReadHalfWord(FLASH_DATA_ADDR + sizeof(data));
uint16_t calc_crc = Flash_CalculateCRC16(loaded_data, sizeof(loaded_data));

if (loaded_crc == calc_crc) {
    printf("✓ Data integrity OK\n");
} else {
    printf("✗ Data corrupted!\n");
}
```

### 3. Factory Reset

```c
void factory_reset(void) {
    // ลบทุกอย่าง
    Flash_EraseAll();
    
    // ตั้งค่า default config
    Config_t default_config = {
        .magic = CONFIG_MAGIC,
        .brightness = 50,
        .volume = 75
    };
    
    Flash_SaveConfig(&default_config, sizeof(default_config) - 2);
    
    printf("Factory reset completed\n");
}
```

### 4. Data Migration

เมื่อ config structure เปลี่ยน version:

```c
#define CONFIG_VERSION_1  1
#define CONFIG_VERSION_2  2

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t brightness;
    uint16_t volume;
    uint16_t crc;
} ConfigV1_t;

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t brightness;
    uint16_t volume;
    uint16_t contrast;  // ← เพิ่มใหม่
    uint16_t crc;
} ConfigV2_t;

void migrate_config(void) {
    ConfigV1_t old_config;
    
    if (Flash_LoadConfig(&old_config, sizeof(old_config) - 2)) {
        if (old_config.version == CONFIG_VERSION_1) {
            // Migrate to V2
            ConfigV2_t new_config = {
                .magic = old_config.magic,
                .version = CONFIG_VERSION_2,
                .brightness = old_config.brightness,
                .volume = old_config.volume,
                .contrast = 50  // ค่า default
            };
            
            Flash_SaveConfig(&new_config, sizeof(new_config) - 2);
            printf("Config migrated to V2\n");
        }
    }
}
```

---

## Best Practices

### 1. เมื่อไหร่ควรใช้ Flash vs External EEPROM

| ลักษณะการใช้งาน | แนะนำ |
|-----------------|-------|
| เขียนน้อย (<10 ครั้ง/วัน) | ✅ Flash |
| เขียนปานกลาง (10-100 ครั้ง/วัน) | ✅ Flash + Wear Leveling |
| เขียนบ่อย (>100 ครั้ง/วัน) | ⚠️ External EEPROM |
| ข้อมูลขนาดใหญ่ (>128 bytes) | ⚠️ External EEPROM |

### 2. การจัดการ Flash Lifetime

```c
// ❌ ไม่ดี - เขียนบ่อยเกินไป
while(1) {
    sensor_value = read_sensor();
    Flash_ErasePage(FLASH_DATA_PAGE);
    Flash_WriteWord(FLASH_DATA_ADDR, sensor_value);
    Delay_Ms(1000);  // เขียนทุกวินาที!
}

// ✅ ดี - เขียนเฉพาะเมื่อจำเป็น
uint32_t last_saved_value = 0;
while(1) {
    sensor_value = read_sensor();
    
    // เขียนเฉพาะเมื่อค่าเปลี่ยน
    if (abs(sensor_value - last_saved_value) > THRESHOLD) {
        Flash_ErasePage(FLASH_DATA_PAGE);
        Flash_WriteWord(FLASH_DATA_ADDR, sensor_value);
        last_saved_value = sensor_value;
    }
    
    Delay_Ms(1000);
}
```

### 3. Error Handling

```c
FlashStatus status = Flash_WriteWord(FLASH_DATA_ADDR, data);

switch (status) {
    case FLASH_OK:
        printf("Write successful\n");
        break;
    case FLASH_ERROR_RANGE:
        printf("Address out of range\n");
        break;
    case FLASH_ERROR_ALIGN:
        printf("Address not aligned\n");
        break;
    case FLASH_ERROR_VERIFY:
        printf("Verification failed\n");
        break;
    default:
        printf("Unknown error: %d\n", status);
        break;
}
```

### 4. Memory Layout Planning

```c
// กำหนด address offsets อย่างชัดเจน
#define CONFIG_ADDR         FLASH_CONFIG_ADDR
#define DEVICE_NAME_ADDR    (FLASH_DATA_ADDR + 0)
#define USER_NAME_ADDR      (FLASH_DATA_ADDR + 32)
#define SENSOR_CAL_ADDR     (FLASH_DATA_ADDR + 64)

// ใช้ constants แทนตัวเลข
Flash_WriteString(DEVICE_NAME_ADDR, "MyDevice");
Flash_WriteString(USER_NAME_ADDR, "Admin");
```

---

## API Reference

### Initialization

#### `Flash_Init()`
เริ่มต้นระบบ Flash storage

**Returns:** `FlashStatus` - FLASH_OK ถ้าสำเร็จ

```c
Flash_Init();
```

---

### Erase Operations

#### `Flash_ErasePage(uint8_t page_num)`
ลบข้อมูลใน page ที่กำหนด

**Parameters:**
- `page_num` - หมายเลข page (254 หรือ 255)

**Returns:** `FlashStatus`

```c
Flash_ErasePage(FLASH_CONFIG_PAGE);
Flash_ErasePage(FLASH_DATA_PAGE);
```

#### `Flash_EraseAll()`
ลบข้อมูลทั้งหมดใน storage area

**Returns:** `FlashStatus`

```c
Flash_EraseAll();  // Factory reset
```

---

### Read Operations

#### `Flash_ReadByte(uint32_t addr)`
อ่านข้อมูล 1 byte

**Parameters:**
- `addr` - ที่อยู่ใน Flash

**Returns:** `uint8_t` - ค่าที่อ่านได้

```c
uint8_t value = Flash_ReadByte(FLASH_DATA_ADDR);
```

#### `Flash_ReadHalfWord(uint32_t addr)`
อ่านข้อมูล 16-bit

**Parameters:**
- `addr` - ที่อยู่ใน Flash (ต้อง align 2 bytes)

**Returns:** `uint16_t`

```c
uint16_t value = Flash_ReadHalfWord(FLASH_DATA_ADDR);
```

#### `Flash_ReadWord(uint32_t addr)`
อ่านข้อมูล 32-bit

**Parameters:**
- `addr` - ที่อยู่ใน Flash (ต้อง align 4 bytes)

**Returns:** `uint32_t`

```c
uint32_t value = Flash_ReadWord(FLASH_DATA_ADDR);
```

---

### Write Operations

> [!WARNING]
> ต้อง **erase page ก่อน** เขียนข้อมูลใหม่!

#### `Flash_WriteByte(uint32_t addr, uint8_t data)`
เขียนข้อมูล 1 byte

**Returns:** `FlashStatus`

```c
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteByte(FLASH_DATA_ADDR, 0xAB);
```

#### `Flash_WriteHalfWord(uint32_t addr, uint16_t data)`
เขียนข้อมูล 16-bit

**Returns:** `FlashStatus`

```c
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteHalfWord(FLASH_DATA_ADDR, 0x1234);
```

#### `Flash_WriteWord(uint32_t addr, uint32_t data)`
เขียนข้อมูล 32-bit

**Returns:** `FlashStatus`

```c
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteWord(FLASH_DATA_ADDR, 0x12345678);
```

---

### String Operations

#### `Flash_ReadString(uint32_t addr, char* buffer, uint16_t max_len)`
อ่าน null-terminated string

**Parameters:**
- `addr` - ที่อยู่ใน Flash
- `buffer` - buffer สำหรับเก็บ string
- `max_len` - ขนาดสูงสุดของ buffer

**Returns:** `uint16_t` - ความยาว string (ไม่รวม null)

```c
char name[32];
uint16_t len = Flash_ReadString(FLASH_DATA_ADDR, name, sizeof(name));
```

#### `Flash_WriteString(uint32_t addr, const char* str)`
เขียน null-terminated string

**Parameters:**
- `addr` - ที่อยู่ใน Flash
- `str` - string ที่ต้องการเขียน

**Returns:** `FlashStatus`

```c
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteString(FLASH_DATA_ADDR, "Hello");
```

---

### Struct Operations

#### `Flash_ReadStruct(uint32_t addr, void* ptr, uint16_t size)`
อ่าน struct/buffer

**Parameters:**
- `addr` - ที่อยู่ใน Flash
- `ptr` - pointer ไปยัง struct ปลายทาง
- `size` - ขนาดของ struct (bytes)

**Returns:** `FlashStatus`

```c
MyStruct_t data;
Flash_ReadStruct(FLASH_DATA_ADDR, &data, sizeof(data));
```

#### `Flash_WriteStruct(uint32_t addr, const void* ptr, uint16_t size)`
เขียน struct/buffer

**Parameters:**
- `addr` - ที่อยู่ใน Flash
- `ptr` - pointer ไปยัง struct ต้นทาง
- `size` - ขนาดของ struct (bytes)

**Returns:** `FlashStatus`

```c
MyStruct_t data = {...};
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteStruct(FLASH_DATA_ADDR, &data, sizeof(data));
```

---

### Configuration Management

#### `Flash_SaveConfig(const void* ptr, uint16_t size)`
บันทึก configuration พร้อม CRC

**Parameters:**
- `ptr` - pointer ไปยัง config struct
- `size` - ขนาดของ struct **ไม่รวม CRC field**

**Returns:** `FlashStatus`

```c
Config_t config = {...};
Flash_SaveConfig(&config, sizeof(config) - sizeof(config.crc));
```

#### `Flash_LoadConfig(void* ptr, uint16_t size)`
โหลด configuration และตรวจสอบ CRC

**Parameters:**
- `ptr` - pointer ไปยัง config struct ปลายทาง
- `size` - ขนาดของ struct **ไม่รวม CRC field**

**Returns:** `bool` - true ถ้าโหลดสำเร็จและ CRC ถูกต้อง

```c
Config_t config;
if (Flash_LoadConfig(&config, sizeof(config) - sizeof(config.crc))) {
    // ใช้ config
}
```

#### `Flash_IsConfigValid()`
ตรวจสอบว่ามี valid configuration หรือไม่

**Returns:** `bool`

```c
if (Flash_IsConfigValid()) {
    // มี config
}
```

---

### Utility Functions

#### `Flash_CalculateCRC16(const uint8_t* data, uint16_t len)`
คำนวณ CRC16-CCITT checksum

**Returns:** `uint16_t` - CRC checksum

```c
uint8_t data[] = {0x01, 0x02, 0x03};
uint16_t crc = Flash_CalculateCRC16(data, sizeof(data));
```

#### `Flash_IsAddressValid(uint32_t addr)`
ตรวจสอบว่า address อยู่ใน storage area หรือไม่

**Returns:** `bool`

```c
if (Flash_IsAddressValid(addr)) {
    // ใช้งานได้
}
```

#### `Flash_GetPageAddress(uint8_t page_num)`
แปลง page number เป็น address

**Returns:** `uint32_t` - address ของ page

```c
uint32_t addr = Flash_GetPageAddress(FLASH_CONFIG_PAGE);
```

---

### Advanced Functions

#### `Flash_WriteByteWithErase(uint32_t addr, uint8_t data)`
เขียน byte พร้อม auto-erase (modify-erase-write)

> [!WARNING]
> ช้ากว่าปกติ - ใช้เฉพาะเมื่อจำเป็น

**Returns:** `FlashStatus`

```c
Flash_WriteByteWithErase(FLASH_DATA_ADDR + 5, 0xAB);
```

---

## Troubleshooting

### ปัญหาที่พบบ่อย

#### 1. เขียนข้อมูลไม่ได้

**อาการ:** `Flash_WriteWord()` return `FLASH_ERROR_VERIFY`

**สาเหตุ:**
- ลืม erase page ก่อนเขียน
- Address ไม่ถูกต้อง

**วิธีแก้:**
```c
// ✅ ถูกต้อง
Flash_ErasePage(FLASH_DATA_PAGE);
Flash_WriteWord(FLASH_DATA_ADDR, data);

// ❌ ผิด - ลืม erase
Flash_WriteWord(FLASH_DATA_ADDR, data);
```

#### 2. Address Alignment Error

**อาการ:** `FLASH_ERROR_ALIGN`

**สาเหตุ:** Address ไม่ align ถูกต้อง

**วิธีแก้:**
```c
// ❌ ผิด - half-word ต้อง align 2
Flash_WriteHalfWord(0x0803F81, 0x1234);  // เลขคี่

// ✅ ถูกต้อง
Flash_WriteHalfWord(0x0803F80, 0x1234);  // เลขคู่
```

#### 3. CRC Validation Failed

**อาการ:** `Flash_LoadConfig()` return false

**สาเหตุ:**
- ข้อมูลเสียหาย
- Flash ว่าง (ยังไม่เคยเขียน)
- Struct definition เปลี่ยน

**วิธีแก้:**
```c
if (!Flash_LoadConfig(&config, sizeof(config) - 2)) {
    // ใช้ค่า default
    config = default_config;
    Flash_SaveConfig(&config, sizeof(config) - 2);
}
```

#### 4. String ยาวเกินไป

**อาการ:** `Flash_WriteString()` return `FLASH_ERROR_RANGE`

**สาเหตุ:** String เกิน `FLASH_MAX_STRING_LENGTH` (60 chars)

**วิธีแก้:**
```c
// ตรวจสอบความยาวก่อนเขียน
if (strlen(str) <= FLASH_MAX_STRING_LENGTH) {
    Flash_WriteString(addr, str);
} else {
    printf("String too long!\n");
}
```

### Debug Tips

#### 1. ตรวจสอบข้อมูลใน Flash

```c
void dump_flash_page(uint8_t page_num) {
    uint32_t addr = Flash_GetPageAddress(page_num);
    
    printf("Page %d dump:\n", page_num);
    for (int i = 0; i < FLASH_PAGE_SIZE; i += 16) {
        printf("%08X: ", addr + i);
        for (int j = 0; j < 16; j++) {
            printf("%02X ", Flash_ReadByte(addr + i + j));
        }
        printf("\n");
    }
}
```

#### 2. ตรวจสอบ Flash Status

```c
FlashStatus status = Flash_WriteWord(addr, data);
printf("Flash status: %d\n", status);

switch (status) {
    case FLASH_OK: printf("OK\n"); break;
    case FLASH_ERROR_RANGE: printf("Address out of range\n"); break;
    case FLASH_ERROR_ALIGN: printf("Alignment error\n"); break;
    case FLASH_ERROR_VERIFY: printf("Verify failed\n"); break;
}
```

---

## ตัวอย่างโปรเจค

ดูตัวอย่างการใช้งานเพิ่มเติมใน `/User/SimpleHAL/Examples/Flash/`:

1. [flash_basic_read_write.c](flash_basic_read_write.c) - การอ่าน/เขียนพื้นฐาน
2. [flash_config_storage.c](flash_config_storage.c) - จัดเก็บ configuration
3. [flash_string_storage.c](flash_string_storage.c) - จัดเก็บ string
4. [flash_struct_storage.c](flash_struct_storage.c) - จัดเก็บ struct
5. [flash_wear_leveling.c](flash_wear_leveling.c) - เทคนิค wear leveling

---

## สรุป

SimpleFlash Library ช่วยให้คุณจัดเก็บข้อมูลใน Flash memory ได้ง่ายและปลอดภัย:

✅ **ใช้งานง่าย** - API แบบ Arduino-style  
✅ **ปลอดภัย** - CRC validation ตรวจสอบความถูกต้อง  
✅ **ยืดอายุ** - Wear leveling ลดการ erase  
✅ **ครบถ้วน** - รองรับทุกประเภทข้อมูล

**Happy Coding! 🚀**
