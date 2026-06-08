/*
 * j1939_data.h
 *
 *  Created on: Dec 8, 2025
 *      Author: abian
 */

#ifndef J1939_DATA_H
#define J1939_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Vehicle data structure
typedef struct {
    // Engine parameters
    float engine_speed;          // RPM
    float engine_torque;         // %
    float engine_percent_load;   // %
    float engine_coolant_temp;   // °C
    float engine_exhaust_temp;
    float engine_oil_temp;       // °C
    float engine_oil_pressure;   // kPa
    float engine_fuel_rate;      // L/h
    float engine_fuel_temp;      // °C
    float engine_fuel_pressure;  // kPa

    // Vehicle parameters
    float vehicle_speed;         // km/h
    float accelerator_position;  // %
    float brake_pedal_position;  // %
    float brake_application_pressure;
    float retarder_torque;       // %
    float steering_angle;        // rad
    float yaw_rate;              // rad/s
    float lateral_acceleration;  // m/s²
    float longitudinal_acceleration; // m/s²

    // Transmission
    uint8_t transmission_gear;   // gear value
    float transmission_oil_temp; // °C
    float transmission_oil_pressure; // kPa

    // Electrical
    float battery_voltage;       // V
    float alternator_current;    // A
    float net_battery_current;   // A

    // Environmental
    float ambient_air_temp;      // °C
    float cab_interior_temp;     // °C
    float road_surface_temp;     // °C
    float barometric_pressure;   // kPa

    // Fuel system
    float fuel_level;            // %
    float washer_fluid_level;    // %

    // Aftertreatment
    float scr_dosing_rate;       // g/h
    float scr_tank_level;        // %
    float scr_tank_temp;         // °C
    float dpf_inlet_pressure;    // kPa
    float dpf_outlet_temp;       // °C

    // Turbo
    float turbo_boost_pressure;  // kPa
    float turbo_speed;           // RPM
    float intake_air_temp;       // °C
    float intake_manifold_pressure; // kPa

    // Status indicators
    bool seat_belt_buckled;
    bool turn_signal_left;
    bool turn_signal_right;
    bool hazard_lights;
    bool high_beam;
    bool water_in_fuel;
    bool abs_offroad_active;

    // Trip information
    float trip_distance;         // km
    float total_distance;        // km
    float trip_fuel;             // L
    float total_fuel;            // L
    float trip_idle_time;        // hr
    float trip_engine_runtime;   // hr

    // Fan
    float fan_speed;             // RPM
    float fan_percent_speed;     // %

    // Brake lining
    float brake_lining_front_left;   // %
    float brake_lining_front_right;  // %
    float brake_lining_rear_left;    // %
    float brake_lining_rear_right;   // %

    // GPS
    float latitude;              // deg
    float longitude;             // deg
    float compass_bearing;       // deg
    float pitch;                 // deg

    // Time and date
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} Vehicle_Data_t;

// Data update functions
void J1939_Update_Engine_Speed(float rpm);
void J1939_Update_Vehicle_Speed(float kmh);
void J1939_Update_Engine_Temperature(float coolant_temp, float oil_temp);
void J1939_Update_Fuel_System(float fuel_level, float fuel_rate, float fuel_pressure);
void J1939_Update_Electrical(float battery_voltage, float alternator_current);
void J1939_Update_Transmission(uint8_t gear, float oil_temp, float oil_pressure);
void J1939_Update_Aftertreatment(float scr_rate, float scr_level, float scr_temp);
void J1939_Update_Brakes(float pedal_position, float app_pressure, float lining_percent);
void J1939_Update_Environmental(float ambient_temp, float cab_temp, float road_temp);
void J1939_Update_Turbo(float boost_pressure, float turbo_speed);
void J1939_Update_Status_Indicators(void);
void J1939_Update_Time_Date(uint8_t sec, uint8_t min, uint8_t hr, uint8_t d, uint8_t m, uint8_t y);
void J1939_Update_GPS(float lat, float lon, float bearing, float pitch);

// Data getter
Vehicle_Data_t* J1939_Get_Vehicle_Data(void);

#ifdef __cplusplus
}
#endif

#endif // J1939_DATA_H
