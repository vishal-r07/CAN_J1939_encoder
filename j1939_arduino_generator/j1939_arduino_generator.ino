#include "j1939_can_helper.h"
#include "j1939_encode_decode.h"
#include "j1939_signal_definitions.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* =============================================================================
   J1939 MULTI-SPN TEST CONFIGURATION
   ============================================================================= */
#define PGN_EEC1                 61444   // Electronic Engine Controller 1
#define PGN_EEC2                 61443   // Electronic Engine Controller 2
#define PGN_CCVS1                65265   // Cruise Control/Vehicle Speed 1
#define PGN_LFE                  65266   // Fuel Economy Liquid

#define SPN_ENGINE_SPEED         190     // Engine Speed (rpm)
#define SPN_ACCEL_PEDAL_POS      91      // Accelerator Pedal Position 1 (%)
#define SPN_VEHICLE_SPEED        84      // Wheel-Based Vehicle Speed (km/h)
#define SPN_ENGINE_FUEL_RATE     183     // Engine Fuel Rate (L/h)
#define SPN_ENGINE_INST_FUEL_ECON 184    // Engine Instantaneous Fuel Economy (km/L)

#define EEC1_PRIORITY            3
#define EEC2_PRIORITY            3
#define CCVS1_PRIORITY           6
#define LFE_PRIORITY             6

#define SOURCE_ADDRESS           0x00
#define TEST_PERIOD_MS           3       // High-rate stress testing: 3ms broadcast rate for all test messages

// Unique boundary markers for each signal to identify transmission bounds
#define MARKER_SPN190            0.0f 
#define MARKER_SPN91             0.0f
#define MARKER_SPN84             0.0f
#define MARKER_SPN183            0.0f
#define MARKER_SPN184            0.0f

#define MARKER_DURATION_MS       3000    // Transmit marker for 3 seconds
#define TOTAL_DATA_POINTS        5000    // Large dataset for stress testing (5000 frames per pattern)

/* =============================================================================
   DYNAMIC WAVEFORM FUNCTIONS
   ============================================================================= */

// SPN 190: Engine Speed -> Ramp function (starts/ends at 0.0, max 3000.0)
float Get_SPN190_Value(uint32_t index) {
    uint32_t half = TOTAL_DATA_POINTS / 2;
    if (index < half) {
        return 3000.0f * (float)index / (float)half;
    } else {
        return 3000.0f - 3000.0f * (float)(index - half) / (float)half;
    }
}

// SPN 91: Accelerator Pedal Position 1 -> Step function (starts/ends at 0.0, max 100.0)
float Get_SPN91_Value(uint32_t index) {
    uint32_t samples_per_step = TOTAL_DATA_POINTS / 20;
    if (samples_per_step == 0) samples_per_step = 1;
    uint32_t step_idx = index / samples_per_step;
    if (step_idx < 10) {
        return (float)step_idx * 10.0f;
    } else if (step_idx < 20) {
        return 100.0f - (float)(step_idx - 10) * 10.0f;
    } else {
        return 0.0f;
    }
}

// SPN 84: Wheel-Based Vehicle Speed -> Complete sine wave with ramp-in/out (starts/ends at 0.0, center 50.0, range 20.0 to 80.0)
float Get_SPN84_Value(uint32_t index) {
    if (index < 500) {
        return 50.0f * (float)index / 500.0f;
    } else if (index < 4500) {
        return 50.0f + 30.0f * sinf(2.0f * M_PI * (float)(index - 500) / 4000.0f);
    } else {
        return 50.0f - 50.0f * (float)(index - 4500) / 500.0f;
    }
}

// SPN 183: Engine Fuel Rate -> State sequence (starts/ends at 0.0, max 380.0)
float Get_SPN183_Value(uint32_t index) {
    uint32_t samples_per_interval = TOTAL_DATA_POINTS / 5;
    if (samples_per_interval == 0) samples_per_interval = 1;
    uint32_t interval = index / samples_per_interval;
    if (interval == 0) return 0.0f;
    if (interval == 1) return 120.0f;
    if (interval == 2) return 250.0f;
    if (interval == 3) return 380.0f;
    return 0.0f;
}

// SPN 184: Engine Instantaneous Fuel Economy -> Constant pattern
float Get_SPN184_Value(uint32_t index) {
    return 15.0f;
}

enum TestState {
    STATE_START_MARKER,
    STATE_EXCEL_PATTERN,
    STATE_END_MARKER,
    STATE_DONE
};

TestState current_state = STATE_START_MARKER;
uint32_t state_start_time = 0;
uint32_t last_tx_ms = 0;

uint32_t current_excel_index = 0;
bool led_state = false;

/* =============================================================================
   SETUP
   ============================================================================= */
void setup() {
    Serial.begin(115200);
    delay(2000); 

    Serial.println("\n=========================================");
    Serial.println("  STM32 J1939 MULTI-SIGNAL GENERATOR    ");
    Serial.print("  Target Data Points: ");
    Serial.println(TOTAL_DATA_POINTS);
    Serial.println("=========================================");

    // Initialize CAN Helper Library
    if (J1939_CAN_Hardware_Init()) {
        Serial.println("[SUCCESS] CAN Helper Library Initialized.");
    } else {
        Serial.println("[ERROR] CAN Helper Library Failed.");
        while (1); 
    }

    Serial.println("[SUCCESS] CAN lines routed to PB8 (RX) and PB9 (TX).");

    pinMode(LED_BUILTIN, OUTPUT);

    state_start_time = millis();
    Serial.println("\n[TEST] Transmitting START MARKERS (3 seconds)...");
}

/* =============================================================================
   LOOP
   ============================================================================= */
void loop() {
    uint32_t current_time = millis();
    
    // CAN Transmission Scheduler (run at 3ms intervals for stress testing)
    if (current_time - last_tx_ms >= TEST_PERIOD_MS) {
        float val_spn190 = 0.0f;
        float val_spn91 = 0.0f;
        float val_spn84 = 0.0f;
        float val_spn183 = 0.0f;
        float val_spn184 = 0.0f;
        bool send_frame = true;

        switch (current_state) {
            case STATE_START_MARKER:
                val_spn190 = MARKER_SPN190;
                val_spn91  = MARKER_SPN91;
                val_spn84  = MARKER_SPN84;
                val_spn183 = MARKER_SPN183;
                val_spn184 = MARKER_SPN184;

                if (current_time - state_start_time >= MARKER_DURATION_MS) {
                    current_state = STATE_EXCEL_PATTERN;
                    state_start_time = current_time;
                    current_excel_index = 0;
                    Serial.println("\n[TEST] Transmitting WAVEFORMS...");
                }
                break;

            case STATE_EXCEL_PATTERN:
                if (current_excel_index < TOTAL_DATA_POINTS) {
                    val_spn190 = Get_SPN190_Value(current_excel_index);
                    val_spn91  = Get_SPN91_Value(current_excel_index);
                    val_spn84  = Get_SPN84_Value(current_excel_index);
                    val_spn183 = Get_SPN183_Value(current_excel_index);
                    val_spn184 = Get_SPN184_Value(current_excel_index);
                    current_excel_index++;
                }
                
                if (current_excel_index >= TOTAL_DATA_POINTS) {
                    current_state = STATE_END_MARKER;
                    state_start_time = current_time;
                    Serial.println("\n[TEST] Waveform patterns finished. Transmitting END MARKERS...");
                }
                break;

            case STATE_END_MARKER:
                val_spn190 = MARKER_SPN190;
                val_spn91  = MARKER_SPN91;
                val_spn84  = MARKER_SPN84;
                val_spn183 = MARKER_SPN183;
                val_spn184 = MARKER_SPN184;

                if (current_time - state_start_time >= MARKER_DURATION_MS) {
                    current_state = STATE_DONE;
                    Serial.println("\n[TEST] Sequence COMPLETE. Halting transmission.");
                }
                break;

            case STATE_DONE:
                send_frame = false;
                digitalWrite(LED_BUILTIN, HIGH); // LED solid on upon completion
                break;
        }

        // Encode and transmit standard J1939 messages back-to-back
        if (send_frame) {
            // 1. Electronic Engine Controller 1 (EEC1) -> SPN 190
            uint8_t payload_eec1[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            const J1939_Signal_Definition_t* def_190 = J1939_Find_Signal_By_SPN(SPN_ENGINE_SPEED);
            if (def_190 != NULL) {
                J1939_Encode_Signal(def_190, val_spn190, payload_eec1);
                J1939_CAN_Hardware_Transmit(PGN_EEC1, payload_eec1, 8, EEC1_PRIORITY, SOURCE_ADDRESS);
            }

            // 2. Electronic Engine Controller 2 (EEC2) -> SPN 91
            uint8_t payload_eec2[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            const J1939_Signal_Definition_t* def_91 = J1939_Find_Signal_By_SPN(SPN_ACCEL_PEDAL_POS);
            if (def_91 != NULL) {
                J1939_Encode_Signal(def_91, val_spn91, payload_eec2);
                J1939_CAN_Hardware_Transmit(PGN_EEC2, payload_eec2, 8, EEC2_PRIORITY, SOURCE_ADDRESS);
            }

            // 3. Cruise Control/Vehicle Speed 1 (CCVS1) -> SPN 84
            uint8_t payload_ccvs1[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            const J1939_Signal_Definition_t* def_84 = J1939_Find_Signal_By_SPN(SPN_VEHICLE_SPEED);
            if (def_84 != NULL) {
                J1939_Encode_Signal(def_84, val_spn84, payload_ccvs1);
                J1939_CAN_Hardware_Transmit(PGN_CCVS1, payload_ccvs1, 8, CCVS1_PRIORITY, SOURCE_ADDRESS);
            }

            // 4. Fuel Economy Liquid (LFE) -> SPN 183 & SPN 184
            uint8_t payload_lfe[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            const J1939_Signal_Definition_t* def_183 = J1939_Find_Signal_By_SPN(SPN_ENGINE_FUEL_RATE);
            const J1939_Signal_Definition_t* def_184 = J1939_Find_Signal_By_SPN(SPN_ENGINE_INST_FUEL_ECON);
            if (def_183 != NULL) {
                J1939_Encode_Signal(def_183, val_spn183, payload_lfe);
            }
            if (def_184 != NULL) {
                J1939_Encode_Signal(def_184, val_spn184, payload_lfe);
            }
            if (def_183 != NULL || def_184 != NULL) {
                J1939_CAN_Hardware_Transmit(PGN_LFE, payload_lfe, 8, LFE_PRIORITY, SOURCE_ADDRESS);
            }

            last_tx_ms = current_time;
            led_state = !led_state;
            digitalWrite(LED_BUILTIN, led_state); 
        }
    }
}