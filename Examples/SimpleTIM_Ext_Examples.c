/**
 * @file SimpleTIM_Ext_Examples.c
 * @brief ตัวอย่างการใช้งาน SimpleTIM_Ext Library
 * @version 1.0
 * @date 2025-12-21
 * 
 * @details
 * ไฟล์นี้มีตัวอย่างการใช้งาน SimpleTIM_Ext แบบต่างๆ
 * ตั้งแต่ basic ถึง project-level
 * คัดลอกโค้ดไปใส่ใน main.c เพื่อทดสอบ
 */

#include "../SimpleTIM_Ext.h"
#include "../SimpleGPIO.h"
#include "../Lib/Timer/timer.h"
#include <stdio.h>

/* ========== Example 1: Basic Stopwatch (HH:MM:SS) ========== */

/**
 * @brief ตัวอย่างการใช้ stopwatch พื้นฐาน
 * 
 * แสดงเวลาแบบ HH:MM:SS ทุก 1 วินาที
 */
void example_basic_stopwatch(void) {
    printf("=== Basic Stopwatch Example ===\r\n");
    
    char time_str[32];
    
    // เริ่มต้น stopwatch
    Stopwatch_Init();
    Stopwatch_Start();
    
    printf("Stopwatch started! Press Ctrl+C to stop.\r\n\r\n");
    
    while(1) {
        // อ่านเวลาแบบ normalized HH:MM:SS
        Stopwatch_GetTimeString(time_str, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);
        printf("\rTime: %s", time_str);
        
        Delay_Ms(1000);
    }
}

/* ========== Example 2: Basic Countdown (HH:MM:SS) ========== */

void countdown_alarm(void) {
    printf("\r\n\r\n*** TIME'S UP! ***\r\n");
}

/**
 * @brief ตัวอย่างการใช้ countdown timer พื้นฐาน
 * 
 * Countdown จาก 5 นาที พร้อม alarm
 */
void example_basic_countdown(void) {
    printf("=== Basic Countdown Example ===\r\n");
    
    char time_str[32];
    
    // ตั้งเวลา countdown 5 นาที
    Countdown_Init(0, 5, 0);
    Countdown_SetAlarmCallback(countdown_alarm);
    Countdown_Start();
    
    printf("Countdown started: 5 minutes\r\n\r\n");
    
    while(1) {
        if (!Countdown_IsFinished()) {
            Countdown_GetTimeString(time_str, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);
            printf("\rRemaining: %s", time_str);
            Delay_Ms(1000);
        } else {
            printf("\r\nCountdown finished!\r\n");
            break;
        }
    }
}

/* ========== Example 3: Stopwatch MM:SS Format ========== */

/**
 * @brief Stopwatch แสดงผลแบบ MM:SS
 * 
 * เหมาะสำหรับการจับเวลาสั้นๆ
 */
void example_stopwatch_mmss(void) {
    printf("=== Stopwatch MM:SS Format ===\r\n");
    
    char time_str[32];
    
    Stopwatch_Init();
    Stopwatch_Start();
    
    printf("Format: MM:SS (Minutes:Seconds)\r\n\r\n");
    
    while(1) {
        // แสดงแบบ normalized MM:SS
        Stopwatch_GetTimeString(time_str, TIME_FORMAT_MMSS, TIME_DISPLAY_NORMALIZED);
        printf("\rTime: %s", time_str);
        
        Delay_Ms(100);
    }
}

/* ========== Example 4: Countdown MM:SS Format ========== */

/**
 * @brief Kitchen timer style countdown
 * 
 * Countdown แบบ MM:SS เหมาะสำหรับ kitchen timer
 */
void example_countdown_mmss(void) {
    printf("=== Kitchen Timer (MM:SS) ===\r\n");
    
    char time_str[32];
    
    // ตั้งเวลา 3 นาที 30 วินาที
    Countdown_Init(0, 3, 30);
    Countdown_SetAlarmCallback(countdown_alarm);
    Countdown_Start();
    
    printf("Timer set: 3:30\r\n\r\n");
    
    while(1) {
        if (!Countdown_IsFinished()) {
            Countdown_GetTimeString(time_str, TIME_FORMAT_MMSS, TIME_DISPLAY_NORMALIZED);
            printf("\r%s", time_str);
            Delay_Ms(100);
        } else {
            break;
        }
    }
}

/* ========== Example 5: Stopwatch SS Format ========== */

/**
 * @brief Stopwatch แสดงผลเป็นวินาทีเท่านั้น
 * 
 * เหมาะสำหรับการวัดเวลาแบบรวดเร็ว
 */
void example_stopwatch_seconds(void) {
    printf("=== Stopwatch (Seconds Only) ===\r\n");
    
    char time_str[32];
    
    Stopwatch_Init();
    Stopwatch_Start();
    
    printf("Format: Total seconds\r\n\r\n");
    
    while(1) {
        // แสดงเป็นวินาทีทั้งหมด (RAW mode)
        Stopwatch_GetTimeString(time_str, TIME_FORMAT_SS, TIME_DISPLAY_RAW);
        printf("\rSeconds: %s", time_str);
        
        Delay_Ms(100);
    }
}

/* ========== Example 6: Countdown SS Format ========== */

/**
 * @brief Countdown แบบวินาที
 * 
 * เหมาะสำหรับ short intervals
 */
void example_countdown_seconds(void) {
    printf("=== Countdown (Seconds) ===\r\n");
    
    char time_str[32];
    
    // Countdown 30 วินาที
    Countdown_InitFromSeconds(30);
    Countdown_SetAlarmCallback(countdown_alarm);
    Countdown_Start();
    
    printf("Countdown: 30 seconds\r\n\r\n");
    
    while(1) {
        if (!Countdown_IsFinished()) {
            Countdown_GetTimeString(time_str, TIME_FORMAT_SS, TIME_DISPLAY_RAW);
            printf("\r%s seconds", time_str);
            Delay_Ms(100);
        } else {
            break;
        }
    }
}

/* ========== Example 7: RAW Display Mode Demo ========== */

/**
 * @brief สาธิตการแสดงผลแบบ RAW mode
 * 
 * แสดงค่าเวลาแบบไม่จำกัด (450 วินาที, 146 นาที)
 */
void example_raw_display_mode(void) {
    printf("=== RAW Display Mode Demo ===\r\n\r\n");
    
    char time_str[32];
    
    // ตั้งเวลา 450 วินาที (7.5 นาที)
    Countdown_InitFromSeconds(450);
    Countdown_Start();
    
    printf("Countdown: 450 seconds\r\n\r\n");
    
    while(1) {
        if (!Countdown_IsFinished()) {
            // แสดงแบบ RAW - วินาทีไม่จำกัด
            Countdown_GetTimeString(time_str, TIME_FORMAT_SS, TIME_DISPLAY_RAW);
            printf("\rRAW SS:     %s seconds   ", time_str);
            
            // แสดงแบบ NORMALIZED
            Countdown_GetTimeString(time_str, TIME_FORMAT_HHMMSS, TIME_DISPLAY_NORMALIZED);
            printf("| NORM: %s", time_str);
            
            Delay_Ms(1000);
        } else {
            printf("\r\nFinished!\r\n");
            break;
        }
    }
}

/* ========== Example 8: Lap Timer ========== */

#define MAX_LAPS 10

typedef struct {
    uint32_t lap_time_ms;
    char time_str[20];
} Lap_t;

/**
 * @brief Lap timer implementation
 * 
 * Stopwatch พร้อมบันทึก lap times
 */
void example_lap_timer(void) {
    printf("=== Lap Timer ===\r\n");
    
    Lap_t laps[MAX_LAPS];
    uint8_t lap_count = 0;
    uint32_t last_lap_ms = 0;
    char time_str[32];
    
    Stopwatch_Init();
    Stopwatch_Start();
    
    printf("Stopwatch running. Simulating lap button press every 5 seconds...\r\n\r\n");
    
    while(lap_count < MAX_LAPS) {
        // แสดงเวลาปัจจุบัน
        Stopwatch_GetTimeString(time_str, TIME_FORMAT_MMSS, TIME_DISPLAY_NORMALIZED);
        printf("\rCurrent: %s | Laps: %u", time_str, lap_count);
        
        // จำลองการกดปุ่ม lap ทุก 5 วินาที
        static uint32_t last_lap_time = 0;
        if (Get_CurrentMs() - last_lap_time >= 5000) {
            last_lap_time = Get_CurrentMs();
            
            // บันทึก lap
            uint32_t current_ms = Stopwatch_GetTotalMilliseconds();
            laps[lap_count].lap_time_ms = current_ms - last_lap_ms;
            
            Time_t lap_time;
            Time_FromSeconds(laps[lap_count].lap_time_ms / 1000, &lap_time, TIME_DISPLAY_NORMALIZED);
            sprintf(laps[lap_count].time_str, "%02u:%02u.%03lu", 
                    lap_time.minutes, lap_time.seconds, 
                    (laps[lap_count].lap_time_ms % 1000));
            
            printf("\r\nLap %u: %s\r\n", lap_count + 1, laps[lap_count].time_str);
            
            last_lap_ms = current_ms;
            lap_count++;
        }
        
        Delay_Ms(100);
    }
    
    // แสดงสรุป
    printf("\r\n\r\n=== Lap Summary ===\r\n");
    for (uint8_t i = 0; i < lap_count; i++) {
        printf("Lap %u: %s\r\n", i + 1, laps[i].time_str);
    }
}

/* ========== Example 9: Multi-Timer Management ========== */

/**
 * @brief ใช้ stopwatch และ countdown พร้อมกัน
 * 
 * สลับแสดงผลระหว่าง timers
 */
void example_multi_timer(void) {
    printf("=== Multi-Timer Management ===\r\n\r\n");
    
    char sw_str[32], cd_str[32];
    
    // เริ่ม stopwatch
    Stopwatch_Init();
    Stopwatch_Start();
    
    // เริ่ม countdown 2 นาที
    Countdown_Init(0, 2, 0);
    Countdown_Start();
    
    printf("Stopwatch: Running\r\n");
    printf("Countdown: 2:00\r\n\r\n");
    
    while(1) {
        // อ่านทั้งสอง timers
        Stopwatch_GetTimeString(sw_str, TIME_FORMAT_MMSS, TIME_DISPLAY_NORMALIZED);
        
        if (!Countdown_IsFinished()) {
            Countdown_GetTimeString(cd_str, TIME_FORMAT_MMSS, TIME_DISPLAY_NORMALIZED);
        } else {
            sprintf(cd_str, "FINISHED");
        }
        
        printf("\rStopwatch: %s | Countdown: %s   ", sw_str, cd_str);
        
        if (Countdown_IsFinished()) {
            printf("\r\nCountdown finished!\r\n");
            break;
        }
        
        Delay_Ms(100);
    }
}

/* ========== Example 10: Kitchen Timer Project ========== */

#define BUZZER_PIN  PC0
#define BTN_1MIN    PD1
#define BTN_5MIN    PD2
#define BTN_10MIN   PD3
#define LED_PIN     PC1

void kitchen_timer_alarm(void) {
    // เปิด buzzer
    digitalWrite(BUZZER_PIN, HIGH);
    Delay_Ms(2000);
    digitalWrite(BUZZER_PIN, LOW);
}

/**
 * @brief Kitchen Timer Project
 * 
 * Countdown timer พร้อม buzzer alarm
 * ปุ่มตั้งเวลา: 1, 5, 10 นาที
 */
void example_kitchen_timer(void) {
    printf("=== Kitchen Timer Project ===\r\n\r\n");
    
    // Setup pins
    pinMode(BUZZER_PIN, PIN_MODE_OUTPUT);
    pinMode(LED_PIN, PIN_MODE_OUTPUT);
    pinMode(BTN_1MIN, PIN_MODE_INPUT_PULLUP);
    pinMode(BTN_5MIN, PIN_MODE_INPUT_PULLUP);
    pinMode(BTN_10MIN, PIN_MODE_INPUT_PULLUP);
    
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    char time_str[32];
    uint8_t timer_active = 0;
    
    printf("Press button to set timer:\r\n");
    printf("  BTN1: 1 minute\r\n");
    printf("  BTN2: 5 minutes\r\n");
    printf("  BTN3: 10 minutes\r\n\r\n");
    
    while(1) {
        // ตรวจสอบปุ่ม
        if (!timer_active) {
            if (digitalRead(BTN_1MIN) == LOW) {
                Countdown_Init(0, 1, 0);
                Countdown_SetAlarmCallback(kitchen_timer_alarm);
                Countdown_Start();
                timer_active = 1;
                printf("Timer set: 1 minute\r\n");
                Delay_Ms(300);  // Debounce
            }
            else if (digitalRead(BTN_5MIN) == LOW) {
                Countdown_Init(0, 5, 0);
                Countdown_SetAlarmCallback(kitchen_timer_alarm);
                Countdown_Start();
                timer_active = 1;
                printf("Timer set: 5 minutes\r\n");
                Delay_Ms(300);
            }
            else if (digitalRead(BTN_10MIN) == LOW) {
                Countdown_Init(0, 10, 0);
                Countdown_SetAlarmCallback(kitchen_timer_alarm);
                Countdown_Start();
                timer_active = 1;
                printf("Timer set: 10 minutes\r\n");
                Delay_Ms(300);
            }
        }
        
        // แสดงผล countdown
        if (timer_active && !Countdown_IsFinished()) {
            Countdown_GetTimeString(time_str, TIME_FORMAT_MMSS, TIME_DISPLAY_NORMALIZED);
            printf("\r%s   ", time_str);
            
            // กระพริบ LED
            static uint32_t last_blink = 0;
            if (Get_CurrentMs() - last_blink >= 500) {
                last_blink = Get_CurrentMs();
                digitalToggle(LED_PIN);
            }
        }
        else if (timer_active && Countdown_IsFinished()) {
            printf("\r\nTimer finished!\r\n\r\n");
            timer_active = 0;
            digitalWrite(LED_PIN, LOW);
        }
        
        Delay_Ms(100);
    }
}

/* ========== Example 11: Workout Interval Timer ========== */

typedef struct {
    uint16_t work_seconds;
    uint16_t rest_seconds;
    uint8_t rounds;
} WorkoutConfig_t;

void workout_beep(void) {
    printf("\a");  // Beep
}

/**
 * @brief Workout Interval Timer
 * 
 * Countdown สลับ work/rest periods
 */
void example_workout_timer(void) {
    printf("=== Workout Interval Timer ===\r\n\r\n");
    
    WorkoutConfig_t config = {
        .work_seconds = 30,   // 30 วินาทีออกกำลัง
        .rest_seconds = 10,   // 10 วินาทีพัก
        .rounds = 5           // 5 รอบ
    };
    
    char time_str[32];
    
    printf("Workout: %u seconds\r\n", config.work_seconds);
    printf("Rest: %u seconds\r\n", config.rest_seconds);
    printf("Rounds: %u\r\n\r\n", config.rounds);
    
    for (uint8_t round = 1; round <= config.rounds; round++) {
        // Work period
        printf("\r\n=== Round %u/%u - WORK! ===\r\n", round, config.rounds);
        Countdown_InitFromSeconds(config.work_seconds);
        Countdown_SetAlarmCallback(workout_beep);
        Countdown_Start();
        
        while (!Countdown_IsFinished()) {
            Countdown_GetTimeString(time_str, TIME_FORMAT_SS, TIME_DISPLAY_RAW);
            printf("\rWork: %s   ", time_str);
            Delay_Ms(100);
        }
        
        // Rest period (ยกเว้นรอบสุดท้าย)
        if (round < config.rounds) {
            printf("\r\n=== Rest ===\r\n");
            Countdown_InitFromSeconds(config.rest_seconds);
            Countdown_SetAlarmCallback(workout_beep);
            Countdown_Start();
            
            while (!Countdown_IsFinished()) {
                Countdown_GetTimeString(time_str, TIME_FORMAT_SS, TIME_DISPLAY_RAW);
                printf("\rRest: %s   ", time_str);
                Delay_Ms(100);
            }
        }
    }
    
    printf("\r\n\r\n=== Workout Complete! ===\r\n");
}

/* ========== Main Function Template ========== */

#if 0  // เปลี่ยนเป็น 1 เพื่อ compile

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Timer_Init();
    
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#endif
    
    printf("\r\n");
    printf("====================================\r\n");
    printf("  SimpleTIM_Ext Examples\r\n");
    printf("====================================\r\n");
    printf("SystemClk: %lu Hz\r\n\r\n", SystemCoreClock);
    
    // เลือก example ที่ต้องการทดสอบ
    // example_basic_stopwatch();
    // example_basic_countdown();
    // example_stopwatch_mmss();
    // example_countdown_mmss();
    // example_stopwatch_seconds();
    // example_countdown_seconds();
    // example_raw_display_mode();
    // example_lap_timer();
    // example_multi_timer();
    // example_kitchen_timer();
    example_workout_timer();
    
    return 0;
}

#endif
