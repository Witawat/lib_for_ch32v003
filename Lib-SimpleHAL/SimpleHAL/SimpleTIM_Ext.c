/**
 * @file SimpleTIM_Ext.c
 * @brief SimpleTIM Extensions Implementation
 * @version 1.0
 * @date 2025-12-21
 */

#include "SimpleTIM_Ext.h"
#include <stdio.h>

/* ========== Internal State Variables ========== */

// Stopwatch state
static volatile uint32_t stopwatch_ms = 0;           // เวลาที่ผ่านไป (milliseconds)
static volatile uint8_t stopwatch_running = 0;       // สถานะการทำงาน
static uint32_t stopwatch_initial_ms = 0;            // เวลาเริ่มต้น (สำหรับ init)

// Countdown state
static volatile uint32_t countdown_ms = 0;           // เวลาที่เหลือ (milliseconds)
static uint32_t countdown_initial_ms = 0;            // เวลาเริ่มต้น
static volatile uint8_t countdown_running = 0;       // สถานะการทำงาน
static volatile uint8_t countdown_finished = 0;      // สถานะหมดเวลา
static void (*countdown_alarm_callback)(void) = NULL; // Alarm callback

/* ========== Internal Helper Functions ========== */

/**
 * @brief Timer callback - เรียกทุก 1ms
 */
static void timer_ext_callback(void) {
    // Update stopwatch
    if (stopwatch_running) {
        stopwatch_ms++;
    }
    
    // Update countdown
    if (countdown_running && countdown_ms > 0) {
        countdown_ms--;
        
        // Check if finished
        if (countdown_ms == 0) {
            countdown_running = 0;
            countdown_finished = 1;
            
            // Call alarm callback
            if (countdown_alarm_callback != NULL) {
                countdown_alarm_callback();
            }
        }
    }
}

/**
 * @brief แปลง milliseconds เป็น Time_t (normalized mode)
 */
static void ms_to_time_normalized(uint32_t ms, Time_t* time) {
    uint32_t total_seconds = ms / 1000;
    
    time->hours = total_seconds / 3600;
    time->minutes = (total_seconds % 3600) / 60;
    time->seconds = total_seconds % 60;
}

/**
 * @brief แปลง milliseconds เป็น Time_t (raw mode สำหรับ SS format)
 */
static void ms_to_time_raw_ss(uint32_t ms, Time_t* time) {
    time->hours = 0;
    time->minutes = 0;
    time->seconds = ms / 1000;  // ไม่จำกัดค่า
}

/**
 * @brief แปลง milliseconds เป็น Time_t (raw mode สำหรับ MMSS format)
 */
static void ms_to_time_raw_mmss(uint32_t ms, Time_t* time) {
    uint32_t total_seconds = ms / 1000;
    
    time->hours = 0;
    time->minutes = total_seconds / 60;  // ไม่จำกัดค่า
    time->seconds = total_seconds % 60;
}

/**
 * @brief แปลง milliseconds เป็น Time_t (raw mode สำหรับ HHMMSS format)
 */
static void ms_to_time_raw_hhmmss(uint32_t ms, Time_t* time) {
    uint32_t total_seconds = ms / 1000;
    
    time->hours = total_seconds / 3600;  // ไม่จำกัดค่า
    time->minutes = (total_seconds % 3600) / 60;
    time->seconds = total_seconds % 60;
}

/* ========== Stopwatch Functions ========== */

void Stopwatch_Init(void) {
    // Initialize timer at 1000Hz (1ms)
    TIM_SimpleInit(TIM_2, 1000);
    TIM_AttachInterrupt(TIM_2, timer_ext_callback);
    TIM_Start(TIM_2);
    
    // Reset state
    stopwatch_ms = 0;
    stopwatch_running = 0;
    stopwatch_initial_ms = 0;
}

void Stopwatch_Start(void) {
    stopwatch_running = 1;
}

void Stopwatch_Stop(void) {
    stopwatch_running = 0;
}

void Stopwatch_Reset(void) {
    stopwatch_running = 0;
    stopwatch_ms = stopwatch_initial_ms;
}

void Stopwatch_GetTime(Time_t* time) {
    if (time == NULL) return;
    ms_to_time_normalized(stopwatch_ms, time);
}

void Stopwatch_GetTimeString(char* buffer, TimeFormat_t format, TimeDisplayMode_t mode) {
    if (buffer == NULL) return;
    
    Time_t time;
    
    // แปลงตาม mode
    if (mode == TIME_DISPLAY_NORMALIZED) {
        ms_to_time_normalized(stopwatch_ms, &time);
    } else {
        // RAW mode - แปลงตาม format
        switch (format) {
            case TIME_FORMAT_SS:
                ms_to_time_raw_ss(stopwatch_ms, &time);
                break;
            case TIME_FORMAT_MMSS:
                ms_to_time_raw_mmss(stopwatch_ms, &time);
                break;
            case TIME_FORMAT_HHMMSS:
                ms_to_time_raw_hhmmss(stopwatch_ms, &time);
                break;
        }
    }
    
    // Format string
    Time_ToString(&time, buffer, format, mode);
}

uint32_t Stopwatch_GetTotalSeconds(void) {
    return stopwatch_ms / 1000;
}

uint32_t Stopwatch_GetTotalMilliseconds(void) {
    return stopwatch_ms;
}

uint8_t Stopwatch_IsRunning(void) {
    return stopwatch_running;
}

/* ========== Countdown Functions ========== */

void Countdown_Init(uint16_t hours, uint8_t minutes, uint8_t seconds) {
    // แปลงเป็น milliseconds
    countdown_initial_ms = ((uint32_t)hours * 3600 + (uint32_t)minutes * 60 + seconds) * 1000;
    countdown_ms = countdown_initial_ms;
    countdown_running = 0;
    countdown_finished = 0;
    
    // Initialize timer if not already done
    static uint8_t timer_initialized = 0;
    if (!timer_initialized) {
        TIM_SimpleInit(TIM_2, 1000);
        TIM_AttachInterrupt(TIM_2, timer_ext_callback);
        TIM_Start(TIM_2);
        timer_initialized = 1;
    }
}

void Countdown_InitFromSeconds(uint32_t total_seconds) {
    countdown_initial_ms = total_seconds * 1000;
    countdown_ms = countdown_initial_ms;
    countdown_running = 0;
    countdown_finished = 0;
    
    // Initialize timer if not already done
    static uint8_t timer_initialized = 0;
    if (!timer_initialized) {
        TIM_SimpleInit(TIM_2, 1000);
        TIM_AttachInterrupt(TIM_2, timer_ext_callback);
        TIM_Start(TIM_2);
        timer_initialized = 1;
    }
}

void Countdown_Start(void) {
    if (countdown_ms > 0) {
        countdown_running = 1;
        countdown_finished = 0;
    }
}

void Countdown_Stop(void) {
    countdown_running = 0;
}

void Countdown_Reset(void) {
    countdown_running = 0;
    countdown_ms = countdown_initial_ms;
    countdown_finished = 0;
}

void Countdown_GetTime(Time_t* time) {
    if (time == NULL) return;
    ms_to_time_normalized(countdown_ms, time);
}

void Countdown_GetTimeString(char* buffer, TimeFormat_t format, TimeDisplayMode_t mode) {
    if (buffer == NULL) return;
    
    Time_t time;
    
    // แปลงตาม mode
    if (mode == TIME_DISPLAY_NORMALIZED) {
        ms_to_time_normalized(countdown_ms, &time);
    } else {
        // RAW mode - แปลงตาม format
        switch (format) {
            case TIME_FORMAT_SS:
                ms_to_time_raw_ss(countdown_ms, &time);
                break;
            case TIME_FORMAT_MMSS:
                ms_to_time_raw_mmss(countdown_ms, &time);
                break;
            case TIME_FORMAT_HHMMSS:
                ms_to_time_raw_hhmmss(countdown_ms, &time);
                break;
        }
    }
    
    // Format string
    Time_ToString(&time, buffer, format, mode);
}

uint8_t Countdown_IsFinished(void) {
    return countdown_finished;
}

void Countdown_SetAlarmCallback(void (*callback)(void)) {
    countdown_alarm_callback = callback;
}

uint32_t Countdown_GetRemainingSeconds(void) {
    return countdown_ms / 1000;
}

uint8_t Countdown_IsRunning(void) {
    return countdown_running;
}

/* ========== Utility Functions ========== */

void Time_ToString(Time_t* time, char* buffer, TimeFormat_t format, TimeDisplayMode_t mode) {
    if (time == NULL || buffer == NULL) return;
    
    switch (format) {
        case TIME_FORMAT_HHMMSS:
            if (mode == TIME_DISPLAY_NORMALIZED) {
                // Normalized: "02:30:45" (zero-padded)
                sprintf(buffer, "%02u:%02u:%02u", time->hours, time->minutes, time->seconds);
            } else {
                // Raw: hours ไม่จำกัด, minutes และ seconds zero-padded
                sprintf(buffer, "%u:%02u:%02u", time->hours, time->minutes, time->seconds);
            }
            break;
            
        case TIME_FORMAT_MMSS:
            if (mode == TIME_DISPLAY_NORMALIZED) {
                // Normalized: "30:45" (zero-padded)
                sprintf(buffer, "%02u:%02u", time->minutes, time->seconds);
            } else {
                // Raw: minutes ไม่จำกัด, seconds zero-padded
                sprintf(buffer, "%u:%02u", time->minutes, time->seconds);
            }
            break;
            
        case TIME_FORMAT_SS:
            // SS format: แสดงเป็นตัวเลขเท่านั้น
            sprintf(buffer, "%u", time->seconds);
            break;
    }
}

void Time_FromSeconds(uint32_t total_seconds, Time_t* time, TimeDisplayMode_t mode) {
    if (time == NULL) return;
    
    if (mode == TIME_DISPLAY_NORMALIZED) {
        // Normalized: แปลงตามปกติ
        time->hours = total_seconds / 3600;
        time->minutes = (total_seconds % 3600) / 60;
        time->seconds = total_seconds % 60;
    } else {
        // Raw: เก็บค่าทั้งหมดใน seconds
        // (ผู้ใช้ต้องเลือก format ที่เหมาะสมเอง)
        time->hours = 0;
        time->minutes = 0;
        time->seconds = total_seconds;
    }
}

uint32_t Time_ToSeconds(Time_t* time) {
    if (time == NULL) return 0;
    
    return (uint32_t)time->hours * 3600 + (uint32_t)time->minutes * 60 + time->seconds;
}
