/*
 * =============================================================================
 * FILE : j1939_pattern_generator.h
 * WHAT : Signal pattern generator for simulating realistic vehicle data
 * =============================================================================
 */

#ifndef J1939_PATTERN_GENERATOR_H
#define J1939_PATTERN_GENERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Maximum number of simultaneously registered signals */
#define PATTERN_GEN_MAX_SIGNALS  60u

/* =============================================================================
 * PATTERN TYPE ENUM
 * ============================================================================= */
typedef enum {
    PATTERN_CONSTANT        = 0,  /* Never changes */
    PATTERN_RAMP            = 1,  /* Linear up/down triangle wave */
    PATTERN_SINE            = 2,  /* Smooth sinusoidal oscillation */
    PATTERN_STEP            = 3,  /* Discrete jumps at intervals */
    PATTERN_RANDOM_WALK     = 4,  /* Small random variations (noise) */
    PATTERN_STATE_SEQUENCE  = 5,  /* Cycles through fixed values */
} Pattern_Type_t;

/* =============================================================================
 * PATTERN CONFIGURATION STRUCT
 * ============================================================================= */
typedef struct {
    uint32_t        spn;                    /* SPN number this pattern drives */
    Pattern_Type_t  pattern_type;           /* Type of pattern */
    
    /* Range limits (apply to all patterns) */
    float   min_value;
    float   max_value;
    float   initial_value;
    
    /* PATTERN_RAMP */
    float   ramp_period_seconds;            /* Time to sweep min->max */
    
    /* PATTERN_SINE */
    float   sine_period_seconds;            /* Full oscillation period */
    
    /* PATTERN_STEP */
    float    step_size;                     /* Increment per step */
    uint32_t step_interval_ms;              /* Milliseconds between steps */
    
    /* PATTERN_RANDOM_WALK */
    float   random_max_step;                /* Max delta per update */
    
    /* PATTERN_STATE_SEQUENCE */
    const float* state_values;              /* Array of discrete values */
    uint8_t      num_states;                /* Length of array */
    uint32_t     state_hold_ms;             /* Time per state */
    
    /* Timing */
    uint32_t update_interval_ms;            /* How often to recalculate */
    
} Pattern_Config_t;

/* =============================================================================
 * PUBLIC API
 * ============================================================================= */

/* Initialize the generator system */
void Pattern_Generator_Init(uint32_t sim_start_time_ms);

/* Register a new pattern */
bool Pattern_Generator_Register(const Pattern_Config_t* config);

/* Update all registered patterns (call every 1ms from main loop) */
void Pattern_Generator_Update(uint32_t current_time_ms);

/* Get current value for an SPN */
float Pattern_Generator_Get_Value(uint32_t spn, float default_value);

/* Reset all patterns to initial values */
void Pattern_Generator_Reset(uint32_t current_time_ms);

/* Get number of registered patterns */
uint8_t Pattern_Generator_Get_Count(void);

/* Check if any pattern is currently active */
bool Pattern_Generator_Is_Running(void);

#ifdef __cplusplus
}
#endif
#endif /* J1939_PATTERN_GENERATOR_H */