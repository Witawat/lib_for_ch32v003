/**
 * @file SimpleDMA.c
 * @brief Simple DMA Library Implementation
 * @version 1.0
 * @date 2025-12-22
 */

#include "SimpleDMA.h"
#include "SimpleADC.h"
#include <string.h>

/* ========== Private Variables ========== */

// Callback functions สำหรับแต่ละ channel
static DMA_TransferCompleteCallback transfer_complete_callbacks[7] = {NULL};
static DMA_ErrorCallback error_callbacks[7] = {NULL};

// Status tracking
static volatile DMA_Status channel_status[7] = {DMA_STATUS_IDLE};

/* ========== Private Function Prototypes ========== */

static DMA_Channel_TypeDef* get_channel_base(DMA_Channel channel);
static IRQn_Type get_channel_irqn(DMA_Channel channel);
static void enable_dma_clock(void);

/* ========== Public Functions ========== */

/**
 * @brief เริ่มต้นการใช้งาน DMA channel
 */
void DMA_SimpleInit(DMA_Config_t* config) {
    DMA_InitTypeDef DMA_InitStructure = {0};
    
    // 1. เปิด Clock
    enable_dma_clock();
    
    // 2. รีเซ็ต channel
    DMA_Reset(config->channel);
    
    // 3. ตั้งค่า DMA
    DMA_Channel_TypeDef* dma_ch = get_channel_base(config->channel);
    
    // Peripheral address
    DMA_InitStructure.DMA_PeripheralBaseAddr = config->periph_addr;
    
    // Memory address
    DMA_InitStructure.DMA_MemoryBaseAddr = config->mem_addr;
    
    // Direction
    if (config->direction == DMA_DIR_MEM_TO_MEM) {
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
    } else if (config->direction == DMA_DIR_MEM_TO_PERIPH) {
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    } else {
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    }
    
    // Buffer size
    DMA_InitStructure.DMA_BufferSize = config->buffer_size;
    
    // Peripheral increment
    DMA_InitStructure.DMA_PeripheralInc = config->periph_increment ? 
                                          DMA_PeripheralInc_Enable : DMA_PeripheralInc_Disable;
    
    // Memory increment
    DMA_InitStructure.DMA_MemoryInc = config->mem_increment ? 
                                      DMA_MemoryInc_Enable : DMA_MemoryInc_Disable;
    
    // Data size
    uint32_t data_width;
    switch (config->data_size) {
        case DMA_SIZE_BYTE:
            data_width = DMA_PeripheralDataSize_Byte;
            break;
        case DMA_SIZE_HALFWORD:
            data_width = DMA_PeripheralDataSize_HalfWord;
            break;
        case DMA_SIZE_WORD:
            data_width = DMA_PeripheralDataSize_Word;
            break;
        default:
            data_width = DMA_PeripheralDataSize_Byte;
            break;
    }
    DMA_InitStructure.DMA_PeripheralDataSize = data_width;
    DMA_InitStructure.DMA_MemoryDataSize = data_width;
    
    // Mode (Normal/Circular)
    DMA_InitStructure.DMA_Mode = (config->mode == DMA_MODE_CIRCULAR) ? 
                                 DMA_Mode_Circular : DMA_Mode_Normal;
    
    // Priority
    uint32_t priority;
    switch (config->priority) {
        case DMA_PRIORITY_LOW:
            priority = DMA_Priority_Low;
            break;
        case DMA_PRIORITY_MEDIUM:
            priority = DMA_Priority_Medium;
            break;
        case DMA_PRIORITY_HIGH:
            priority = DMA_Priority_High;
            break;
        case DMA_PRIORITY_VERY_HIGH:
            priority = DMA_Priority_VeryHigh;
            break;
        default:
            priority = DMA_Priority_Medium;
            break;
    }
    DMA_InitStructure.DMA_Priority = priority;
    
    // Apply configuration
    DMA_Init(dma_ch, &DMA_InitStructure);
    
    // 4. Set status
    channel_status[config->channel - 1] = DMA_STATUS_IDLE;
}

/**
 * @brief เริ่มการถ่ายโอนข้อมูล
 */
void DMA_Start(DMA_Channel channel) {
    DMA_Channel_TypeDef* dma_ch = get_channel_base(channel);
    
    // Clear flags
    uint32_t flag_base = ((channel - 1) * 4);
    DMA_ClearFlag(DMA1_FLAG_TC1 << flag_base);
    DMA_ClearFlag(DMA1_FLAG_TE1 << flag_base);
    
    // Set status
    channel_status[channel - 1] = DMA_STATUS_BUSY;
    
    // Enable channel
    DMA_Cmd(dma_ch, ENABLE);
}

/**
 * @brief หยุดการถ่ายโอนข้อมูล
 */
void DMA_Stop(DMA_Channel channel) {
    DMA_Channel_TypeDef* dma_ch = get_channel_base(channel);
    
    // Disable channel
    DMA_Cmd(dma_ch, DISABLE);
    
    // Set status
    channel_status[channel - 1] = DMA_STATUS_IDLE;
}

/**
 * @brief ตรวจสอบสถานะของ DMA channel
 */
DMA_Status DMA_GetStatus(DMA_Channel channel) {
    uint32_t flag_base = ((channel - 1) * 4);
    
    // Check transfer complete flag
    if (DMA_GetFlagStatus(DMA1_FLAG_TC1 << flag_base) != RESET) {
        channel_status[channel - 1] = DMA_STATUS_COMPLETE;
    }
    
    // Check error flag
    if (DMA_GetFlagStatus(DMA1_FLAG_TE1 << flag_base) != RESET) {
        channel_status[channel - 1] = DMA_STATUS_ERROR;
    }
    
    return channel_status[channel - 1];
}

/**
 * @brief รอให้การถ่ายโอนเสร็จสิ้น (blocking)
 */
uint8_t DMA_WaitComplete(DMA_Channel channel, uint32_t timeout_ms) {
    // uint32_t start_time = 0;  // Reserved for future timeout implementation
    uint8_t use_timeout = (timeout_ms > 0);
    
    // Note: ต้องมี timer/delay library สำหรับ timeout
    // สำหรับตอนนี้ใช้ simple polling
    
    while (1) {
        DMA_Status status = DMA_GetStatus(channel);
        
        if (status == DMA_STATUS_COMPLETE) {
            return 1;
        }
        
        if (status == DMA_STATUS_ERROR) {
            return 0;
        }
        
        // TODO: Implement timeout checking with millis()
        if (use_timeout) {
            // Placeholder for timeout logic
        }
    }
}

/**
 * @brief ตั้งค่า callback function สำหรับ Transfer Complete
 */
void DMA_SetTransferCompleteCallback(DMA_Channel channel, DMA_TransferCompleteCallback callback) {
    transfer_complete_callbacks[channel - 1] = callback;
    
    // Enable TC interrupt
    DMA_Channel_TypeDef* dma_ch = get_channel_base(channel);
    DMA_ITConfig(dma_ch, DMA_IT_TC, ENABLE);
    
    // Enable NVIC
    IRQn_Type irqn = get_channel_irqn(channel);
    NVIC_EnableIRQ(irqn);
}

/**
 * @brief ตั้งค่า callback function สำหรับ Error
 */
void DMA_SetErrorCallback(DMA_Channel channel, DMA_ErrorCallback callback) {
    error_callbacks[channel - 1] = callback;
    
    // Enable TE interrupt
    DMA_Channel_TypeDef* dma_ch = get_channel_base(channel);
    DMA_ITConfig(dma_ch, DMA_IT_TE, ENABLE);
    
    // Enable NVIC
    IRQn_Type irqn = get_channel_irqn(channel);
    NVIC_EnableIRQ(irqn);
}

/**
 * @brief รีเซ็ต DMA channel
 */
void DMA_Reset(DMA_Channel channel) {
    DMA_Channel_TypeDef* dma_ch = get_channel_base(channel);
    
    // Disable channel
    DMA_Cmd(dma_ch, DISABLE);
    
    // Clear all flags
    uint32_t flag_base = ((channel - 1) * 4);
    DMA_ClearFlag(DMA1_FLAG_GL1 << flag_base);
    
    // Reset status
    channel_status[channel - 1] = DMA_STATUS_IDLE;
}

/**
 * @brief ตรวจสอบจำนวนข้อมูลที่เหลืออยู่
 */
uint16_t DMA_GetRemainingCount(DMA_Channel channel) {
    DMA_Channel_TypeDef* dma_ch = get_channel_base(channel);
    return DMA_GetCurrDataCounter(dma_ch);
}

/* ----- Memory Transfer Functions ----- */

/**
 * @brief Copy memory ด้วย DMA (blocking)
 */
void DMA_MemCopy(void* dst, const void* src, uint16_t size) {
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
        .buffer_size = size
    };
    
    DMA_SimpleInit(&config);
    DMA_Start(DMA_CH1);
    DMA_WaitComplete(DMA_CH1, 0);
}

/**
 * @brief Copy memory ด้วย DMA (non-blocking)
 */
void DMA_MemCopyAsync(DMA_Channel channel, void* dst, const void* src, uint16_t size) {
    DMA_Config_t config = {
        .channel = channel,
        .direction = DMA_DIR_MEM_TO_MEM,
        .priority = DMA_PRIORITY_HIGH,
        .data_size = DMA_SIZE_BYTE,
        .mode = DMA_MODE_NORMAL,
        .mem_increment = 1,
        .periph_increment = 1,
        .periph_addr = (uint32_t)src,
        .mem_addr = (uint32_t)dst,
        .buffer_size = size
    };
    
    DMA_SimpleInit(&config);
    DMA_Start(channel);
}

/**
 * @brief Set memory ด้วย DMA
 */
void DMA_MemSet(void* dst, uint8_t value, uint16_t size) {
    // Create a single-byte source
    static uint8_t fill_value;
    fill_value = value;
    
    DMA_Config_t config = {
        .channel = DMA_CH1,
        .direction = DMA_DIR_MEM_TO_MEM,
        .priority = DMA_PRIORITY_HIGH,
        .data_size = DMA_SIZE_BYTE,
        .mode = DMA_MODE_NORMAL,
        .mem_increment = 1,
        .periph_increment = 0,  // Don't increment source
        .periph_addr = (uint32_t)&fill_value,
        .mem_addr = (uint32_t)dst,
        .buffer_size = size
    };
    
    DMA_SimpleInit(&config);
    DMA_Start(DMA_CH1);
    DMA_WaitComplete(DMA_CH1, 0);
}

/* ----- ADC Integration Functions ----- */

/**
 * @brief เริ่มต้น DMA สำหรับ ADC continuous conversion
 */
void DMA_ADC_Init(DMA_Channel channel, uint16_t* buffer, uint16_t buffer_size, uint8_t circular) {
    DMA_Config_t config = {
        .channel = channel,
        .direction = DMA_DIR_PERIPH_TO_MEM,
        .priority = DMA_PRIORITY_HIGH,
        .data_size = DMA_SIZE_HALFWORD,  // ADC is 10-bit, stored in 16-bit
        .mode = circular ? DMA_MODE_CIRCULAR : DMA_MODE_NORMAL,
        .mem_increment = 1,
        .periph_increment = 0,
        .periph_addr = (uint32_t)&ADC1->RDATAR,
        .mem_addr = (uint32_t)buffer,
        .buffer_size = buffer_size
    };
    
    DMA_SimpleInit(&config);
    
    // Enable ADC DMA
    ADC_DMACmd(ADC1, ENABLE);
}

/**
 * @brief เริ่มต้น DMA สำหรับ ADC multi-channel
 */
void DMA_ADC_InitMultiChannel(DMA_Channel channel, uint16_t* buffer, uint8_t num_channels, uint16_t samples_per_channel) {
    uint16_t total_size = num_channels * samples_per_channel;
    DMA_ADC_Init(channel, buffer, total_size, 1);  // Use circular mode
}

/* ----- USART Integration Functions ----- */

/**
 * @brief เริ่มต้น DMA สำหรับ USART transmission
 */
void DMA_USART_InitTx(DMA_Channel channel, uint8_t* buffer, uint16_t buffer_size) {
    DMA_Config_t config = {
        .channel = channel,
        .direction = DMA_DIR_MEM_TO_PERIPH,
        .priority = DMA_PRIORITY_MEDIUM,
        .data_size = DMA_SIZE_BYTE,
        .mode = DMA_MODE_NORMAL,
        .mem_increment = 1,
        .periph_increment = 0,
        .periph_addr = (uint32_t)&USART1->DATAR,
        .mem_addr = (uint32_t)buffer,
        .buffer_size = buffer_size
    };
    
    DMA_SimpleInit(&config);
    
    // Enable USART DMA TX
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

/**
 * @brief เริ่มต้น DMA สำหรับ USART reception
 */
void DMA_USART_InitRx(DMA_Channel channel, uint8_t* buffer, uint16_t buffer_size, uint8_t circular) {
    DMA_Config_t config = {
        .channel = channel,
        .direction = DMA_DIR_PERIPH_TO_MEM,
        .priority = DMA_PRIORITY_MEDIUM,
        .data_size = DMA_SIZE_BYTE,
        .mode = circular ? DMA_MODE_CIRCULAR : DMA_MODE_NORMAL,
        .mem_increment = 1,
        .periph_increment = 0,
        .periph_addr = (uint32_t)&USART1->DATAR,
        .mem_addr = (uint32_t)buffer,
        .buffer_size = buffer_size
    };
    
    DMA_SimpleInit(&config);
    
    // Enable USART DMA RX
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
}

/**
 * @brief ส่งข้อมูลผ่าน USART ด้วย DMA
 */
void DMA_USART_Transmit(DMA_Channel channel, const uint8_t* data, uint16_t length) {
    DMA_Channel_TypeDef* dma_ch = get_channel_base(channel);
    
    // Wait for previous transfer to complete
    while (DMA_GetStatus(channel) == DMA_STATUS_BUSY);
    
    // Update transfer size and memory address
    DMA_Cmd(dma_ch, DISABLE);
    dma_ch->MADDR = (uint32_t)data;
    dma_ch->CNTR = length;
    
    // Start transfer
    DMA_Start(channel);
}

/**
 * @brief ตรวจสอบจำนวนข้อมูลที่รับได้ (สำหรับ circular mode)
 */
uint16_t DMA_USART_GetReceivedCount(DMA_Channel channel, uint16_t buffer_size) {
    uint16_t remaining = DMA_GetRemainingCount(channel);
    return buffer_size - remaining;
}

/* ----- SPI Integration Functions ----- */

/**
 * @brief เริ่มต้น DMA สำหรับ SPI transmission
 */
void DMA_SPI_Init(DMA_Channel tx_channel, DMA_Channel rx_channel) {
    // TX configuration
    DMA_Config_t tx_config = {
        .channel = tx_channel,
        .direction = DMA_DIR_MEM_TO_PERIPH,
        .priority = DMA_PRIORITY_HIGH,
        .data_size = DMA_SIZE_BYTE,
        .mode = DMA_MODE_NORMAL,
        .mem_increment = 1,
        .periph_increment = 0,
        .periph_addr = (uint32_t)&SPI1->DATAR,
        .mem_addr = 0,  // Will be set later
        .buffer_size = 0  // Will be set later
    };
    
    DMA_SimpleInit(&tx_config);
    
    // RX configuration
    DMA_Config_t rx_config = {
        .channel = rx_channel,
        .direction = DMA_DIR_PERIPH_TO_MEM,
        .priority = DMA_PRIORITY_HIGH,
        .data_size = DMA_SIZE_BYTE,
        .mode = DMA_MODE_NORMAL,
        .mem_increment = 1,
        .periph_increment = 0,
        .periph_addr = (uint32_t)&SPI1->DATAR,
        .mem_addr = 0,  // Will be set later
        .buffer_size = 0  // Will be set later
    };
    
    DMA_SimpleInit(&rx_config);
    
    // Enable SPI DMA
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
}

/**
 * @brief ส่งและรับข้อมูลผ่าน SPI ด้วย DMA
 */
void DMA_SPI_TransferBuffer(DMA_Channel tx_channel, DMA_Channel rx_channel, 
                           const uint8_t* tx_data, uint8_t* rx_data, uint16_t length) {
    DMA_Channel_TypeDef* tx_ch = get_channel_base(tx_channel);
    DMA_Channel_TypeDef* rx_ch = get_channel_base(rx_channel);
    
    // Configure RX
    DMA_Cmd(rx_ch, DISABLE);
    rx_ch->MADDR = (uint32_t)rx_data;
    rx_ch->CNTR = length;
    
    // Configure TX
    DMA_Cmd(tx_ch, DISABLE);
    tx_ch->MADDR = (uint32_t)tx_data;
    tx_ch->CNTR = length;
    
    // Start RX first, then TX
    DMA_Start(rx_channel);
    DMA_Start(tx_channel);
    
    // Wait for completion
    DMA_WaitComplete(tx_channel, 0);
    DMA_WaitComplete(rx_channel, 0);
}

/* ----- Simplified analogRead DMA Functions ----- */

// Private variables for analogRead DMA
static DMA_Channel adc_dma_channel = DMA_CH1;
static uint8_t adc_dma_active = 0;

/**
 * @brief เริ่มต้น DMA สำหรับ analogRead แบบง่าย
 */
void DMA_analogReadStart(uint8_t pin, uint16_t* buffer, uint16_t buffer_size, uint8_t continuous) {
    // ตรวจสอบว่าเป็น ADC pin หรือไม่
    uint8_t adc_ch;
    
    // แปลง GPIO pin เป็น ADC channel
    switch(pin) {
        case 0x12: adc_ch = ADC_Channel_0; break;  // PA2
        case 0x11: adc_ch = ADC_Channel_1; break;  // PA1
        case 0x24: adc_ch = ADC_Channel_2; break;  // PC4
        case 0x32: adc_ch = ADC_Channel_3; break;  // PD2
        case 0x33: adc_ch = ADC_Channel_4; break;  // PD3
        case 0x35: adc_ch = ADC_Channel_5; break;  // PD5
        case 0x36: adc_ch = ADC_Channel_6; break;  // PD6
        case 0x34: adc_ch = ADC_Channel_7; break;  // PD4
        default:
            // ไม่ใช่ ADC pin
            return;
    }
    
    // Configure GPIO as analog input (using SimpleADC)
    ADC_EnableChannel((ADC_Channel)adc_ch);
    
    // ตั้งค่า ADC สำหรับ continuous conversion
    ADC_InitTypeDef ADC_InitStructure = {0};
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC1, adc_ch, 1, ADC_SampleTime_241Cycles);
    
    ADC_Cmd(ADC1, ENABLE);
    
    // Calibration
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    
    // ตั้งค่า DMA
    DMA_ADC_Init(adc_dma_channel, buffer, buffer_size, continuous);
    DMA_Start(adc_dma_channel);
    
    // เริ่ม ADC conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    adc_dma_active = 1;
}

/**
 * @brief อ่านค่าเฉลี่ยจาก DMA buffer
 */
uint16_t DMA_analogReadAverage(uint16_t* buffer, uint16_t buffer_size) {
    uint32_t sum = 0;
    
    for (uint16_t i = 0; i < buffer_size; i++) {
        sum += buffer[i];
    }
    
    return (uint16_t)(sum / buffer_size);
}

/**
 * @brief หยุดการอ่าน ADC ด้วย DMA
 */
void DMA_analogReadStop(void) {
    if (adc_dma_active) {
        // หยุด ADC
        ADC_SoftwareStartConvCmd(ADC1, DISABLE);
        ADC_Cmd(ADC1, DISABLE);
        
        // หยุด DMA
        DMA_Stop(adc_dma_channel);
        
        adc_dma_active = 0;
    }
}

/**
 * @brief ตรวจสอบว่า DMA ADC กำลังทำงานหรือไม่
 */
uint8_t DMA_analogReadBusy(void) {
    return adc_dma_active;
}

/* ----- Helper Functions ----- */

/**
 * @brief แปลง DMA_Channel เป็น DMA_Channel_TypeDef*
 */
DMA_Channel_TypeDef* DMA_GetChannelBase(DMA_Channel channel) {
    return get_channel_base(channel);
}

/**
 * @brief แปลง DMA_Channel เป็น IRQn
 */
IRQn_Type DMA_GetChannelIRQn(DMA_Channel channel) {
    return get_channel_irqn(channel);
}

/**
 * @brief เปิดใช้งาน DMA interrupt
 */
void DMA_EnableInterrupt(DMA_Channel channel, uint8_t enable) {
    IRQn_Type irqn = get_channel_irqn(channel);
    
    if (enable) {
        NVIC_EnableIRQ(irqn);
    } else {
        NVIC_DisableIRQ(irqn);
    }
}

/* ========== Private Functions ========== */

/**
 * @brief Get DMA channel base address
 */
static DMA_Channel_TypeDef* get_channel_base(DMA_Channel channel) {
    switch (channel) {
        case DMA_CH1: return DMA1_Channel1;
        case DMA_CH2: return DMA1_Channel2;
        case DMA_CH3: return DMA1_Channel3;
        case DMA_CH4: return DMA1_Channel4;
        case DMA_CH5: return DMA1_Channel5;
        case DMA_CH6: return DMA1_Channel6;
        case DMA_CH7: return DMA1_Channel7;
        default: return DMA1_Channel1;
    }
}

/**
 * @brief Get DMA channel IRQ number
 */
static IRQn_Type get_channel_irqn(DMA_Channel channel) {
    switch (channel) {
        case DMA_CH1: return DMA1_Channel1_IRQn;
        case DMA_CH2: return DMA1_Channel2_IRQn;
        case DMA_CH3: return DMA1_Channel3_IRQn;
        case DMA_CH4: return DMA1_Channel4_IRQn;
        case DMA_CH5: return DMA1_Channel5_IRQn;
        case DMA_CH6: return DMA1_Channel6_IRQn;
        case DMA_CH7: return DMA1_Channel7_IRQn;
        default: return DMA1_Channel1_IRQn;
    }
}

/**
 * @brief Enable DMA clock
 */
static void enable_dma_clock(void) {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
}

/* ========== Interrupt Handlers ========== */

/**
 * @brief DMA Channel 1 interrupt handler
 */
void DMA1_Channel1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel1_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC1) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC1);
        channel_status[0] = DMA_STATUS_COMPLETE;
        if (transfer_complete_callbacks[0] != NULL) {
            transfer_complete_callbacks[0](DMA_CH1);
        }
    }
    
    if (DMA_GetITStatus(DMA1_IT_TE1) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TE1);
        channel_status[0] = DMA_STATUS_ERROR;
        if (error_callbacks[0] != NULL) {
            error_callbacks[0](DMA_CH1);
        }
    }
}

/**
 * @brief DMA Channel 2 interrupt handler
 */
void DMA1_Channel2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel2_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC2) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC2);
        channel_status[1] = DMA_STATUS_COMPLETE;
        if (transfer_complete_callbacks[1] != NULL) {
            transfer_complete_callbacks[1](DMA_CH2);
        }
    }
    
    if (DMA_GetITStatus(DMA1_IT_TE2) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TE2);
        channel_status[1] = DMA_STATUS_ERROR;
        if (error_callbacks[1] != NULL) {
            error_callbacks[1](DMA_CH2);
        }
    }
}

/**
 * @brief DMA Channel 3 interrupt handler
 */
void DMA1_Channel3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel3_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC3) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC3);
        channel_status[2] = DMA_STATUS_COMPLETE;
        if (transfer_complete_callbacks[2] != NULL) {
            transfer_complete_callbacks[2](DMA_CH3);
        }
    }
    
    if (DMA_GetITStatus(DMA1_IT_TE3) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TE3);
        channel_status[2] = DMA_STATUS_ERROR;
        if (error_callbacks[2] != NULL) {
            error_callbacks[2](DMA_CH3);
        }
    }
}

/**
 * @brief DMA Channel 4 interrupt handler
 */
void DMA1_Channel4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel4_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC4) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC4);
        channel_status[3] = DMA_STATUS_COMPLETE;
        if (transfer_complete_callbacks[3] != NULL) {
            transfer_complete_callbacks[3](DMA_CH4);
        }
    }
    
    if (DMA_GetITStatus(DMA1_IT_TE4) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TE4);
        channel_status[3] = DMA_STATUS_ERROR;
        if (error_callbacks[3] != NULL) {
            error_callbacks[3](DMA_CH4);
        }
    }
}

/**
 * @brief DMA Channel 5 interrupt handler
 */
void DMA1_Channel5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel5_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC5) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC5);
        channel_status[4] = DMA_STATUS_COMPLETE;
        if (transfer_complete_callbacks[4] != NULL) {
            transfer_complete_callbacks[4](DMA_CH5);
        }
    }
    
    if (DMA_GetITStatus(DMA1_IT_TE5) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TE5);
        channel_status[4] = DMA_STATUS_ERROR;
        if (error_callbacks[4] != NULL) {
            error_callbacks[4](DMA_CH5);
        }
    }
}

/**
 * @brief DMA Channel 6 interrupt handler
 */
void DMA1_Channel6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel6_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC6) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC6);
        channel_status[5] = DMA_STATUS_COMPLETE;
        if (transfer_complete_callbacks[5] != NULL) {
            transfer_complete_callbacks[5](DMA_CH6);
        }
    }
    
    if (DMA_GetITStatus(DMA1_IT_TE6) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TE6);
        channel_status[5] = DMA_STATUS_ERROR;
        if (error_callbacks[5] != NULL) {
            error_callbacks[5](DMA_CH6);
        }
    }
}

/**
 * @brief DMA Channel 7 interrupt handler
 */
void DMA1_Channel7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel7_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC7) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC7);
        channel_status[6] = DMA_STATUS_COMPLETE;
        if (transfer_complete_callbacks[6] != NULL) {
            transfer_complete_callbacks[6](DMA_CH7);
        }
    }
    
    if (DMA_GetITStatus(DMA1_IT_TE7) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TE7);
        channel_status[6] = DMA_STATUS_ERROR;
        if (error_callbacks[6] != NULL) {
            error_callbacks[6](DMA_CH7);
        }
    }
}
