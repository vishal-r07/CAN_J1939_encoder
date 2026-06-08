#include "j1939_patterns.h"
#include "j1939_pgn_spn.h"
#include <stdlib.h>
#include <math.h>

#define MAX_SIGNALS 100
#define PI 3.14159265358979323846f

static Signal_Generator_t signal_generators[MAX_SIGNALS];
static uint8_t num_signals = 0;
static Duration_Control_t duration_control = {0};
static uint32_t system_time_ms = 0;

// Initialize pattern system
void Pattern_Init(void) {
    num_signals = 0;
    duration_control.total_duration_ms = 300000; // Default 5 minutes
    duration_control.elapsed_time_ms = 0;
    duration_control.is_running = false;
    duration_control.frames_generated = 0;
}

// Set test duration in seconds
void Pattern_SetDuration(uint32_t duration_seconds) {
    duration_control.total_duration_ms = duration_seconds * 1000;
}

// Start pattern generation
void Pattern_Start(void) {
    duration_control.elapsed_time_ms = 0;
    duration_control.frames_generated = 0;
    duration_control.is_running = true;

    // Reset all generators
    for (uint8_t i = 0; i < num_signals; i++) {
        signal_generators[i].last_update_time = 0;
        signal_generators[i].pattern_counter = 0;
        signal_generators[i].current_value = signal_generators[i].config.min_value;
    }
}

// Stop pattern generation
void Pattern_Stop(void) {
    duration_control.is_running = false;
}

// Update all patterns
void Pattern_Update(uint32_t current_time_ms) {
    system_time_ms = current_time_ms;

    if (!duration_control.is_running) {
        return;
    }

    // Update elapsed time
    duration_control.elapsed_time_ms = current_time_ms;

    // Check if duration expired
    if (duration_control.elapsed_time_ms >= duration_control.total_duration_ms) {
        Pattern_Stop();
        return;
    }

    // Update each signal
    for (uint8_t i = 0; i < num_signals; i++) {
        Signal_Generator_t* gen = &signal_generators[i];

        if (!gen->enabled) {
            continue;
        }

        float new_value = gen->current_value;

        switch (gen->config.type) {
            case PATTERN_RAMP: {
                // Linear ramp from min to max and back
                float phase = fmodf(current_time_ms / 1000.0f, gen->config.period_seconds * 2);
                if (phase < gen->config.period_seconds) {
                    // Ramp up
                    new_value = gen->config.min_value +
                               (gen->config.max_value - gen->config.min_value) *
                               (phase / gen->config.period_seconds);
                } else {
                    // Ramp down
                    phase = phase - gen->config.period_seconds;
                    new_value = gen->config.max_value -
                               (gen->config.max_value - gen->config.min_value) *
                               (phase / gen->config.period_seconds);
                }
                break;
            }

            case PATTERN_SINE: {
                // Sine wave oscillation
                float phase = 2.0f * PI * fmodf(current_time_ms / 1000.0f, gen->config.period_seconds) / gen->config.period_seconds;
                float amplitude = (gen->config.max_value - gen->config.min_value) / 2.0f;
                float center = (gen->config.max_value + gen->config.min_value) / 2.0f;
                new_value = center + amplitude * sinf(phase);
                break;
            }

            case PATTERN_STEP: {
                // Step changes at regular intervals
                uint32_t step_count = current_time_ms / gen->config.step_duration_ms;
                if (step_count > gen->pattern_counter) {
                    gen->pattern_counter = step_count;
                    new_value = gen->current_value + gen->config.step_value;

                    // Wrap around to min if exceeds max
                    if (new_value > gen->config.max_value) {
                        new_value = gen->config.min_value;
                    } else if (new_value < gen->config.min_value) {
                        new_value = gen->config.min_value;
                    }
                }
                break;
            }

            case PATTERN_RANDOM_WALK: {
                // Random walk within bounds
                if (current_time_ms - gen->last_update_time > 100) { // Update every 100ms
                    float change = ((rand() % 100) / 100.0f - 0.5f) * (gen->config.max_value - gen->config.min_value) * 0.1f;
                    new_value = gen->current_value + change;

                    // Clamp to bounds
                    if (new_value > gen->config.max_value) new_value = gen->config.max_value;
                    if (new_value < gen->config.min_value) new_value = gen->config.min_value;

                    gen->last_update_time = current_time_ms;
                }
                break;
            }

            case PATTERN_STATE_MACHINE: {
                // Cycle through discrete states
                if (gen->config.num_states > 0) {
                    uint32_t state_index = (current_time_ms / gen->config.step_duration_ms) % gen->config.num_states;
                    new_value = gen->config.state_values[state_index];
                }
                break;
            }

            case PATTERN_CONSTANT:
            default:
                // Keep current value
                break;
        }

        gen->current_value = new_value;
    }

    duration_control.frames_generated++;
}

// Get current value for SPN
float Pattern_GetValue(uint16_t spn) {
    for (uint8_t i = 0; i < num_signals; i++) {
        if (signal_generators[i].spn == spn && signal_generators[i].enabled) {
            return signal_generators[i].current_value;
        }
    }
    return 0.0f;
}

// Configure a pattern
void Pattern_Configure(uint16_t spn, Pattern_Type_t type, float min_val, float max_val, float period_sec) {
    // Check if already exists
    for (uint8_t i = 0; i < num_signals; i++) {
        if (signal_generators[i].spn == spn) {
            signal_generators[i].config.type = type;
            signal_generators[i].config.min_value = min_val;
            signal_generators[i].config.max_value = max_val;
            signal_generators[i].config.period_seconds = period_sec;
            signal_generators[i].enabled = true;
            return;
        }
    }

    // Add new signal
    if (num_signals < MAX_SIGNALS) {
        signal_generators[num_signals].spn = spn;
        signal_generators[num_signals].config.type = type;
        signal_generators[num_signals].config.min_value = min_val;
        signal_generators[num_signals].config.max_value = max_val;
        signal_generators[num_signals].config.period_seconds = period_sec;
        signal_generators[num_signals].current_value = min_val;
        signal_generators[num_signals].last_update_time = 0;
        signal_generators[num_signals].pattern_counter = 0;
        signal_generators[num_signals].enabled = true;
        num_signals++;
    }
}

// Configure state machine pattern
void Pattern_ConfigureStateMachine(uint16_t spn, uint8_t num_states, float* state_values, uint32_t state_duration_ms) {
    for (uint8_t i = 0; i < num_signals; i++) {
        if (signal_generators[i].spn == spn) {
            signal_generators[i].config.type = PATTERN_STATE_MACHINE;
            signal_generators[i].config.num_states = num_states;
            signal_generators[i].config.state_values = state_values;
            signal_generators[i].config.step_duration_ms = state_duration_ms;
            signal_generators[i].enabled = true;
            return;
        }
    }

    if (num_signals < MAX_SIGNALS) {
        signal_generators[num_signals].spn = spn;
        signal_generators[num_signals].config.type = PATTERN_STATE_MACHINE;
        signal_generators[num_signals].config.num_states = num_states;
        signal_generators[num_signals].config.state_values = state_values;
        signal_generators[num_signals].config.step_duration_ms = state_duration_ms;
        signal_generators[num_signals].current_value = state_values[0];
        signal_generators[num_signals].last_update_time = 0;
        signal_generators[num_signals].pattern_counter = 0;
        signal_generators[num_signals].enabled = true;
        num_signals++;
    }
}

bool Pattern_IsRunning(void) {
    return duration_control.is_running;
}

uint32_t Pattern_GetRemainingTimeMs(void) {
    if (duration_control.elapsed_time_ms >= duration_control.total_duration_ms) {
        return 0;
    }
    return duration_control.total_duration_ms - duration_control.elapsed_time_ms;
}

uint32_t Pattern_GetFramesGenerated(void) {
    return duration_control.frames_generated;
}
