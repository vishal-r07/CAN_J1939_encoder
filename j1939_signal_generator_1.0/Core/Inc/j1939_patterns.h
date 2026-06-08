/*
 * j1939_patterns.h
 *
 *  Created on: Dec 8, 2025
 *      Author: abian
 */
#ifndef J1939_PATTERNS_H
#define J1939_PATTERNS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Pattern types
typedef enum {
    PATTERN_RAMP,           // Linear increase/decrease
    PATTERN_SINE,           // Sine wave oscillation
    PATTERN_STEP,           // Sudden changes
    PATTERN_RANDOM_WALK,    // Random movement within bounds
    PATTERN_CORRELATED,     // Follows another signal
    PATTERN_STATE_MACHINE,  // Discrete states
    PATTERN_CONSTANT        // Constant value
} Pattern_Type_t;

// Pattern configuration
typedef struct {
    Pattern_Type_t type;
    float min_value;
    float max_value;
    float period_seconds;   // For sine/ramp patterns
    float step_value;       // For step patterns
    uint32_t step_duration_ms; // For step patterns
    uint8_t num_states;     // For state machines
    float* state_values;    // For state machines
    float correlation_factor; // For correlated patterns
    uint32_t correlation_with_spn; // SPN to correlate with
} Pattern_Config_t;

// Signal generator
typedef struct {
    uint16_t spn;
    Pattern_Config_t config;
    float current_value;
    uint32_t last_update_time;
    uint32_t pattern_counter;
    bool enabled;
} Signal_Generator_t;

// Duration control
typedef struct {
    uint32_t total_duration_ms;  // Total test duration
    uint32_t elapsed_time_ms;    // Elapsed time
    bool is_running;
    uint32_t frames_generated;
} Duration_Control_t;

// Function prototypes
void Pattern_Init(void);
void Pattern_SetDuration(uint32_t duration_seconds);
void Pattern_Start(void);
void Pattern_Stop(void);
void Pattern_Update(uint32_t current_time_ms);
float Pattern_GetValue(uint16_t spn);
void Pattern_Configure(uint16_t spn, Pattern_Type_t type, float min_val, float max_val, float period_sec);
void Pattern_ConfigureStateMachine(uint16_t spn, uint8_t num_states, float* state_values, uint32_t state_duration_ms);
bool Pattern_IsRunning(void);
uint32_t Pattern_GetRemainingTimeMs(void);
uint32_t Pattern_GetFramesGenerated(void);

#ifdef __cplusplus
}
#endif

#endif // J1939_PATTERNS_H
