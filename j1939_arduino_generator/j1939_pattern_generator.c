/*
 * =============================================================================
 * FILE : j1939_pattern_generator.c
 * WHAT : Implementation of signal pattern generator
 * =============================================================================
 */

#include "j1939_pattern_generator.h"
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* =============================================================================
 * INTERNAL RUNTIME STATE
 * ============================================================================= */
typedef struct {
    Pattern_Config_t config;
    float   current_value;
    uint32_t last_update_ms;
    float   ramp_direction;
    uint32_t step_state_index;
} Pattern_State_t;

static Pattern_State_t  g_patterns[PATTERN_GEN_MAX_SIGNALS];
static uint8_t          g_pattern_count = 0u;
static uint32_t         g_sim_start_ms = 0u;
static uint32_t         g_prng_state = 0xDEADBEEFu;
static bool             g_is_running = true;

/* =============================================================================
 * PRIVATE: Pseudo-random number generator (-1.0 to +1.0)
 * ============================================================================= */
static float prv_random_symmetric(void)
{
    g_prng_state = (g_prng_state * 1664525u) + 1013904223u;
    int32_t signed_val = (int32_t)g_prng_state;
    return (float)signed_val / 2147483648.0f;
}

/* =============================================================================
 * PRIVATE: Clamp value to range
 * ============================================================================= */
static float prv_clamp(float value, float lo, float hi)
{
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

/* =============================================================================
 * PRIVATE: Compute next value for a pattern
 * ============================================================================= */
static float prv_compute_next_value(Pattern_State_t* state, uint32_t current_time_ms)
{
    const Pattern_Config_t* cfg = &state->config;
    uint32_t elapsed_ms = current_time_ms - g_sim_start_ms;
    float elapsed_sec = (float)elapsed_ms / 1000.0f;
    
    switch (cfg->pattern_type) {
        
        case PATTERN_CONSTANT:
            return state->current_value;
            
        case PATTERN_RAMP: {
            float range = cfg->max_value - cfg->min_value;
            float period_ms = (cfg->ramp_period_seconds > 0.0f) 
                              ? cfg->ramp_period_seconds * 1000.0f 
                              : 10000.0f;
            float step = range * (float)cfg->update_interval_ms / period_ms;
            float new_value = state->current_value + state->ramp_direction * step;
            
            if (new_value >= cfg->max_value) {
                new_value = cfg->max_value;
                state->ramp_direction = -1.0f;
            } else if (new_value <= cfg->min_value) {
                new_value = cfg->min_value;
                state->ramp_direction = +1.0f;
            }
            return new_value;
        }
        
        case PATTERN_SINE: {
            float center = (cfg->max_value + cfg->min_value) * 0.5f;
            float amplitude = (cfg->max_value - cfg->min_value) * 0.5f;
            float period = (cfg->sine_period_seconds > 0.0f) ? cfg->sine_period_seconds : 1.0f;
            float phase = 2.0f * M_PI * elapsed_sec / period;
            float new_value = center + amplitude * sinf(phase);
            return prv_clamp(new_value, cfg->min_value, cfg->max_value);
        }
        
        case PATTERN_STEP: {
            uint32_t current_step = elapsed_ms / cfg->step_interval_ms;
            if (current_step > state->step_state_index) {
                state->step_state_index = current_step;
                float new_value = state->current_value + cfg->step_size;
                if (new_value > cfg->max_value) new_value = cfg->min_value;
                if (new_value < cfg->min_value) new_value = cfg->min_value;
                return new_value;
            }
            return state->current_value;
        }
        
        case PATTERN_RANDOM_WALK: {
            float delta = prv_random_symmetric() * cfg->random_max_step;
            float new_value = state->current_value + delta;
            return prv_clamp(new_value, cfg->min_value, cfg->max_value);
        }
        
        case PATTERN_STATE_SEQUENCE: {
            if ((cfg->state_values == NULL) || (cfg->num_states == 0u)) {
                return state->current_value;
            }
            uint32_t hold = (cfg->state_hold_ms > 0u) ? cfg->state_hold_ms : 1000u;
            uint32_t idx = (elapsed_ms / hold) % (uint32_t)cfg->num_states;
            state->step_state_index = idx;
            return cfg->state_values[idx];
        }
        
        default:
            return state->current_value;
    }
}

/* =============================================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================================= */

void Pattern_Generator_Init(uint32_t sim_start_time_ms)
{
    memset(g_patterns, 0, sizeof(g_patterns));
    g_pattern_count = 0u;
    g_sim_start_ms = sim_start_time_ms;
    g_prng_state = 0xDEADBEEFu;
    g_is_running = true;
}

bool Pattern_Generator_Register(const Pattern_Config_t* config)
{
    if (config == NULL) return false;
    if (g_pattern_count >= PATTERN_GEN_MAX_SIGNALS) return false;
    
    Pattern_State_t* state = &g_patterns[g_pattern_count];
    state->config = *config;
    state->current_value = config->initial_value;
    state->last_update_ms = g_sim_start_ms;
    state->ramp_direction = +1.0f;
    state->step_state_index = 0u;
    
    g_pattern_count++;
    return true;
}

void Pattern_Generator_Update(uint32_t current_time_ms)
{
    if (!g_is_running) return;
    
    for (uint8_t i = 0u; i < g_pattern_count; i++) {
        Pattern_State_t* state = &g_patterns[i];
        uint32_t elapsed = current_time_ms - state->last_update_ms;
        
        if (elapsed >= state->config.update_interval_ms) {
            state->current_value = prv_compute_next_value(state, current_time_ms);
            state->last_update_ms = current_time_ms;
        }
    }
}

float Pattern_Generator_Get_Value(uint32_t spn, float default_value)
{
    for (uint8_t i = 0u; i < g_pattern_count; i++) {
        if (g_patterns[i].config.spn == spn) {
            return g_patterns[i].current_value;
        }
    }
    return default_value;
}

void Pattern_Generator_Reset(uint32_t current_time_ms)
{
    g_sim_start_ms = current_time_ms;
    g_is_running = true;
    
    for (uint8_t i = 0u; i < g_pattern_count; i++) {
        Pattern_State_t* state = &g_patterns[i];
        state->current_value = state->config.initial_value;
        state->last_update_ms = current_time_ms;
        state->ramp_direction = +1.0f;
        state->step_state_index = 0u;
    }
}

uint8_t Pattern_Generator_Get_Count(void)
{
    return g_pattern_count;
}

bool Pattern_Generator_Is_Running(void)
{
    return g_is_running;
}