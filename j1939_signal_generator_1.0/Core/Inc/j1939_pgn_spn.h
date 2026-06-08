/*
 * j1939_pgn_spn.h
 *
 *  Created on: Dec 8, 2025
 *      Author: abian
 */

#ifndef J1939_PGN_SPN_H
#define J1939_PGN_SPN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define J1939_ADDRESS_ECU         0x00
#define J1939_ADDRESS_DISPLAY     0x80
#define J1939_ADDRESS_ENGINE      0x00
#define J1939_ADDRESS_TRANSMISSION 0x03

// PGN Definitions from your document
#define PGN_ENGINE_TORQUE_REQUEST     0x0000
#define PGN_VEHICLE_SPEED             0xFEF1
#define PGN_TIME_DATE                 0xFEE6
#define PGN_ENGINE_FUEL               0xFEE9
#define PGN_ENGINE_FLUIDS             0xFEEF
#define PGN_ENGINE_FUEL_ECONOMY       0xFEF2
#define PGN_AMBIENT_CONDITIONS        0xFEF5
#define PGN_ENGINE_FLUIDS2            0xFEF6
#define PGN_ELECTRICAL                0xFEF7
#define PGN_TRANSMISSION_FLUIDS       0xFEF8
#define PGN_SEAT_BELT                 0xE000
#define PGN_RETARDER                  0xF000
#define PGN_BRAKE_ABS                 0xF001
#define PGN_TRANSMISSION_INFO         0xF002
#define PGN_ACCELERATOR_PEDAL         0xF003
#define PGN_ENGINE_INFO               0xF004
#define PGN_TRANSMISSION_GEAR         0xF005
#define PGN_VEHICLE_DYNAMICS          0xF009
#define PGN_ENGINE_AIR_FLOW           0xF00A
#define PGN_STEERING_ANGLE            0xF00B
#define PGN_TORQUE_CONVERTER          0xF00C
#define PGN_SCR_DOSING1               0xF023
#define PGN_SCR_DOSING2               0xF026
#define PGN_FAN_INFO                  0xFEBD
#define PGN_TURBO_TEMP                0xFE9A
#define PGN_EXHAUST_TEMP              0xFEA3
#define PGN_ENGINE_TEMP2              0xFEA4
#define PGN_TURBO_BOOST               0xFEA6
#define PGN_TOTAL_FUEL                0xFEAF
#define PGN_TRIP_INFO                 0xFEB0
#define PGN_TOTAL_DISTANCE            0xFEB1
#define PGN_TRIP_DISTANCE             0xFEBA
#define PGN_FUEL_INJECTION            0xFEDB
#define PGN_ENGINE_IDLE               0xFEDC
#define PGN_TURBO_SPEED               0xFEDD
#define PGN_AIR_START                 0xFEDE
#define PGN_ENGINE_CONTROL            0xFEDF
#define PGN_TRIP_DISTANCE2            0xFEE0
#define PGN_ENGINE_TORQUE_LIMIT       0xFEE3
#define PGN_ENGINE_TOTALS             0xFEE5
#define PGN_COMPASS_INFO              0xFEE8
#define PGN_ENGINE_TEMP               0xFEEE
#define PGN_AFTERTREATMENT_FUEL       0xFDA0
#define PGN_AFTERTREATMENT_NOX        0xF00E
#define PGN_AFTERTREATMENT_NOX2       0xF00F
#define PGN_SCR_TEMP1                 0xFD3E
#define PGN_SCR_TEMP2                 0xFD38
#define PGN_COOLANT_TEMP2             0xFD6F
#define PGN_DPF_PRESS2                0xFD8B
#define PGN_DPF_PRESS1                0xFD8C
#define PGN_TRANSMISSION_OIL_LIFE     0xFD95
#define PGN_DPF_TEMP2                 0xFDB0
#define PGN_DPF_TEMP1                 0xFDB3
#define PGN_LIGHTING                  0xFDCC
#define PGN_FUEL_FILTER               0xFE6A
#define PGN_TACHOGRAPH                0xFE6C
#define PGN_AUX_SENSORS               0xFE8C
#define PGN_FUEL_LEAKAGE              0xFE91
#define PGN_CLUTCH_LIFE               0xFEAB
#define PGN_BRAKE_LINING              0xFEAC
#define PGN_WHEEL_SPEEDS              0xFEBF
#define PGN_WASHER_FUEL              0xFEFC
#define PGN_SCR_TANK_INFO             0xFE56
#define PGN_BRAKE_PRESSURES           0xFEFA
#define PGN_GPS_LOCATION              0xFEF3
#define PGN_WATER_IN_FUEL             0xFEFF
#define PGN_TRANSMISSION_AWD          0xFFA1

// SPN Numbers from your document
typedef enum {
    // Engine parameters
    SPN_ENGINE_REQUESTED_TORQUE = 4191,
    SPN_VEHICLE_SPEED = 84,
    SPN_ENGINE_SPEED = 190,
    SPN_ENGINE_TORQUE = 513,
    SPN_ENGINE_PERCENT_LOAD = 92,
    SPN_ACCELERATOR_POSITION1 = 91,
    SPN_ACCELERATOR_POSITION2 = 29,

    // Fuel system
    SPN_ENGINE_FUEL_RATE = 183,
    SPN_ENGINE_FUEL_PRESSURE = 94,
    SPN_ENGINE_FUEL_TEMP = 174,
    SPN_FUEL_LEVEL1 = 96,
    SPN_FUEL_LEVEL2 = 38,

    // Temperatures
    SPN_ENGINE_COOLANT_TEMP = 110,
    SPN_ENGINE_OIL_TEMP = 175,
    SPN_INTAKE_AIR_TEMP = 172,
    SPN_ENGINE_EXHAUST_TEMP = 173,
    SPN_AMBIENT_AIR_TEMP = 171,
    SPN_CAB_INTERIOR_TEMP = 170,

    // Pressures
    SPN_ENGINE_OIL_PRESSURE = 100,
    SPN_INTAKE_MANIFOLD_PRESSURE = 102,
    SPN_TURBO_BOOST_PRESSURE = 1127,
    SPN_BAROMETRIC_PRESSURE = 108,

    // Electrical
    SPN_BATTERY_VOLTAGE = 167,
    SPN_ALTERNATOR_CURRENT = 115,
    SPN_NET_BATTERY_CURRENT = 114,

    // Transmission
    SPN_TRANSMISSION_GEAR = 523,
    SPN_TRANSMISSION_OIL_TEMP = 177,
    SPN_TRANSMISSION_OIL_PRESSURE = 127,

    // Aftertreatment
    SPN_SCR_DOSING_RATE1 = 4331,
    SPN_SCR_TANK_LEVEL = 1761,
    SPN_SCR_TANK_TEMP = 3031,
    SPN_DPF_INLET_PRESSURE = 81,
    SPN_DPF_DIFF_PRESSURE = 3610,

    // Vehicle dynamics
    SPN_STEERING_ANGLE = 1807,
    SPN_YAW_RATE = 1808,
    SPN_LATERAL_ACCEL = 1809,
    SPN_LONGITUDINAL_ACCEL = 1810,

    // Brakes
    SPN_BRAKE_PEDAL_POSITION = 521,
    SPN_BRAKE_APPLICATION_PRESSURE = 116,
    SPN_RETARDER_TORQUE = 520,

    // Other
    SPN_SEAT_BELT_STATUS = 1856,
    SPN_TURN_SIGNAL = 2876,
    SPN_HAZARD_LIGHTS = 2875,
    SPN_HIGH_BEAM = 2874,
    SPN_WATER_IN_FUEL = 97,
    SPN_WASHER_FLUID_LEVEL = 80,
    SPN_DPF_OUTLET_TEMP = 3246,
    SPN_FAN_SPEED = 1639,
    SPN_FAN_PERCENT_SPEED = 975,
} SPN_Number_t;

// Data structure for PGN/SPN
typedef struct {
    uint32_t PGN_Number;         // Parameter Group Number
    uint16_t SPN_Number;         // Suspect Parameter Number
    uint8_t Data_Length;         // Data Length (bytes)
    float Start_Position_Byte;   // Start byte.bit
    uint8_t Length_Bits;         // Number of bits for this SPN
    float Minimum_Data_Value;    // Minimum value of data range
    float Maximum_Data_Value;    // Maximum value of data range
    float Minimum_Operation_Value; // Minimum operation value
    float Maximum_Operation_Value; // Maximum operation value
    float Value_Per_Bit;         // Resolution (value per bit)
    float Offset;                // Offset value
    const char* Unit;            // Unit string
    const char* SPN_Name;        // SPN name
} PGN_SPN_Data_t;

// Function prototypes
float J1939_GetSPNValue(const PGN_SPN_Data_t* spn, uint8_t* data);
bool J1939_SetSPNValue(const PGN_SPN_Data_t* spn, uint8_t* data, float value);
const PGN_SPN_Data_t* J1939_FindSPN(uint16_t spn_number);
const PGN_SPN_Data_t* J1939_FindPGN(uint32_t pgn_number);

// Global PGN/SPN table (extern declaration)
extern const PGN_SPN_Data_t PGN_SPN_Table[];
extern const uint16_t PGN_SPN_Table_Size;

#ifdef __cplusplus
}
#endif

#endif // J1939_PGN_SPN_H
