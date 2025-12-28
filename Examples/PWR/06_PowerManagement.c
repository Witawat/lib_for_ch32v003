/********************************** SimplePWR Example *******************************
 * Example Name       : 06_PowerManagement.c
 * Description        : Complete power management system with state machine
 *                      Automatically adjusts power mode based on activity
 **********************************************************************************/

#include "ch32v00x.h"
#include "SimpleHAL.h"

/*
 * Hardware Setup:
 * - LED connected to PD6 (status indicator)
 * - Button connected to PD2 (activity trigger)
 * - Optional: USART for debug output
 * 
 * Expected Behavior:
 * - System starts in IDLE state
 * - Button press triggers ACTIVE state
 * - After 5 seconds of inactivity, enters SLEEP state
 * - After 30 seconds of inactivity, enters STANDBY state
 * - LED indicates current power state
 */

#define LED_PIN         PD6
#define BUTTON_PIN      PD2

// Power states
typedef enum {
    STATE_ACTIVE,       // Full power, all peripherals active
    STATE_IDLE,         // Reduced power, waiting for activity
    STATE_SLEEP,        // Sleep mode, quick wake-up
    STATE_STANDBY       // Deep sleep, minimal power
} PowerState_t;

// Timeouts (in seconds)
#define IDLE_TO_SLEEP_TIMEOUT       5
#define SLEEP_TO_STANDBY_TIMEOUT    30

// Global variables
volatile uint32_t last_activity_time = 0;
PowerState_t current_state = STATE_IDLE;

// Function prototypes
void EnterActiveState(void);
void EnterIdleState(void);
void EnterSleepState(void);
void EnterStandbyState(void);
void UpdatePowerState(void);
void OnButtonPress(void);

void main(void)
{
    // Initialize system
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    
    // Configure LED pin
    pinMode(LED_PIN, OUTPUT);
    
    // Configure button pin with interrupt
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, OnButtonPress, FALLING);
    
    // Initialize activity timer
    last_activity_time = millis();
    
    // Startup indication
    for (uint8_t i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        Delay_Ms(100);
        digitalWrite(LED_PIN, LOW);
        Delay_Ms(100);
    }
    
    // Main loop
    while(1)
    {
        // Update power state based on activity
        UpdatePowerState();
        
        // Execute state-specific tasks
        switch(current_state)
        {
            case STATE_ACTIVE:
                // Active mode - do work
                digitalWrite(LED_PIN, HIGH);
                Delay_Ms(100);
                break;
                
            case STATE_IDLE:
                // Idle mode - slow blink
                digitalWrite(LED_PIN, HIGH);
                Delay_Ms(50);
                digitalWrite(LED_PIN, LOW);
                Delay_Ms(950);
                break;
                
            case STATE_SLEEP:
                // Sleep mode - very slow blink
                digitalWrite(LED_PIN, HIGH);
                Delay_Ms(50);
                digitalWrite(LED_PIN, LOW);
                PWR_Sleep();  // Sleep until next timer interrupt
                Delay_Ms(2000);
                break;
                
            case STATE_STANDBY:
                // Standby mode - enter deep sleep
                digitalWrite(LED_PIN, LOW);
                EnterStandbyState();
                break;
        }
    }
}

void EnterActiveState(void)
{
    if (current_state != STATE_ACTIVE) {
        current_state = STATE_ACTIVE;
        // Enable all peripherals as needed
        // e.g., USART, ADC, SPI, etc.
    }
}

void EnterIdleState(void)
{
    if (current_state != STATE_IDLE) {
        current_state = STATE_IDLE;
        // Disable non-essential peripherals
    }
}

void EnterSleepState(void)
{
    if (current_state != STATE_SLEEP) {
        current_state = STATE_SLEEP;
        // Disable more peripherals
        // Keep only timer for wake-up
    }
}

void EnterStandbyState(void)
{
    // Disable all peripherals
    // Save critical data if needed
    
    // Configure wake-up source (button on PD2)
    PWR_EnableWakeupPin();
    
    // Enter standby mode
    // System will reset on wake-up
    PWR_StandbyUntilInterrupt();
}

void UpdatePowerState(void)
{
    uint32_t idle_time = (millis() - last_activity_time) / 1000;  // Convert to seconds
    
    if (idle_time < IDLE_TO_SLEEP_TIMEOUT) {
        EnterIdleState();
    } else if (idle_time < SLEEP_TO_STANDBY_TIMEOUT) {
        EnterSleepState();
    } else {
        EnterStandbyState();
    }
}

void OnButtonPress(void)
{
    // Record activity
    last_activity_time = millis();
    
    // Return to active state
    EnterActiveState();
    
    // Debounce delay
    Delay_Ms(50);
}

/*
 * Power State Transition Diagram:
 * 
 *     ACTIVE ──> IDLE ──> SLEEP ──> STANDBY
 *        ^         ^        ^          |
 *        └─────────┴────────┴──────────┘
 *              (Button Press)
 * 
 * 
 * Power Consumption by State:
 * 
 * State     | Current | Peripherals Active
 * ----------|---------|----------------------------------
 * ACTIVE    | ~5 mA   | All peripherals enabled
 * IDLE      | ~3 mA   | Reduced clock, some peripherals off
 * SLEEP     | ~1 mA   | CPU stopped, timers running
 * STANDBY   | ~5 uA   | Everything off, wake-up only
 * 
 * 
 * State Transition Conditions:
 * 
 * ACTIVE → IDLE:
 * - No activity for 1 second
 * 
 * IDLE → SLEEP:
 * - No activity for 5 seconds
 * 
 * SLEEP → STANDBY:
 * - No activity for 30 seconds
 * 
 * ANY → ACTIVE:
 * - Button press
 * - External interrupt
 * - Communication received
 * 
 * 
 * Application Examples:
 * 
 * 1. Sensor Node:
 *    - ACTIVE: Reading sensors, transmitting data
 *    - IDLE: Waiting for next reading
 *    - SLEEP: Between reading intervals
 *    - STANDBY: Long-term sleep (wake every hour)
 * 
 * 2. Remote Control:
 *    - ACTIVE: Button pressed, sending command
 *    - IDLE: Waiting for button press
 *    - SLEEP: After 5 seconds of inactivity
 *    - STANDBY: After 30 seconds (wake on button)
 * 
 * 3. Data Logger:
 *    - ACTIVE: Writing to flash
 *    - IDLE: Buffering data
 *    - SLEEP: Waiting for next sample
 *    - STANDBY: Night mode (wake at dawn)
 * 
 * 
 * Advanced Features to Add:
 * 
 * 1. Battery Monitoring:
 *    - Use PVD to detect low battery
 *    - Adjust power states based on battery level
 *    - Enter standby when battery is critical
 * 
 * 2. Dynamic Timeout Adjustment:
 *    - Shorter timeouts when battery is low
 *    - Longer timeouts when battery is full
 * 
 * 3. Wake-up Source Tracking:
 *    - Determine what caused wake-up
 *    - Different actions for different sources
 * 
 * 4. Power Budget Management:
 *    - Track energy consumption
 *    - Predict remaining battery life
 *    - Warn user when battery is low
 */
