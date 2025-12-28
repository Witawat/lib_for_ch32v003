# SimpleDMA Library - คู่มือการใช้งาน

> **Simple DMA Library สำหรับ CH32V003**  
> ถ่ายโอนข้อมูลความเร็วสูงโดยไม่ใช้ CPU พร้อม integration กับ SimpleHAL peripherals

---

## 📋 สารบัญ

1. [ภาพรวม](#ภาพรวม)
2. [ทฤษฎี DMA](#ทฤษฎี-dma)
3. [การเริ่มต้นใช้งาน](#การเริ่มต้นใช้งาน)
4. [การใช้งานขั้นพื้นฐาน](#การใช้งานขั้นพื้นฐาน)
5. [การใช้งานขั้นกลาง](#การใช้งานขั้นกลาง)
6. [เทคนิคขั้นสูง](#เทคนิคขั้นสูง)
7. [Best Practices](#best-practices)
8. [API Reference](#api-reference)
9. [Troubleshooting](#troubleshooting)

---

## ภาพรวม

### คุณสมบัติหลัก

- ✅ รองรับ 7 DMA channels
- ✅ 3 โหมดการถ่ายโอน (Memory-to-Memory, Peripheral-to-Memory, Memory-to-Peripheral)
- ✅ Circular buffer mode สำหรับข้อมูลต่อเนื่อง
- ✅ Priority management (4 ระดับ)
- ✅ Callback functions สำหรับ Transfer Complete และ Error
- ✅ Integration กับ SimpleADC, SimpleUSART, SimpleSPI
- ✅ API ใช้งานง่ายแบบ Arduino-style

### ความสามารถของ CH32V003 DMA

CH32V003 มี DMA controller 1 ตัว พร้อม **7 channels**:

| Channel | Priority (default) | การใช้งานทั่วไป |
|---------|-------------------|-----------------|
| DMA_CH1 | สูงสุด | ADC, Memory-to-Memory |
| DMA_CH2 | สูง | USART TX |
| DMA_CH3 | กลาง | USART RX |
| DMA_CH4 | กลาง | SPI TX |
| DMA_CH5 | กลาง | SPI RX |
| DMA_CH6 | ต่ำ | I2C, Timer |
| DMA_CH7 | ต่ำสุด | General purpose |

> [!NOTE]
> **Channel Priority**
> - Channel number ต่ำกว่ามี hardware priority สูงกว่า (ถ้าตั้ง software priority เท่ากัน)
> - สามารถตั้ง software priority ได้ 4 ระดับ: Low, Medium, High, Very High

---

## ทฤษฎี DMA

### DMA คืออะไร?

**DMA (Direct Memory Access)** คือ hardware controller ที่สามารถถ่ายโอนข้อมูลระหว่าง:
- Memory ↔ Memory
- Peripheral ↔ Memory
- Memory ↔ Peripheral

โดย**ไม่ต้องใช้ CPU** ทำให้ CPU สามารถทำงานอื่นได้พร้อมกัน

### ทำไมต้องใช้ DMA?

**ปัญหาของการไม่ใช้ DMA:**
```c
// ❌ CPU ต้องรอ ADC conversion
for (int i = 0; i < 1000; i++) {
    adc_buffer[i] = ADC_Read(ADC_CH_0);  // CPU blocked!
}

// ❌ CPU ต้องรอส่งข้อมูลทาง USART
for (int i = 0; i < 1000; i++) {
    USART_WriteByte(data[i]);  // CPU blocked!
}
```

**ข้อดีของการใช้ DMA:**
```c
// ✅ DMA ทำงานอัตโนมัติ CPU ทำงานอื่นได้
DMA_ADC_Init(DMA_CH1, adc_buffer, 1000, 1);
DMA_Start(DMA_CH1);

// CPU ทำงานอื่นได้ทันที!
process_previous_data();
calculate_statistics();
update_display();
```

### Transfer Modes

#### 1. Normal Mode
- ถ่ายโอนข้อมูลครั้งเดียวแล้วหยุด
- เหมาะสำหรับ: Memory copy, Single transfer

```c
DMA_Config_t config = {
    .mode = DMA_MODE_NORMAL,
    .buffer_size = 100
};
// ถ่ายโอน 100 bytes แล้วหยุด
```

#### 2. Circular Mode
- ถ่ายโอนข้อมูลวนซ้ำ (ring buffer)
- เมื่อถึงจุดสิ้นสุด buffer จะกลับไปเริ่มต้นอัตโนมัติ
- เหมาะสำหรับ: ADC continuous, USART RX, Audio streaming

```c
DMA_Config_t config = {
    .mode = DMA_MODE_CIRCULAR,
    .buffer_size = 100
};
// ถ่ายโอนวนซ้ำ: 0→99→0→99→...
```

### Priority Levels

DMA มี 2 ระดับ priority:

1. **Hardware Priority** (ตาม channel number)
   - Channel 1 > Channel 2 > ... > Channel 7
   - ใช้เมื่อ software priority เท่ากัน

2. **Software Priority** (ตั้งค่าได้)
   - Very High > High > Medium > Low
   - Override hardware priority

```c
// ตัวอย่าง: Channel 7 แต่ priority Very High
DMA_Config_t config = {
    .channel = DMA_CH7,
    .priority = DMA_PRIORITY_VERY_HIGH  // จะได้ priority สูงกว่า CH1-6 ที่เป็น Low
};
```

---

## การเริ่มต้นใช้งาน

### 1. การติดตั้ง

Include header file ในโปรแกรม:

```c
#include "SimpleHAL/SimpleHAL.h"  // รวม SimpleDMA อยู่แล้ว
// หรือ
#include "SimpleHAL/SimpleDMA.h"
```

### 2. ตัวอย่างโค้ดพื้นฐาน

```c
#include "SimpleHAL/SimpleHAL.h"

int main(void) {
    // เริ่มต้นระบบ
    SystemCoreClockUpdate();
    Delay_Init();
    
    // ตัวอย่าง: Copy memory ด้วย DMA
    uint8_t src[100], dst[100];
    
    // เติมข้อมูล
    for (int i = 0; i < 100; i++) {
        src[i] = i;
    }
    
    // Copy ด้วย DMA (blocking)
    DMA_MemCopy(dst, src, 100);
    
    // ตรวจสอบผลลัพธ์
    for (int i = 0; i < 100; i++) {
        if (dst[i] != src[i]) {
            printf("Error!\n");
        }
    }
    
    printf("Copy complete!\n");
}
```

---

## การใช้งานขั้นพื้นฐาน

### 1. Memory-to-Memory Transfer

#### Blocking Mode (รอจนเสร็จ)

```c
uint8_t source[1000];
uint8_t destination[1000];

// Copy ด้วย DMA (blocking)
DMA_MemCopy(destination, source, 1000);

// โค้ดบรรทัดนี้จะรันหลังจาก copy เสร็จ
printf("Copy done!\n");
```

#### Non-blocking Mode (ไม่รอ)

```c
// Copy ด้วย DMA (non-blocking)
DMA_MemCopyAsync(DMA_CH1, destination, source, 1000);

// CPU ทำงานอื่นได้ทันที
do_other_work();

// ตรวจสอบสถานะ
while (DMA_GetStatus(DMA_CH1) != DMA_STATUS_COMPLETE) {
    // รอหรือทำงานอื่น
}
```

### 2. การใช้ Callbacks

```c
volatile uint8_t done = 0;

void on_complete(DMA_Channel channel) {
    done = 1;
    printf("Transfer complete!\n");
}

void on_error(DMA_Channel channel) {
    printf("Transfer error!\n");
}

int main(void) {
    // ตั้งค่า callbacks
    DMA_SetTransferCompleteCallback(DMA_CH1, on_complete);
    DMA_SetErrorCallback(DMA_CH1, on_error);
    
    // เริ่ม transfer
    DMA_MemCopyAsync(DMA_CH1, dst, src, 1000);
    
    // รอผ่าน callback
    while (!done) {
        // ทำงานอื่น
    }
}
```

### 3. Memory Set

```c
uint8_t buffer[1000];

// Clear buffer (set เป็น 0)
DMA_MemSet(buffer, 0, 1000);

// Fill buffer ด้วยค่า 0xFF
DMA_MemSet(buffer, 0xFF, 1000);
```

---

## การใช้งานขั้นกลาง

### 1. DMA กับ ADC

#### Single Channel Continuous

```c
#define SAMPLES 100
uint16_t adc_buffer[SAMPLES];

// เริ่มต้น ADC
ADC_SimpleInit();

// ตั้งค่า DMA สำหรับ ADC (circular mode)
DMA_ADC_Init(DMA_CH1, adc_buffer, SAMPLES, 1);
DMA_Start(DMA_CH1);

// เริ่ม ADC continuous conversion
ADC_SoftwareStartConvCmd(ADC1, ENABLE);

// อ่านค่าจาก buffer
while (1) {
    uint16_t latest = adc_buffer[SAMPLES - 1];
    float voltage = ADC_ToVoltage(latest, 3.3);
    printf("Voltage: %.3fV\n", voltage);
    Delay_Ms(100);
}
```

#### Multi-Channel

```c
#define NUM_CHANNELS 3
#define SAMPLES_PER_CH 10
uint16_t adc_buffer[NUM_CHANNELS * SAMPLES_PER_CH];

// ตั้งค่า ADC multi-channel
ADC_Channel channels[] = {ADC_CH_PA2, ADC_CH_PA1, ADC_CH_PC4};
ADC_SimpleInitChannels(channels, NUM_CHANNELS);

// ตั้งค่า DMA
DMA_ADC_InitMultiChannel(DMA_CH1, adc_buffer, NUM_CHANNELS, SAMPLES_PER_CH);
DMA_Start(DMA_CH1);

// Buffer layout: [CH0, CH1, CH2, CH0, CH1, CH2, ...]
// คำนวณค่าเฉลี่ย channel 0
uint32_t sum = 0;
for (int i = 0; i < SAMPLES_PER_CH; i++) {
    sum += adc_buffer[i * NUM_CHANNELS + 0];  // Channel 0
}
uint16_t avg = sum / SAMPLES_PER_CH;
```

### 2. DMA กับ USART

#### Transmission (TX)

```c
uint8_t tx_buffer[256];

// เตรียมข้อมูล
sprintf((char*)tx_buffer, "Hello from DMA!\n");

// ตั้งค่า DMA สำหรับ USART TX
USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
DMA_USART_InitTx(DMA_CH2, tx_buffer, 256);

// ส่งข้อมูล
DMA_USART_Transmit(DMA_CH2, tx_buffer, strlen((char*)tx_buffer));

// CPU ทำงานอื่นได้
while (DMA_GetStatus(DMA_CH2) == DMA_STATUS_BUSY) {
    do_other_work();
}
```

#### Reception (RX) - Circular Buffer

```c
#define RX_BUF_SIZE 128
uint8_t rx_buffer[RX_BUF_SIZE];
uint16_t last_pos = 0;

// ตั้งค่า DMA circular buffer
USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
DMA_USART_InitRx(DMA_CH3, rx_buffer, RX_BUF_SIZE, 1);  // Circular mode
DMA_Start(DMA_CH3);

while (1) {
    // ตรวจสอบข้อมูลใหม่
    uint16_t current_pos = DMA_USART_GetReceivedCount(DMA_CH3, RX_BUF_SIZE);
    
    if (current_pos != last_pos) {
        // มีข้อมูลใหม่
        if (current_pos > last_pos) {
            // ประมวลผลข้อมูล [last_pos, current_pos)
            process_data(&rx_buffer[last_pos], current_pos - last_pos);
        } else {
            // Wrap around: ประมวลผล [last_pos, RX_BUF_SIZE) และ [0, current_pos)
            process_data(&rx_buffer[last_pos], RX_BUF_SIZE - last_pos);
            process_data(&rx_buffer[0], current_pos);
        }
        
        last_pos = current_pos;
    }
    
    Delay_Ms(10);
}
```

### 3. DMA กับ SPI

```c
#define TRANSFER_SIZE 64
uint8_t tx_data[TRANSFER_SIZE];
uint8_t rx_data[TRANSFER_SIZE];

// เตรียมข้อมูล
for (int i = 0; i < TRANSFER_SIZE; i++) {
    tx_data[i] = i;
}

// ตั้งค่า SPI และ DMA
SPI_SimpleInit(SPI_MODE0, SPI_1MHZ, SPI_PINS_DEFAULT);
DMA_SPI_Init(DMA_CH4, DMA_CH5);  // TX=CH4, RX=CH5

// ส่งและรับข้อมูล
SPI_SetCS(0);  // CS = LOW
DMA_SPI_TransferBuffer(DMA_CH4, DMA_CH5, tx_data, rx_data, TRANSFER_SIZE);
SPI_SetCS(1);  // CS = HIGH

// ตรวจสอบข้อมูลที่รับ
for (int i = 0; i < TRANSFER_SIZE; i++) {
    printf("%02X ", rx_data[i]);
}
```

---

## เทคนิคขั้นสูง

### 1. Double Buffering

เทคนิคนี้ใช้ 2 buffers สลับกัน เพื่อให้ CPU และ DMA ทำงานพร้อมกัน:

```c
#define BUF_SIZE 256

uint8_t buffer_a[BUF_SIZE];
uint8_t buffer_b[BUF_SIZE];
uint8_t* processing_buf = buffer_a;
uint8_t* filling_buf = buffer_b;

volatile uint8_t dma_done = 0;

void on_dma_complete(DMA_Channel ch) {
    dma_done = 1;
}

int main(void) {
    DMA_SetTransferCompleteCallback(DMA_CH1, on_dma_complete);
    
    while (1) {
        // 1. CPU เตรียมข้อมูลใน filling_buf
        prepare_data(filling_buf);
        
        // 2. เริ่ม DMA transfer จาก filling_buf
        dma_done = 0;
        DMA_MemCopyAsync(DMA_CH1, output, filling_buf, BUF_SIZE);
        
        // 3. สลับ buffers
        uint8_t* temp = processing_buf;
        processing_buf = filling_buf;
        filling_buf = temp;
        
        // 4. CPU ประมวลผล processing_buf ขณะที่ DMA ทำงาน
        process_data(processing_buf);
        
        // 5. รอให้ DMA เสร็จ (ถ้ายังไม่เสร็จ)
        while (!dma_done);
    }
}
```

**ประโยชน์:**
- CPU และ DMA ทำงานพร้อมกัน 100%
- เพิ่ม throughput สูงสุด
- เหมาะสำหรับ: Audio/Video streaming, High-speed data acquisition

### 2. Priority Management

```c
// Scenario: ADC ต้องการ priority สูงสุด, USART ปานกลาง

// ADC - Very High Priority
DMA_Config_t adc_config = {
    .channel = DMA_CH1,
    .priority = DMA_PRIORITY_VERY_HIGH,
    // ...
};
DMA_SimpleInit(&adc_config);

// USART TX - Medium Priority
DMA_Config_t usart_config = {
    .channel = DMA_CH2,
    .priority = DMA_PRIORITY_MEDIUM,
    // ...
};
DMA_SimpleInit(&usart_config);

// ถ้า ADC และ USART request พร้อมกัน → ADC จะได้ก่อน
```

### 3. Error Handling

```c
volatile uint8_t error_occurred = 0;
volatile DMA_Channel error_channel = 0;

void on_error(DMA_Channel ch) {
    error_occurred = 1;
    error_channel = ch;
}

int main(void) {
    // ตั้งค่า error callback ทุก channels
    for (DMA_Channel ch = DMA_CH1; ch <= DMA_CH7; ch++) {
        DMA_SetErrorCallback(ch, on_error);
    }
    
    // เริ่ม transfer
    DMA_MemCopyAsync(DMA_CH1, dst, src, 1000);
    
    // ตรวจสอบ error
    if (error_occurred) {
        printf("Error on channel %d\n", error_channel);
        
        // รีเซ็ต channel
        DMA_Reset(error_channel);
        
        // ลองใหม่
        DMA_Start(error_channel);
    }
}
```

### 4. Multi-Channel Coordination

```c
// ใช้หลาย channels พร้อมกัน
volatile uint8_t ch1_done = 0, ch2_done = 0, ch3_done = 0;

void on_ch1_complete(DMA_Channel ch) { ch1_done = 1; }
void on_ch2_complete(DMA_Channel ch) { ch2_done = 1; }
void on_ch3_complete(DMA_Channel ch) { ch3_done = 1; }

int main(void) {
    // ตั้งค่า callbacks
    DMA_SetTransferCompleteCallback(DMA_CH1, on_ch1_complete);
    DMA_SetTransferCompleteCallback(DMA_CH2, on_ch2_complete);
    DMA_SetTransferCompleteCallback(DMA_CH3, on_ch3_complete);
    
    // เริ่ม transfers พร้อมกัน
    DMA_MemCopyAsync(DMA_CH1, dst1, src1, 1000);
    DMA_MemCopyAsync(DMA_CH2, dst2, src2, 500);
    DMA_MemCopyAsync(DMA_CH3, dst3, src3, 200);
    
    // รอให้ทุก channels เสร็จ
    while (!ch1_done || !ch2_done || !ch3_done) {
        // ทำงานอื่นได้
    }
    
    printf("All transfers complete!\n");
}
```

---

## Best Practices

### 1. การเลือก Channel

| Peripheral | แนะนำ Channel | เหตุผล |
|-----------|--------------|--------|
| ADC | CH1 | Priority สูง, ข้อมูล real-time |
| USART TX | CH2 | Priority กลาง-สูง |
| USART RX | CH3 | Priority กลาง, circular buffer |
| SPI TX | CH4 | Priority กลาง |
| SPI RX | CH5 | Priority กลาง |
| Memory Copy | CH6, CH7 | Priority ต่ำ, ไม่เร่งด่วน |

### 2. Memory Alignment

```c
// ❌ ไม่ดี - ไม่ align
uint8_t buffer[100];  // อาจไม่ align

// ✅ ดี - force alignment
uint8_t buffer[100] __attribute__((aligned(4)));

// ✅ ดีกว่า - ใช้ uint32_t
uint32_t buffer[25];  // 100 bytes, auto-aligned
```

### 3. Buffer Management

```c
// ❌ ไม่ดี - buffer ใน stack (อาจหมดอายุ)
void bad_example(void) {
    uint8_t buffer[100];  // Stack variable
    DMA_MemCopyAsync(DMA_CH1, dst, buffer, 100);  // ❌ อันตราย!
}  // buffer หมดอายุ แต่ DMA ยังทำงาน!

// ✅ ดี - buffer global หรือ static
uint8_t buffer[100];  // Global

void good_example(void) {
    DMA_MemCopyAsync(DMA_CH1, dst, buffer, 100);  // ✅ ปลอดภัย
}

// ✅ ดี - buffer static ใน function
void also_good(void) {
    static uint8_t buffer[100];  // Static
    DMA_MemCopyAsync(DMA_CH1, dst, buffer, 100);  // ✅ ปลอดภัย
}
```

### 4. Interrupt Priority

```c
// ตั้งค่า interrupt priority ให้เหมาะสม
void setup_interrupts(void) {
    // ADC DMA - Priority สูงสุด
    NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
    
    // USART DMA - Priority กลาง
    NVIC_SetPriority(DMA1_Channel2_IRQn, 1);
    NVIC_SetPriority(DMA1_Channel3_IRQn, 1);
    
    // Memory Copy - Priority ต่ำ
    NVIC_SetPriority(DMA1_Channel6_IRQn, 2);
}
```

### 5. Performance Optimization

```c
// ❌ ช้า - ใช้ byte transfer สำหรับข้อมูลขนาดใหญ่
DMA_Config_t config = {
    .data_size = DMA_SIZE_BYTE,  // 8-bit
    .buffer_size = 1000
};

// ✅ เร็วกว่า - ใช้ word transfer (ถ้า align ถูกต้อง)
DMA_Config_t config = {
    .data_size = DMA_SIZE_WORD,  // 32-bit
    .buffer_size = 250  // 1000 bytes / 4
};
// เร็วกว่าถึง 4 เท่า!
```

---

## API Reference

### Basic Functions

#### `DMA_SimpleInit(DMA_Config_t* config)`
เริ่มต้น DMA channel ตาม configuration

**Parameters:**
- `config` - pointer ไปยัง configuration structure

**Example:**
```c
DMA_Config_t config = {
    .channel = DMA_CH1,
    .direction = DMA_DIR_MEM_TO_MEM,
    .priority = DMA_PRIORITY_HIGH,
    .data_size = DMA_SIZE_BYTE,
    .mode = DMA_MODE_NORMAL,
    .mem_increment = 1,
    .periph_increment = 1,
    .periph_addr = (uint32_t)src,
    .mem_addr = (uint32_t)dst,
    .buffer_size = 100
};
DMA_SimpleInit(&config);
```

#### `DMA_Start(DMA_Channel channel)`
เริ่มการถ่ายโอนข้อมูล

#### `DMA_Stop(DMA_Channel channel)`
หยุดการถ่ายโอนข้อมูล

#### `DMA_GetStatus(DMA_Channel channel)`
ตรวจสอบสถานะของ channel

**Returns:** `DMA_Status` - IDLE, BUSY, COMPLETE, ERROR

### Memory Functions

#### `DMA_MemCopy(void* dst, const void* src, uint16_t size)`
Copy memory แบบ blocking

#### `DMA_MemCopyAsync(DMA_Channel channel, void* dst, const void* src, uint16_t size)`
Copy memory แบบ non-blocking

#### `DMA_MemSet(void* dst, uint8_t value, uint16_t size)`
Set memory ด้วยค่าที่กำหนด

### ADC Functions

#### `DMA_ADC_Init(DMA_Channel channel, uint16_t* buffer, uint16_t buffer_size, uint8_t circular)`
ตั้งค่า DMA สำหรับ ADC

#### `DMA_ADC_InitMultiChannel(DMA_Channel channel, uint16_t* buffer, uint8_t num_channels, uint16_t samples_per_channel)`
ตั้งค่า DMA สำหรับ ADC multi-channel

### USART Functions

#### `DMA_USART_InitTx(DMA_Channel channel, uint8_t* buffer, uint16_t buffer_size)`
ตั้งค่า DMA สำหรับ USART TX

#### `DMA_USART_InitRx(DMA_Channel channel, uint8_t* buffer, uint16_t buffer_size, uint8_t circular)`
ตั้งค่า DMA สำหรับ USART RX

#### `DMA_USART_Transmit(DMA_Channel channel, const uint8_t* data, uint16_t length)`
ส่งข้อมูลผ่าน USART ด้วย DMA

#### `DMA_USART_GetReceivedCount(DMA_Channel channel, uint16_t buffer_size)`
ตรวจสอบจำนวนข้อมูลที่รับได้

### SPI Functions

#### `DMA_SPI_Init(DMA_Channel tx_channel, DMA_Channel rx_channel)`
ตั้งค่า DMA สำหรับ SPI

#### `DMA_SPI_TransferBuffer(DMA_Channel tx_channel, DMA_Channel rx_channel, const uint8_t* tx_data, uint8_t* rx_data, uint16_t length)`
ส่งและรับข้อมูลผ่าน SPI ด้วย DMA

---

## Troubleshooting

### ปัญหาที่พบบ่อย

#### 1. DMA ไม่ทำงาน

**อาการ:** DMA_GetStatus() ยังคงเป็น IDLE

**สาเหตุและแก้ไข:**
```c
// ❌ ลืม Start
DMA_SimpleInit(&config);
// DMA ไม่ทำงาน!

// ✅ ต้อง Start
DMA_SimpleInit(&config);
DMA_Start(DMA_CH1);  // เริ่มทำงาน
```

#### 2. ข้อมูลผิดพลาด

**อาการ:** ข้อมูลที่ถ่ายโอนไม่ถูกต้อง

**สาเหตุ:** Buffer หมดอายุหรือถูกเขียนทับ

```c
// ❌ Buffer ใน stack
void bad(void) {
    uint8_t buf[100];
    DMA_MemCopyAsync(DMA_CH1, dst, buf, 100);
}  // buf หมดอายุ!

// ✅ ใช้ global/static
static uint8_t buf[100];
void good(void) {
    DMA_MemCopyAsync(DMA_CH1, dst, buf, 100);
}
```

#### 3. Callback ไม่ถูกเรียก

**สาเหตุ:** ลืมเปิด interrupt

```c
// ❌ ตั้ง callback แต่ไม่เปิด interrupt
DMA_SetTransferCompleteCallback(DMA_CH1, on_complete);
// Callback ไม่ทำงาน!

// ✅ Callback จะเปิด interrupt อัตโนมัติ
DMA_SetTransferCompleteCallback(DMA_CH1, on_complete);
// ตอนนี้ callback จะทำงาน
```

#### 4. ADC DMA ไม่อ่านค่า

**สาเหตุ:** ลืมเปิด ADC DMA mode

```c
// ❌ ลืมเปิด ADC DMA
DMA_ADC_Init(DMA_CH1, buffer, 100, 1);
DMA_Start(DMA_CH1);
// ADC ไม่ส่งข้อมูลไป DMA!

// ✅ DMA_ADC_Init() เปิดให้อัตโนมัติแล้ว
DMA_ADC_Init(DMA_CH1, buffer, 100, 1);
DMA_Start(DMA_CH1);
ADC_SoftwareStartConvCmd(ADC1, ENABLE);  // เริ่ม conversion
```

#### 5. Performance ไม่ดีตามที่คาดหวัง

**สาเหตุ:** ใช้ data size ไม่เหมาะสม

```c
// ❌ ช้า - ใช้ byte transfer
config.data_size = DMA_SIZE_BYTE;
config.buffer_size = 1000;

// ✅ เร็วกว่า - ใช้ word transfer (ถ้า align ถูก)
config.data_size = DMA_SIZE_WORD;
config.buffer_size = 250;  // 1000/4
```

---

## ตัวอย่างการใช้งานจริง

### 1. Data Logger

```c
// บันทึกค่า ADC ทุกๆ 1ms เป็นเวลา 10 วินาที
#define SAMPLE_RATE 1000  // Hz
#define DURATION 10       // seconds
#define TOTAL_SAMPLES (SAMPLE_RATE * DURATION)

uint16_t adc_log[TOTAL_SAMPLES];

void start_logging(void) {
    DMA_ADC_Init(DMA_CH1, adc_log, TOTAL_SAMPLES, 0);  // Normal mode
    DMA_Start(DMA_CH1);
    
    // ตั้งค่า ADC timer trigger (1kHz)
    // ... (ตั้งค่า timer)
    
    // รอจนเต็ม
    while (DMA_GetStatus(DMA_CH1) != DMA_STATUS_COMPLETE);
    
    // บันทึกลง Flash/SD card
    save_to_storage(adc_log, TOTAL_SAMPLES);
}
```

### 2. Serial Protocol Parser

```c
#define RX_BUF_SIZE 256
uint8_t rx_buffer[RX_BUF_SIZE];
uint16_t last_pos = 0;

void parse_serial_data(void) {
    uint16_t current_pos = DMA_USART_GetReceivedCount(DMA_CH3, RX_BUF_SIZE);
    
    if (current_pos != last_pos) {
        // มีข้อมูลใหม่
        uint16_t start = last_pos;
        uint16_t end = current_pos;
        
        // หา packet delimiter (เช่น '\n')
        for (uint16_t i = start; i != end; i = (i + 1) % RX_BUF_SIZE) {
            if (rx_buffer[i] == '\n') {
                // พบ packet สมบูรณ์
                process_packet(rx_buffer, start, i);
                start = (i + 1) % RX_BUF_SIZE;
            }
        }
        
        last_pos = current_pos;
    }
}
```

---

**เวอร์ชัน:** 1.0  
**วันที่:** 2025-12-22  
**ผู้พัฒนา:** CH32V003 SimpleHAL Team
