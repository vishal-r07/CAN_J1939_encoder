#include "j1939_can_helper.h"
#include "j1939_encode_decode.h"
#include "j1939_pattern_generator.h"
#include "j1939_signal_definitions.h"

/* =============================================================================
   J1939 PERIODIC MESSAGE SCHEDULER STRUCT
   ============================================================================= */
typedef struct {
    uint32_t pgn;
    uint32_t period_ms;
    uint32_t last_tx_ms;
    uint8_t  priority;
    uint8_t  source_address;
    uint32_t spns[8]; /* List of SPNs in this PGN, terminated by 0 */
} J1939_Periodic_Msg_t;

/* List of J1939 messages to transmit periodically */
static J1939_Periodic_Msg_t periodic_messages[] = {
    /* EEC1 (PGN 61444 / 0xF004) - 50 ms - Engine speed, torques */
    { PGN_EEC1, 50, 0, 3, 0x00, { SPN_ENGINE_SPEED, SPN_DRIVER_DEMAND_TORQUE, SPN_ACTUAL_ENGINE_TORQUE, SPN_ENGINE_DEMAND_TORQUE, SPN_ENGINE_TORQUE_MODE, 0 } },
    
    /* EEC2 (PGN 61443 / 0xF003) - 50 ms - Accelerator, Load */
    { PGN_EEC2, 50, 0, 3, 0x00, { SPN_ACCEL_PEDAL_POS, SPN_ENGINE_LOAD_PCT, 0 } },
    
    /* Engine Temperature 1 (PGN 65262 / 0xFEEE) - 1000 ms - Coolant & Oil Temp */
    { PGN_ENGINE_TEMP1, 1000, 0, 3, 0x00, { SPN_COOLANT_TEMP, SPN_ENGINE_OIL_TEMP, 0 } },
    
    /* Engine Fluids 1 (PGN 65263 / 0xFEEF) - 500 ms - Oil Level & Pressure */
    { PGN_ENGINE_FLUIDS1, 500, 0, 3, 0x00, { SPN_ENGINE_OIL_LEVEL, SPN_ENGINE_OIL_PRESSURE, 0 } },
    
    /* CCVS1 (PGN 65265 / 0xFEF1) - 100 ms - Vehicle Speed */
    { PGN_CCVS1, 100, 0, 3, 0x00, { SPN_VEHICLE_SPEED, 0 } },
    
    /* VDS (PGN 61449 / 0xF009) - 50 ms - Vehicle Dynamics (Steering, Yaw, Accel) */
    { PGN_VDS, 50, 0, 3, 0x00, { SPN_STEERING_ANGLE, SPN_YAW_RATE, SPN_LATERAL_ACCEL, SPN_LONG_ACCEL, 0 } },
    
    /* ETC2 (PGN 61445 / 0xF005) - 50 ms - Transmission Current Gear (Source Address 0x03) */
    { PGN_ETC2, 50, 0, 3, 0x03, { SPN_TRANS_GEAR, 0 } },
    
    /* Transmission Fluids 1 (PGN 65272 / 0xFEF8) - 1000 ms - Trans Temp & Pressure */
    { PGN_TRANS_FLUIDS1, 1000, 0, 3, 0x03, { SPN_TRANS_OIL_PRESSURE, SPN_TRANS_OIL_TEMP, 0 } },
    
    /* EBC1 (PGN 61441 / 0xF001) - 100 ms - Brake Pedal Position */
    { PGN_EBC1, 100, 0, 3, 0x00, { SPN_BRAKE_PEDAL_POS, 0 } },
    
    /* Brake Air Pressure (PGN 65274 / 0xFEFA) - 1000 ms - Brake Application Pressure */
    { PGN_BRAKE_AIR_PRESS, 1000, 0, 3, 0x00, { SPN_BRAKE_APP_PRESSURE, 0 } },
    
    /* AT1 SCR Dosing Rate (PGN 61475 / 0xF023) - 100 ms - SCR Dosing Rate */
    { PGN_AT1_SCR_DOSING, 100, 0, 3, 0x00, { SPN_SCR_DOSING_RATE, 0 } },
    
    /* AT1 SCR Tank Info (PGN 65110 / 0xFE56) - 1000 ms - Tank Level & Temp */
    { PGN_AT1_SCR_TANK, 1000, 0, 3, 0x00, { SPN_SCR_TANK_LEVEL, SPN_SCR_TANK_TEMP, 0 } },
    
    /* DPF Temperature 1 (PGN 64947 / 0xFDB3) - 1000 ms - DPF Inlet/Outlet Temp */
    { PGN_DPF_TEMP1, 1000, 0, 3, 0x00, { SPN_DPF_INLET_TEMP, SPN_DPF_OUTLET_TEMP, 0 } },
    
    /* Ambient Conditions (PGN 65269 / 0xFEF5) - 1000 ms - Baro, Cabin, Road, Ambient Temp */
    { PGN_AMB, 1000, 0, 3, 0x00, { SPN_BARO_PRESSURE, SPN_CAB_TEMP, SPN_AMBIENT_TEMP, SPN_AIR_INLET_TEMP, SPN_ROAD_SURFACE_TEMP, 0 } },
    
    /* Electrical (PGN 65271 / 0xFEF7) - 1000 ms - Alternator & Battery Volts */
    { PGN_VOLTS_AMPS, 1000, 0, 3, 0x00, { SPN_ALTERNATOR_CURRENT, SPN_BATTERY_VOLTAGE, 0 } },
    
    /* Fan Drive (PGN 65213 / 0xFEBD) - 1000 ms - Fan Speed & Drive % */
    { PGN_FAN_DRIVE, 1000, 0, 3, 0x00, { SPN_FAN_DRIVE_PCT, SPN_FAN_SPEED, 0 } },
    
    /* Dash Display (PGN 65276 / 0xFEFC) - 1000 ms - Washer Fluid & Fuel Level */
    { PGN_DASH, 1000, 0, 3, 0x00, { SPN_WASHER_FLUID_LEVEL, SPN_FUEL_LEVEL, 0 } }
};

static const uint8_t periodic_messages_count = sizeof(periodic_messages) / sizeof(J1939_Periodic_Msg_t);

/* Discrete states for Transmission Gear (SPN 523) */
static const float gear_sequence[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f };
static const uint8_t gear_sequence_length = sizeof(gear_sequence) / sizeof(float);

/* =============================================================================
   FUNCTION: Configure_Signal_Generator_Patterns
   ============================================================================= */
static void Configure_Signal_Generator_Patterns(void) {
    Pattern_Config_t cfg;

    // 1. Engine Speed (SPN 190) - Sine wave oscillation
    cfg = {};
    cfg.spn = SPN_ENGINE_SPEED;
    cfg.pattern_type = PATTERN_SINE;
    cfg.min_value = 600.0f;
    cfg.max_value = 2200.0f;
    cfg.initial_value = 800.0f;
    cfg.sine_period_seconds = 30.0f;
    cfg.update_interval_ms = 10;
    Pattern_Generator_Register(&cfg);

    // 2. Vehicle Speed (SPN 84) - Ramping up and down
    cfg = {};
    cfg.spn = SPN_VEHICLE_SPEED;
    cfg.pattern_type = PATTERN_RAMP;
    cfg.min_value = 0.0f;
    cfg.max_value = 120.0f;
    cfg.initial_value = 0.0f;
    cfg.ramp_period_seconds = 60.0f;
    cfg.update_interval_ms = 10;
    Pattern_Generator_Register(&cfg);

    // 3. Engine Coolant Temperature (SPN 110) - Random walk
    cfg = {};
    cfg.spn = SPN_COOLANT_TEMP;
    cfg.pattern_type = PATTERN_RANDOM_WALK;
    cfg.min_value = 75.0f;
    cfg.max_value = 105.0f;
    cfg.initial_value = 85.0f;
    cfg.random_max_step = 0.05f;
    cfg.update_interval_ms = 100;
    Pattern_Generator_Register(&cfg);

    // 4. Engine Oil Temperature (SPN 175) - Random walk (slightly higher than coolant)
    cfg = {};
    cfg.spn = SPN_ENGINE_OIL_TEMP;
    cfg.pattern_type = PATTERN_RANDOM_WALK;
    cfg.min_value = 80.0f;
    cfg.max_value = 115.0f;
    cfg.initial_value = 90.0f;
    cfg.random_max_step = 0.05f;
    cfg.update_interval_ms = 100;
    Pattern_Generator_Register(&cfg);

    // 5. Engine Oil Pressure (SPN 100) - Step jumps
    cfg = {};
    cfg.spn = SPN_ENGINE_OIL_PRESSURE;
    cfg.pattern_type = PATTERN_STEP;
    cfg.min_value = 200.0f;
    cfg.max_value = 500.0f;
    cfg.initial_value = 320.0f;
    cfg.step_size = 30.0f;
    cfg.step_interval_ms = 4000;
    cfg.update_interval_ms = 100;
    Pattern_Generator_Register(&cfg);

    // 6. Battery Voltage (SPN 168) - Smooth sine wave oscillation
    cfg = {};
    cfg.spn = SPN_BATTERY_VOLTAGE;
    cfg.pattern_type = PATTERN_SINE;
    cfg.min_value = 23.5f;
    cfg.max_value = 28.2f;
    cfg.initial_value = 26.5f;
    cfg.sine_period_seconds = 120.0f;
    cfg.update_interval_ms = 50;
    Pattern_Generator_Register(&cfg);

    // 7. Fuel Level (SPN 96) - Very slow ramp down simulating consumption
    cfg = {};
    cfg.spn = SPN_FUEL_LEVEL;
    cfg.pattern_type = PATTERN_RAMP;
    cfg.min_value = 15.0f;
    cfg.max_value = 98.0f;
    cfg.initial_value = 95.0f;
    cfg.ramp_period_seconds = 1800.0f; /* 30 minutes */
    cfg.update_interval_ms = 1000;
    Pattern_Generator_Register(&cfg);

    // 8. Accelerator Pedal Position (SPN 91) - Random walk
    cfg = {};
    cfg.spn = SPN_ACCEL_PEDAL_POS;
    cfg.pattern_type = PATTERN_RANDOM_WALK;
    cfg.min_value = 0.0f;
    cfg.max_value = 100.0f;
    cfg.initial_value = 15.0f;
    cfg.random_max_step = 2.0f;
    cfg.update_interval_ms = 50;
    Pattern_Generator_Register(&cfg);

    // 9. Engine Percent Load (SPN 92) - Random walk (correlates with accelerator pedal)
    cfg = {};
    cfg.spn = SPN_ENGINE_LOAD_PCT;
    cfg.pattern_type = PATTERN_RANDOM_WALK;
    cfg.min_value = 10.0f;
    cfg.max_value = 95.0f;
    cfg.initial_value = 35.0f;
    cfg.random_max_step = 2.0f;
    cfg.update_interval_ms = 50;
    Pattern_Generator_Register(&cfg);

    // 10. Transmission Current Gear (SPN 523) - State sequence
    cfg = {};
    cfg.spn = SPN_TRANS_GEAR;
    cfg.pattern_type = PATTERN_STATE_SEQUENCE;
    cfg.min_value = 1.0f;
    cfg.max_value = 8.0f;
    cfg.initial_value = 1.0f;
    cfg.state_values = gear_sequence;
    cfg.num_states = gear_sequence_length;
    cfg.state_hold_ms = 5000; /* 5 seconds per gear */
    cfg.update_interval_ms = 100;
    Pattern_Generator_Register(&cfg);

    // 11. Ambient Air Temperature (SPN 171) - Very slow sine wave (diurnal cycle)
    cfg = {};
    cfg.spn = SPN_AMBIENT_TEMP;
    cfg.pattern_type = PATTERN_SINE;
    cfg.min_value = 18.0f;
    cfg.max_value = 36.0f;
    cfg.initial_value = 24.0f;
    cfg.sine_period_seconds = 3600.0f; /* 1 hour */
    cfg.update_interval_ms = 1000;
    Pattern_Generator_Register(&cfg);

    // 12. Cab Interior Temperature (SPN 170) - Random walk around thermostat setpoint
    cfg = {};
    cfg.spn = SPN_CAB_TEMP;
    cfg.pattern_type = PATTERN_RANDOM_WALK;
    cfg.min_value = 20.0f;
    cfg.max_value = 25.0f;
    cfg.initial_value = 22.0f;
    cfg.random_max_step = 0.02f;
    cfg.update_interval_ms = 200;
    Pattern_Generator_Register(&cfg);

    // 13. Steering Wheel Angle (SPN 1807) - Sine wave (slight weaving)
    cfg = {};
    cfg.spn = SPN_STEERING_ANGLE;
    cfg.pattern_type = PATTERN_SINE;
    cfg.min_value = -0.8f;
    cfg.max_value = 0.8f;
    cfg.initial_value = 0.0f;
    cfg.sine_period_seconds = 12.0f;
    cfg.update_interval_ms = 20;
    Pattern_Generator_Register(&cfg);

    // 14. Yaw Rate (SPN 1408) - Sine wave
    cfg = {};
    cfg.spn = SPN_YAW_RATE;
    cfg.pattern_type = PATTERN_SINE;
    cfg.min_value = -0.2f;
    cfg.max_value = 0.2f;
    cfg.initial_value = 0.0f;
    cfg.sine_period_seconds = 12.0f;
    cfg.update_interval_ms = 20;
    Pattern_Generator_Register(&cfg);

    // 15. Turbocharger Boost Pressure (SPN 102) - Step changes
    cfg = {};
    cfg.spn = SPN_TURBO_BOOST_PRESS;
    cfg.pattern_type = PATTERN_STEP;
    cfg.min_value = 100.0f;
    cfg.max_value = 260.0f;
    cfg.initial_value = 110.0f;
    cfg.step_size = 15.0f;
    cfg.step_interval_ms = 3000;
    cfg.update_interval_ms = 50;
    Pattern_Generator_Register(&cfg);

    // 16. SCR Actual Dosing Rate (SPN 4331) - Random walk
    cfg = {};
    cfg.spn = SPN_SCR_DOSING_RATE;
    cfg.pattern_type = PATTERN_RANDOM_WALK;
    cfg.min_value = 0.0f;
    cfg.max_value = 1800.0f;
    cfg.initial_value = 250.0f;
    cfg.random_max_step = 10.0f;
    cfg.update_interval_ms = 100;
    Pattern_Generator_Register(&cfg);

    // 17. Estimated Percent Fan Speed (SPN 1639) - Ramp up/down
    cfg = {};
    cfg.spn = SPN_FAN_DRIVE_PCT;
    cfg.pattern_type = PATTERN_RAMP;
    cfg.min_value = 5.0f;
    cfg.max_value = 95.0f;
    cfg.initial_value = 10.0f;
    cfg.ramp_period_seconds = 90.0f;
    cfg.update_interval_ms = 100;
    Pattern_Generator_Register(&cfg);

    // 18. Actual Retarder Percent Torque (SPN 520) - Ramping
    cfg = {};
    cfg.spn = SPN_RETARDER_TORQUE;
    cfg.pattern_type = PATTERN_RAMP;
    cfg.min_value = -120.0f;
    cfg.max_value = 0.0f;
    cfg.initial_value = 0.0f;
    cfg.ramp_period_seconds = 45.0f;
    cfg.update_interval_ms = 100;
    Pattern_Generator_Register(&cfg);

    // 19. Brake Pedal Position (SPN 521) - Step pattern
    cfg = {};
    cfg.spn = SPN_BRAKE_PEDAL_POS;
    cfg.pattern_type = PATTERN_STEP;
    cfg.min_value = 0.0f;
    cfg.max_value = 100.0f;
    cfg.initial_value = 0.0f;
    cfg.step_size = 20.0f;
    cfg.step_interval_ms = 5000;
    cfg.update_interval_ms = 100;
    Pattern_Generator_Register(&cfg);
}

/* =============================================================================
   SETUP
   ============================================================================= */
void setup() {
    /* Initialize Serial Debug Port */
    Serial.begin(115200);
    while (!Serial && millis() < 3000); /* Wait up to 3 seconds for usb connection */

    Serial.println("=========================================");
    Serial.println("   STM32F103 J1939 CAN SIGNAL GENERATOR   ");
    Serial.println("=========================================");

    /* Initialize CAN hardware at 250 kbps */
    if (J1939_CAN_Hardware_Init()) {
        Serial.println("[SUCCESS] CAN controller started at 250 kbps.");
    } else {
        Serial.println("[ERROR] Failed to start CAN controller! System halted.");
        while (1) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }

    /* Configure built-in LED on Nucleo board */
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    /* Initialize pattern simulator database */
    Pattern_Generator_Init(millis());

    /* Register signal pattern settings */
    Configure_Signal_Generator_Patterns();
    Serial.print("[INFO] Signal simulation registered: ");
    Serial.print(Pattern_Generator_Get_Count());
    Serial.println(" patterns.");

    Serial.println("[INFO] System ready. Generating periodic J1939 CAN broadcast frames...");
    Serial.println("=========================================");
}

/* =============================================================================
   LOOP
   ============================================================================= */
void loop() {
    uint32_t current_time = millis();

    /* 1. Update the internal state of all signal pattern generators */
    Pattern_Generator_Update(current_time);

    /* 2. Traverse scheduler and transmit messages that have reached their periods */
    for (uint8_t i = 0; i < periodic_messages_count; i++) {
        J1939_Periodic_Msg_t* msg = &periodic_messages[i];
        
        if (current_time - msg->last_tx_ms >= msg->period_ms) {
            uint8_t payload[8];
            J1939_Initialize_Payload(payload); /* Reset buffer to 0xFF */

            /* Encode all SPNs registered inside this PGN */
            for (uint8_t s = 0; s < 8; s++) {
                uint32_t spn = msg->spns[s];
                if (spn == 0) {
                    break; /* End of SPN list */
                }

                /* Look up the database metadata for this SPN */
                const J1939_Signal_Definition_t* def = J1939_Find_Signal_By_SPN(spn);
                if (def != NULL) {
                    /* Determine midpoint of logical range to use as a fallback default */
                    float default_val = (def->min_physical + def->max_physical) * 0.5f;
                    
                    /* Retrieve the dynamically simulated physical value */
                    float value = Pattern_Generator_Get_Value(spn, default_val);

                    /* Encode the float into the binary payload byte buffer */
                    J1939_Encode_Signal(def, value, payload);
                }
            }

            /* Transmit the packed CAN frame */
            bool success = J1939_CAN_Hardware_Transmit(msg->pgn, 
                                                      payload, 
                                                      8, 
                                                      msg->priority, 
                                                      msg->source_address);

            if (success) {
                msg->last_tx_ms = current_time;
            }
        }
    }

    /* 3. Blink built-in LED briefly every 500ms to visually indicate normal operation */
    static uint32_t last_led_blink = 0;
    if (current_time - last_led_blink >= 500) {
        digitalToggle(LED_BUILTIN); /* Toggle Nucleo user LED */
        last_led_blink = current_time;

        /* Periodically output transmission count to serial */
        Serial.print("[HEARTBEAT] Time: ");
        Serial.print(current_time / 1000.0f);
        Serial.print("s | Running patterns: ");
        Serial.println(Pattern_Generator_Get_Count());
    }

    /* 4. Flush incoming message queue (to clear any RX buffers and prevent overflow) */
    uint32_t rx_pgn;
    uint8_t rx_payload[8];
    uint8_t rx_len;
    uint8_t rx_source;
    while (J1939_CAN_Hardware_Receive(&rx_pgn, rx_payload, &rx_len, &rx_source)) {
        /* Optional: print received requests or messages */
        Serial.print("[RX] Received CAN PGN: 0x");
        Serial.print(rx_pgn, HEX);
        Serial.print(" from Source: 0x");
        Serial.println(rx_source, HEX);
    }

    /* Small delay to prevent CPU thrashing */
    delay(1);
}
