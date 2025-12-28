/**
 * @file 03_ADC_DMA.c
 * @brief ตัวอย่างการใช้ DMA กับ ADC Continuous Conversion
 * @version 1.0
 * @date 2025-12-22
 * 
 * @details
 * ตัวอย่างนี้แสดงการใช้ DMA เพื่ออ่านค่า ADC แบบต่อเนื่อง (continuous mode)
 * โดยใช้ circular buffer เก็บข้อมูล ADC หลายตัวอย่างพร้อมกัน
 * 
 * **สิ่งที่เรียนรู้:**
 * - การใช้ DMA กับ ADC continuous conversion
 * - Circular buffer mode
 * - การอ่านค่า ADC หลาย channels พร้อมกัน
 * - การคำนวณค่าเฉลี่ยจาก buffer
 */

#include "SimpleHAL/SimpleHAL.h"
#include <stdio.h>

/* ========== Configuration ========== */

#define NUM_CHANNELS        3      // จำนวน ADC channels
#define SAMPLES_PER_CHANNEL 10     // จำนวนตัวอย่างต่อ channel
#define TOTAL_SAMPLES       (NUM_CHANNELS * SAMPLES_PER_CHANNEL)

// ADC channels ที่ใช้
ADC_Channel adc_channels[NUM_CHANNELS] = {
    ADC_CH_PA2,  // Channel 0
    ADC_CH_PA1,  // Channel 1
    ADC_CH_PC4   // Channel 2
};

/* ========== Global Variables ========== */

uint16_t adc_buffer[TOTAL_SAMPLES];  // Buffer สำหรับเก็บค่า ADC
volatile uint8_t conversion_complete = 0;

/* ========== Callback Functions ========== */

void on_adc_complete(DMA_Channel channel) {
    conversion_complete = 1;
}

/* ========== Function Prototypes ========== */

void setup_adc_multi_channel(void);
uint16_t calculate_average(uint16_t* buffer, uint8_t channel_index);
void print_adc_values(void);

/* ========== Main Function ========== */

int main(void) {
    // เริ่มต้นระบบ
    SystemCoreClockUpdate();
    Delay_Init();
    
    // เริ่มต้น USART
    USART_SimpleInit(BAUD_115200, USART_PINS_DEFAULT);
    
    printf("\r\n=== ADC + DMA Continuous Conversion ===\r\n");
    printf("Channels: %d\r\n", NUM_CHANNELS);
    printf("Samples per channel: %d\r\n", SAMPLES_PER_CHANNEL);
    printf("Total buffer size: %d\r\n\r\n", TOTAL_SAMPLES);
    
    // ตั้งค่า ADC multi-channel
    setup_adc_multi_channel();
    
    // ตั้งค่า DMA สำหรับ ADC
    printf("Initializing DMA for ADC...\r\n");
    DMA_ADC_InitMultiChannel(DMA_CH1, adc_buffer, NUM_CHANNELS, SAMPLES_PER_CHANNEL);
    
    // ตั้งค่า callback (optional)
    DMA_SetTransferCompleteCallback(DMA_CH1, on_adc_complete);
    
    // เริ่ม DMA
    DMA_Start(DMA_CH1);
    
    // เริ่ม ADC continuous conversion
    printf("Starting ADC continuous conversion...\r\n\r\n");
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    // รอให้ buffer เต็มครั้งแรก
    Delay_Ms(100);
    
    printf("=== ADC Readings ===\r\n\r\n");
    
    while (1) {
        // แสดงค่า ADC ทุกๆ 500ms
        print_adc_values();
        Delay_Ms(500);
    }
}

/* ========== Helper Functions ========== */

/**
 * @brief ตั้งค่า ADC สำหรับ multi-channel continuous conversion
 */
void setup_adc_multi_channel(void) {
    ADC_InitTypeDef ADC_InitStructure = {0};
    
    // เปิด Clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    
    // เปิด GPIO pins สำหรับ ADC
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        ADC_EnableChannel(adc_channels[i]);
    }
    
    // ตั้งค่า ADC
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;  // Scan mode สำหรับ multi-channel
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  // Continuous mode
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = NUM_CHANNELS;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    // ตั้งค่า conversion sequence
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        ADC_RegularChannelConfig(ADC1, adc_channels[i], i + 1, ADC_SampleTime_241Cycles);
    }
    
    // เปิดใช้งาน ADC
    ADC_Cmd(ADC1, ENABLE);
    
    // Calibration
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

/**
 * @brief คำนวณค่าเฉลี่ยของ channel
 */
uint16_t calculate_average(uint16_t* buffer, uint8_t channel_index) {
    uint32_t sum = 0;
    
    // Buffer เก็บข้อมูลแบบ interleaved: [CH0, CH1, CH2, CH0, CH1, CH2, ...]
    for (uint8_t i = 0; i < SAMPLES_PER_CHANNEL; i++) {
        uint16_t index = i * NUM_CHANNELS + channel_index;
        sum += buffer[index];
    }
    
    return (uint16_t)(sum / SAMPLES_PER_CHANNEL);
}

/**
 * @brief แสดงค่า ADC ทุก channels
 */
void print_adc_values(void) {
    printf("ADC Values:\r\n");
    
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++) {
        uint16_t avg = calculate_average(adc_buffer, ch);
        float voltage = ADC_ToVoltage(avg, 3.3);
        
        printf("  CH%d (", ch);
        
        // แสดงชื่อ pin
        switch (adc_channels[ch]) {
            case ADC_CH_PA2: printf("PA2"); break;
            case ADC_CH_PA1: printf("PA1"); break;
            case ADC_CH_PC4: printf("PC4"); break;
            default: printf("???"); break;
        }
        
        printf("): %4d (%.3fV)\r\n", avg, voltage);
    }
    
    printf("\r\n");
}

/**
 * @brief ข้อสังเกต
 * 
 * **ข้อดีของการใช้ DMA กับ ADC:**
 * - CPU ไม่ต้องรอ ADC conversion
 * - สามารถเก็บข้อมูลหลายตัวอย่างอัตโนมัติ
 * - Circular mode ทำให้ได้ข้อมูลต่อเนื่อง
 * - เหมาะสำหรับ data acquisition ความเร็วสูง
 * 
 * **การใช้งานจริง:**
 * - อ่านค่า sensor หลายตัวพร้อมกัน
 * - Data logging
 * - Signal processing (FFT, filtering)
 * - Audio sampling
 */
