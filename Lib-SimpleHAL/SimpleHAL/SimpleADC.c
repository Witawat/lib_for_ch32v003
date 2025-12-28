/**
 * @file SimpleADC.c
 * @brief Simple ADC Library Implementation
 * @version 1.0
 * @date 2025-12-12
 */

#include "SimpleADC.h"
#include "SimpleDelay.h"

/* ========== Private Helper Functions ========== */

/**
 * @brief แปลง ADC_Channel เป็น ADC_Channel_x
 */
static uint8_t GetADCChannel(ADC_Channel channel) {
  // ADC channels 0-7 แมปตรงกับ hardware channels
  return (uint8_t)channel;
}

/**
 * @brief แปลง ADC_Channel เป็น GPIO_TypeDef port
 */
static GPIO_TypeDef* GetGPIOPort(ADC_Channel channel) {
  switch (channel) {
  case ADC_CH_0:  // PA2
  case ADC_CH_1:  // PA1
    return GPIOA;
  case ADC_CH_2:  // PC4
    return GPIOC;
  case ADC_CH_3:  // PD2
  case ADC_CH_4:  // PD3
  case ADC_CH_5:  // PD5
  case ADC_CH_6:  // PD6
  case ADC_CH_7:  // PD4
  default:
    return GPIOD;
  }
}

/**
 * @brief แปลง ADC_Channel เป็น GPIO_Pin
 */
static uint16_t GetGPIOPin(ADC_Channel channel) {
  switch (channel) {
  case ADC_CH_0:  return GPIO_Pin_2;  // PA2
  case ADC_CH_1:  return GPIO_Pin_1;  // PA1
  case ADC_CH_2:  return GPIO_Pin_4;  // PC4
  case ADC_CH_3:  return GPIO_Pin_2;  // PD2
  case ADC_CH_4:  return GPIO_Pin_3;  // PD3
  case ADC_CH_5:  return GPIO_Pin_5;  // PD5
  case ADC_CH_6:  return GPIO_Pin_6;  // PD6
  case ADC_CH_7:  return GPIO_Pin_4;  // PD4
  default:        return GPIO_Pin_2;  // Default PD2
  }
}

/* ========== Public Functions ========== */

/**
 * @brief เริ่มต้น ADC peripheral (internal)
 */
static void ADC_InitPeripheral(void) {
  ADC_InitTypeDef ADC_InitStructure = {0};

  // เปิด Clock สำหรับ ADC
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_ADCCLKConfig(RCC_PCLK2_Div8); // ADC Clock = PCLK2/8

  // ตั้งค่า ADC
  ADC_DeInit(ADC1);
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  // เปิดใช้งาน ADC
  ADC_Cmd(ADC1, ENABLE);

  // Calibrate ADC
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1))
    ;
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1))
    ;
}

/**
 * @brief เปิดใช้งาน ADC channel เพิ่มเติม
 */
void ADC_EnableChannel(ADC_Channel channel) {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  GPIO_TypeDef* gpio_port = GetGPIOPort(channel);
  uint16_t gpio_pin = GetGPIOPin(channel);

  // เปิด Clock สำหรับ GPIO port ที่เกี่ยวข้อง
  if (gpio_port == GPIOA) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  } else if (gpio_port == GPIOC) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  } else if (gpio_port == GPIOD) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
  }

  // ตั้งค่า GPIO เป็น Analog Input
  GPIO_InitStructure.GPIO_Pin = gpio_pin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(gpio_port, &GPIO_InitStructure);
}

/**
 * @brief เริ่มต้นการใช้งาน ADC แบบกำหนด channels เอง
 */
void ADC_SimpleInitChannels(ADC_Channel *channels, uint8_t count) {
  // เริ่มต้น ADC peripheral
  ADC_InitPeripheral();

  // เปิดใช้งานเฉพาะ channels ที่ระบุ
  for (uint8_t i = 0; i < count; i++) {
    ADC_EnableChannel(channels[i]);
  }
}

/**
 * @brief เริ่มต้นการใช้งาน ADC (เปิดทุก channels)
 */
void ADC_SimpleInit(void) {
  // เปิดทุก 8 channels
  ADC_Channel all_channels[] = {
    ADC_CH_0,  // PA2
    ADC_CH_1,  // PA1
    ADC_CH_2,  // PC4
    ADC_CH_3,  // PD2
    ADC_CH_4,  // PD3
    ADC_CH_5,  // PD5
    ADC_CH_6,  // PD6
    ADC_CH_7   // PD4
  };
  ADC_SimpleInitChannels(all_channels, 8);
}

/**
 * @brief อ่านค่า ADC จากช่องที่ระบุ
 */
uint16_t ADC_Read(ADC_Channel channel) {
  uint8_t adc_channel = GetADCChannel(channel);

  // ตั้งค่า channel และ sample time
  ADC_RegularChannelConfig(ADC1, adc_channel, 1, ADC_SampleTime_241Cycles);

  // เริ่ม conversion
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

  // รอจนกว่า conversion เสร็จ
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
    ;

  // อ่านค่า
  return ADC_GetConversionValue(ADC1);
}

/**
 * @brief อ่านค่า ADC จากหลายช่อง
 */
void ADC_ReadMultiple(ADC_Channel *channels, uint16_t *values, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    values[i] = ADC_Read(channels[i]);
  }
}

/**
 * @brief แปลงค่า ADC เป็น voltage
 */
float ADC_ToVoltage(uint16_t adc_value, float vref) {
  return ((float)adc_value / (float)ADC_MAX_VALUE) * vref;
}

/**
 * @brief อ่านค่า ADC และแปลงเป็น voltage ทันที
 */
float ADC_ReadVoltage(ADC_Channel channel, float vref) {
  uint16_t adc_value = ADC_Read(channel);
  return ADC_ToVoltage(adc_value, vref);
}

/**
 * @brief อ่านค่า ADC แบบ average หลายครั้ง
 */
uint16_t ADC_ReadAverage(ADC_Channel channel, uint8_t samples) {
  uint32_t sum = 0;

  for (uint8_t i = 0; i < samples; i++) {
    sum += ADC_Read(channel);
    Delay_Us(100); // หน่วงเวลาเล็กน้อยระหว่างการอ่าน
  }

  return (uint16_t)(sum / samples);
}

/**
 * @brief แปลงค่า ADC เป็นเปอร์เซ็นต์
 */
float ADC_ToPercent(uint16_t adc_value) {
  return ((float)adc_value / (float)ADC_MAX_VALUE) * 100.0f;
}

/* ========== Internal Channel Functions ========== */

/**
 * @brief อ่านค่า Internal Reference Voltage (Vrefint)
 */
uint16_t ADC_ReadVrefInt(void) {
  // ตั้งค่า channel และ sample time สำหรับ internal channel
  // Internal channels ต้องใช้ sample time ที่นานกว่า (241 cycles)
  ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 1, ADC_SampleTime_241Cycles);

  // เริ่ม conversion
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

  // รอจนกว่า conversion เสร็จ
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
    ;

  // อ่านค่า
  return ADC_GetConversionValue(ADC1);
}

/**
 * @brief คำนวณแรงดัน VDD จริงจาก Vrefint
 */
float ADC_GetVDD(void) {
  // อ่านค่า Vrefint หลายครั้งและหาค่าเฉลี่ย
  uint32_t sum = 0;
  for (uint8_t i = 0; i < 10; i++) {
    sum += ADC_ReadVrefInt();
    Delay_Us(100);
  }
  uint16_t vrefint_adc = (uint16_t)(sum / 10);

  // คำนวณ VDD: VDD = VREFINT_VOLTAGE × ADC_MAX_VALUE / vrefint_adc
  // ถ้า vrefint_adc = 0 ให้คืนค่า default 3.3V
  if (vrefint_adc == 0) {
    return 3.3f;
  }

  float vdd = (ADC_VREFINT_VOLTAGE * (float)ADC_MAX_VALUE) / (float)vrefint_adc;
  return vdd;
}

/**
 * @brief อ่านค่า ADC พร้อมชดเชยความผันผวนของ VDD
 */
float ADC_ReadVoltageCompensated(ADC_Channel channel) {
  // อ่าน VDD จริง
  float vdd = ADC_GetVDD();

  // อ่านค่า ADC
  uint16_t adc_value = ADC_Read(channel);

  // แปลงเป็นแรงดันโดยใช้ VDD ที่คำนวณได้
  return ((float)adc_value / (float)ADC_MAX_VALUE) * vdd;
}

/**
 * @brief คำนวณเปอร์เซ็นต์แบตเตอรี่
 */
float ADC_GetBatteryPercent(float vdd, float v_min, float v_max) {
  // คำนวณเปอร์เซ็นต์
  float percent = ((vdd - v_min) / (v_max - v_min)) * 100.0f;

  // Clamp ให้อยู่ในช่วง 0-100%
  if (percent < 0.0f) {
    percent = 0.0f;
  } else if (percent > 100.0f) {
    percent = 100.0f;
  }

  return percent;
}
