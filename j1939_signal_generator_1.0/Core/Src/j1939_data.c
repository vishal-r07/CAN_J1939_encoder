/*
 * j1939_data.c
 *
 *  Created on: Dec 8, 2025
 *      Author: abian
 */

#include "j1939_data.h"
#include <string.h>

static Vehicle_Data_t vehicle_data = {0};

// Get vehicle data structure
Vehicle_Data_t* J1939_Get_Vehicle_Data(void) {
    return &vehicle_data;
}

// Update engine speed
void J1939_Update_Engine_Speed(float rpm) {
    vehicle_data.engine_speed = rpm;
}

// Update vehicle speed
void J1939_Update_Vehicle_Speed(float kmh) {
    vehicle_data.vehicle_speed = kmh;
}

// Update engine temperatures
void J1939_Update_Engine_Temperature(float coolant_temp, float oil_temp) {
    vehicle_data.engine_coolant_temp = coolant_temp;
    vehicle_data.engine_oil_temp = oil_temp;
}

// Update fuel system
void J1939_Update_Fuel_System(float fuel_level, float fuel_rate, float fuel_pressure) {
    vehicle_data.fuel_level = fuel_level;
    vehicle_data.engine_fuel_rate = fuel_rate;
    vehicle_data.engine_fuel_pressure = fuel_pressure;
}

// Update electrical system
void J1939_Update_Electrical(float battery_voltage, float alternator_current) {
    vehicle_data.battery_voltage = battery_voltage;
    vehicle_data.alternator_current = alternator_current;
}

// Update transmission
void J1939_Update_Transmission(uint8_t gear, float oil_temp, float oil_pressure) {
    vehicle_data.transmission_gear = gear;
    vehicle_data.transmission_oil_temp = oil_temp;
    vehicle_data.transmission_oil_pressure = oil_pressure;
}

// Update aftertreatment
void J1939_Update_Aftertreatment(float scr_rate, float scr_level, float scr_temp) {
    vehicle_data.scr_dosing_rate = scr_rate;
    vehicle_data.scr_tank_level = scr_level;
    vehicle_data.scr_tank_temp = scr_temp;
}

// Update brakes
void J1939_Update_Brakes(float pedal_position, float app_pressure, float lining_percent) {
    vehicle_data.brake_pedal_position = pedal_position;
    vehicle_data.brake_application_pressure = app_pressure;  // FIXED
    vehicle_data.brake_lining_front_left = lining_percent;
    vehicle_data.brake_lining_front_right = lining_percent;
    vehicle_data.brake_lining_rear_left = lining_percent;
    vehicle_data.brake_lining_rear_right = lining_percent;
}

// Update environmental
void J1939_Update_Environmental(float ambient_temp, float cab_temp, float road_temp) {
    vehicle_data.ambient_air_temp = ambient_temp;
    vehicle_data.cab_interior_temp = cab_temp;
    vehicle_data.road_surface_temp = road_temp;
}

// Update turbo
void J1939_Update_Turbo(float boost_pressure, float turbo_speed) {
    vehicle_data.turbo_boost_pressure = boost_pressure;
    vehicle_data.turbo_speed = turbo_speed;
}

// Update status indicators
void J1939_Update_Status_Indicators(void) {
    // This would read actual hardware inputs
    vehicle_data.seat_belt_buckled = false;
    vehicle_data.turn_signal_left = false;
    vehicle_data.turn_signal_right = false;
    vehicle_data.hazard_lights = false;
    vehicle_data.high_beam = false;
    vehicle_data.water_in_fuel = false;
    vehicle_data.abs_offroad_active = false;
}

// Update time and date
void J1939_Update_Time_Date(uint8_t sec, uint8_t min, uint8_t hr, uint8_t d, uint8_t m, uint8_t y) {
    vehicle_data.seconds = sec;
    vehicle_data.minutes = min;
    vehicle_data.hours = hr;
    vehicle_data.day = d;
    vehicle_data.month = m;
    vehicle_data.year = y;
}

// Update GPS
void J1939_Update_GPS(float lat, float lon, float bearing, float pitch) {
    vehicle_data.latitude = lat;
    vehicle_data.longitude = lon;
    vehicle_data.compass_bearing = bearing;
    vehicle_data.pitch = pitch;
}
