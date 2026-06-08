/*
 * j1939_pgn_spn.c
 *
 *  Created on: Dec 8, 2025
 *      Author: abian
 */
#include "j1939_pgn_spn.h"
#include <string.h>

// Global PGN/SPN table (extracted from your document)
const PGN_SPN_Data_t PGN_SPN_Table[] = {
    // 1. Engine Requested Torque (PGN 0x0000)
    {
        .PGN_Number = PGN_ENGINE_TORQUE_REQUEST,
        .SPN_Number = SPN_ENGINE_REQUESTED_TORQUE,
        .Data_Length = 8,
        .Start_Position_Byte = 6.1f,
        .Length_Bits = 4,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 0.875f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.125f,
        .Offset = 0.0f,
        .Unit = "%",
        .SPN_Name = "Engine Requested Torque - High Resolution"
    },

    // 2. Vehicle Speed (PGN 0xFEF1)
    {
        .PGN_Number = PGN_VEHICLE_SPEED,
        .SPN_Number = SPN_VEHICLE_SPEED,
        .Data_Length = 8,
        .Start_Position_Byte = 2.0f, // Bytes 2-3
        .Length_Bits = 16,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 250.996f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.00390625f, // 1/256
        .Offset = 0.0f,
        .Unit = "km/h",
        .SPN_Name = "Wheel-Based Vehicle Speed"
    },

    // 3. Engine Speed (PGN 0xF004)
    {
        .PGN_Number = PGN_ENGINE_INFO,
        .SPN_Number = SPN_ENGINE_SPEED,
        .Data_Length = 8,
        .Start_Position_Byte = 4.0f,
        .Length_Bits = 16,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 8031.875f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.125f,
        .Offset = 0.0f,
        .Unit = "rpm",
        .SPN_Name = "Engine Speed"
    },

    // 4. Engine Torque (PGN 0xF004)
    {
        .PGN_Number = PGN_ENGINE_INFO,
        .SPN_Number = SPN_ENGINE_TORQUE,
        .Data_Length = 8,
        .Start_Position_Byte = 3.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = -125.0f,
        .Maximum_Data_Value = 125.0f,
        .Minimum_Operation_Value = 0.0f,
        .Maximum_Operation_Value = 125.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = -125.0f,
        .Unit = "%",
        .SPN_Name = "Actual Engine - Percent Torque"
    },

    // 5. Engine Percent Load (PGN 0xF003)
    {
        .PGN_Number = PGN_ACCELERATOR_PEDAL,
        .SPN_Number = SPN_ENGINE_PERCENT_LOAD,
        .Data_Length = 8,
        .Start_Position_Byte = 3.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 250.0f,
        .Minimum_Operation_Value = 0.0f,
        .Maximum_Operation_Value = 125.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = 0.0f,
        .Unit = "%",
        .SPN_Name = "Engine Percent Load At Current Speed"
    },

    // 6. Accelerator Position 1 (PGN 0xF003)
    {
        .PGN_Number = PGN_ACCELERATOR_PEDAL,
        .SPN_Number = SPN_ACCELERATOR_POSITION1,
        .Data_Length = 8,
        .Start_Position_Byte = 2.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 100.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.4f,
        .Offset = 0.0f,
        .Unit = "%",
        .SPN_Name = "Accelerator Pedal Position 1"
    },

    // 7. Engine Coolant Temperature (PGN 0xFEEE)
    {
        .PGN_Number = PGN_ENGINE_TEMP,
        .SPN_Number = SPN_ENGINE_COOLANT_TEMP,
        .Data_Length = 8,
        .Start_Position_Byte = 1.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = -40.0f,
        .Maximum_Data_Value = 210.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = -40.0f,
        .Unit = "deg C",
        .SPN_Name = "Engine Coolant Temperature"
    },

    // 8. Engine Oil Temperature (PGN 0xFEEE)
    {
        .PGN_Number = PGN_ENGINE_TEMP,
        .SPN_Number = SPN_ENGINE_OIL_TEMP,
        .Data_Length = 8,
        .Start_Position_Byte = 2.0f,
        .Length_Bits = 16,
        .Minimum_Data_Value = -273.0f,
        .Maximum_Data_Value = 1735.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.03125f,
        .Offset = -273.0f,
        .Unit = "deg C",
        .SPN_Name = "Engine Oil Temperature 1"
    },

    // 9. Engine Oil Pressure (PGN 0xFEEF)
    {
        .PGN_Number = PGN_ENGINE_FLUIDS,
        .SPN_Number = SPN_ENGINE_OIL_PRESSURE,
        .Data_Length = 8,
        .Start_Position_Byte = 4.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 1000.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 4.0f,
        .Offset = 0.0f,
        .Unit = "Kpa",
        .SPN_Name = "Engine Oil Pressure"
    },

    // 10. Engine Fuel Rate (PGN 0xFEF2)
    {
        .PGN_Number = PGN_ENGINE_FUEL_ECONOMY,
        .SPN_Number = SPN_ENGINE_FUEL_RATE,
        .Data_Length = 8,
        .Start_Position_Byte = 1.0f, // Bytes 1-2
        .Length_Bits = 16,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 3212.75f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.05f,
        .Offset = 0.0f,
        .Unit = "L/h",
        .SPN_Name = "Engine Fuel Rate"
    },

    // 11. Battery Voltage (PGN 0xFEF7)
    {
        .PGN_Number = PGN_ELECTRICAL,
        .SPN_Number = SPN_BATTERY_VOLTAGE,
        .Data_Length = 8,
        .Start_Position_Byte = 3.0f, // Bytes 3-4
        .Length_Bits = 16,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 3212.75f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.05f,
        .Offset = 0.0f,
        .Unit = "V",
        .SPN_Name = "Charging System Potential (Voltage)"
    },

    // 12. Ambient Air Temperature (PGN 0xFEF5)
    {
        .PGN_Number = PGN_AMBIENT_CONDITIONS,
        .SPN_Number = SPN_AMBIENT_AIR_TEMP,
        .Data_Length = 8,
        .Start_Position_Byte = 4.0f, // Bytes 4-5
        .Length_Bits = 16,
        .Minimum_Data_Value = -273.0f,
        .Maximum_Data_Value = 1735.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.03125f,
        .Offset = -273.0f,
        .Unit = "deg C",
        .SPN_Name = "Ambient Air Temperature"
    },

    // 13. Cab Interior Temperature (PGN 0xFEF5)
    {
        .PGN_Number = PGN_AMBIENT_CONDITIONS,
        .SPN_Number = SPN_CAB_INTERIOR_TEMP,
        .Data_Length = 8,
        .Start_Position_Byte = 2.0f, // Bytes 2-3
        .Length_Bits = 16,
        .Minimum_Data_Value = -273.0f,
        .Maximum_Data_Value = 1735.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.03125f,
        .Offset = -273.0f,
        .Unit = "deg C",
        .SPN_Name = "Cab Interior Temperature"
    },

    // 14. Fuel Level 1 (PGN 0xFEFC)
    {
        .PGN_Number = PGN_WASHER_FUEL,
        .SPN_Number = SPN_FUEL_LEVEL1,
        .Data_Length = 8,
        .Start_Position_Byte = 2.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 100.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.4f,
        .Offset = 0.0f,
        .Unit = "%",
        .SPN_Name = "Fuel Level 1"
    },

    // 15. Transmission Gear (PGN 0xF005)
    {
        .PGN_Number = PGN_TRANSMISSION_GEAR,
        .SPN_Number = SPN_TRANSMISSION_GEAR,
        .Data_Length = 8,
        .Start_Position_Byte = 4.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = -125.0f,
        .Maximum_Data_Value = 125.0f,
        .Minimum_Operation_Value = -125.0f,
        .Maximum_Operation_Value = 125.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = -125.0f,
        .Unit = "gear value",
        .SPN_Name = "Transmission Current Gear"
    },

    // 16. Transmission Oil Temperature (PGN 0xFEF8)
    {
        .PGN_Number = PGN_TRANSMISSION_FLUIDS,
        .SPN_Number = SPN_TRANSMISSION_OIL_TEMP,
        .Data_Length = 8,
        .Start_Position_Byte = 5.0f, // Bytes 5-6
        .Length_Bits = 16,
        .Minimum_Data_Value = -273.0f,
        .Maximum_Data_Value = 1735.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.03125f,
        .Offset = -273.0f,
        .Unit = "deg C",
        .SPN_Name = "Transmission Oil Temperature"
    },

    // 17. Transmission Oil Pressure (PGN 0xFEF8)
    {
        .PGN_Number = PGN_TRANSMISSION_FLUIDS,
        .SPN_Number = SPN_TRANSMISSION_OIL_PRESSURE,
        .Data_Length = 8,
        .Start_Position_Byte = 4.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 4000.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 16.0f,
        .Offset = 0.0f,
        .Unit = "Kpa",
        .SPN_Name = "Transmission Oil Pressure"
    },

    // 18. SCR Dosing Rate 1 (PGN 0xF023)
    {
        .PGN_Number = PGN_SCR_DOSING1,
        .SPN_Number = SPN_SCR_DOSING_RATE1,
        .Data_Length = 8,
        .Start_Position_Byte = 1.0f, // Bytes 1-2
        .Length_Bits = 16,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 19276.5f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.3f,
        .Offset = 0.0f,
        .Unit = "g/h",
        .SPN_Name = "Aftertreatment 1 SCR Actual Dosing Reagent Quantity"
    },

    // 19. SCR Tank Level (PGN 0xFE56)
    {
        .PGN_Number = PGN_SCR_TANK_INFO,
        .SPN_Number = SPN_SCR_TANK_LEVEL,
        .Data_Length = 8,
        .Start_Position_Byte = 1.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 100.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.4f,
        .Offset = 0.0f,
        .Unit = "%",
        .SPN_Name = "Aftertreatment 1 SCR Catalyst Tank Level"
    },

    // 20. SCR Tank Temperature (PGN 0xFE56)
    {
        .PGN_Number = PGN_SCR_TANK_INFO,
        .SPN_Number = SPN_SCR_TANK_TEMP,
        .Data_Length = 8,
        .Start_Position_Byte = 2.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = -40.0f,
        .Maximum_Data_Value = 210.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = -40.0f,
        .Unit = "deg C",
        .SPN_Name = "Aftertreatment 1 SCR Catalyst Tank Temperature"
    },

    // 21. DPF Inlet Pressure (PGN 0xFEF6)
    {
        .PGN_Number = PGN_ENGINE_FLUIDS2,
        .SPN_Number = SPN_DPF_INLET_PRESSURE,
        .Data_Length = 8,
        .Start_Position_Byte = 1.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 125.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.5f,
        .Offset = 0.0f,
        .Unit = "kPa",
        .SPN_Name = "Engine Diesel Particulate Filter Inlet Pressure"
    },

    // 22. DPF Outlet Temperature (PGN 0xFDB3)
    {
        .PGN_Number = PGN_DPF_TEMP1,
        .SPN_Number = SPN_DPF_OUTLET_TEMP,
        .Data_Length = 8,
        .Start_Position_Byte = 3.0f, // Bytes 3-4
        .Length_Bits = 16,
        .Minimum_Data_Value = -273.0f,
        .Maximum_Data_Value = 1735.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.03125f,
        .Offset = -273.0f,
        .Unit = "C",
        .SPN_Name = "Aftertreatment 1 Diesel Particulate Filter Outlet Gas Temperature"
    },

    // 23. Fan Speed (PGN 0xFEBD)
    {
        .PGN_Number = PGN_FAN_INFO,
        .SPN_Number = SPN_FAN_SPEED,
        .Data_Length = 8,
        .Start_Position_Byte = 3.0f, // Bytes 3-4
        .Length_Bits = 16,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 8031.875f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.125f,
        .Offset = 0.0f,
        .Unit = "Rpm",
        .SPN_Name = "Fan Speed"
    },

    // 24. Fan Percent Speed (PGN 0xFEBD)
    {
        .PGN_Number = PGN_FAN_INFO,
        .SPN_Number = SPN_FAN_PERCENT_SPEED,
        .Data_Length = 8,
        .Start_Position_Byte = 1.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 100.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.4f,
        .Offset = 0.0f,
        .Unit = "%",
        .SPN_Name = "Estimated Percent Fan Speed"
    },

    // 25. Steering Angle (PGN 0xF009)
    {
        .PGN_Number = PGN_VEHICLE_DYNAMICS,
        .SPN_Number = SPN_STEERING_ANGLE,
        .Data_Length = 8,
        .Start_Position_Byte = 1.0f, // Bytes 1-2
        .Length_Bits = 16,
        .Minimum_Data_Value = -31.374f,
        .Maximum_Data_Value = 31.374f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.0009765625f, // 1/1024
        .Offset = -31.374f,
        .Unit = "rad",
        .SPN_Name = "Steering Wheel Angle"
    },

    // 26. Yaw Rate (PGN 0xF009)
    {
        .PGN_Number = PGN_VEHICLE_DYNAMICS,
        .SPN_Number = SPN_YAW_RATE,
        .Data_Length = 8,
        .Start_Position_Byte = 4.0f, // Bytes 4-5
        .Length_Bits = 16,
        .Minimum_Data_Value = -3.92f,
        .Maximum_Data_Value = 3.92f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.00012207031f, // 1/8192
        .Offset = -3.92f,
        .Unit = "rad/s",
        .SPN_Name = "Yaw Rate"
    },

    // 27. Lateral Acceleration (PGN 0xF009)
    {
        .PGN_Number = PGN_VEHICLE_DYNAMICS,
        .SPN_Number = SPN_LATERAL_ACCEL,
        .Data_Length = 8,
        .Start_Position_Byte = 6.0f, // Bytes 6-7
        .Length_Bits = 16,
        .Minimum_Data_Value = -15.687f,
        .Maximum_Data_Value = 15.687f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.00048828125f, // 1/2048
        .Offset = -15.687f,
        .Unit = "m/s²",
        .SPN_Name = "Lateral Acceleration"
    },

    // 28. Longitudinal Acceleration (PGN 0xF009)
    {
        .PGN_Number = PGN_VEHICLE_DYNAMICS,
        .SPN_Number = SPN_LONGITUDINAL_ACCEL,
        .Data_Length = 8,
        .Start_Position_Byte = 8.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = -12.5f,
        .Maximum_Data_Value = 12.5f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.1f,
        .Offset = -12.5f,
        .Unit = "m/s²",
        .SPN_Name = "Longitudinal Acceleration"
    },

    // 29. Brake Pedal Position (PGN 0xF001)
    {
        .PGN_Number = PGN_BRAKE_ABS,
        .SPN_Number = SPN_BRAKE_PEDAL_POSITION,
        .Data_Length = 8,
        .Start_Position_Byte = 2.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 100.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.4f,
        .Offset = 0.0f,
        .Unit = "%",
        .SPN_Name = "Brake Pedal Position"
    },

    // 30. Retarder Torque (PGN 0xF000)
    {
        .PGN_Number = PGN_RETARDER,
        .SPN_Number = SPN_RETARDER_TORQUE,
        .Data_Length = 8,
        .Start_Position_Byte = 2.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = -125.0f,
        .Maximum_Data_Value = 125.0f,
        .Minimum_Operation_Value = -125.0f,
        .Maximum_Operation_Value = 0.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = -125.0f,
        .Unit = "%",
        .SPN_Name = "Actual Retarder-Percent Torque"
    },

    // 31. Brake Application Pressure (PGN 0xFEFA)
    {
        .PGN_Number = PGN_BRAKE_PRESSURES,
        .SPN_Number = SPN_BRAKE_APPLICATION_PRESSURE,
        .Data_Length = 8,
        .Start_Position_Byte = 1.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 1000.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 4.0f,
        .Offset = 0.0f,
        .Unit = "kPa",
        .SPN_Name = "Brake Application Pressure"
    },

    // 32. Seat Belt Status (PGN 0xE000)
    {
        .PGN_Number = PGN_SEAT_BELT,
        .SPN_Number = SPN_SEAT_BELT_STATUS,
        .Data_Length = 8,
        .Start_Position_Byte = 4.7f,
        .Length_Bits = 2,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 3.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = 0.0f,
        .Unit = "Status",
        .SPN_Name = "Seat Belt Switch"
    },

    // 33. Turn Signal (PGN 0xFDCC)
    {
        .PGN_Number = PGN_LIGHTING,
        .SPN_Number = SPN_TURN_SIGNAL,
        .Data_Length = 8,
        .Start_Position_Byte = 2.1f,
        .Length_Bits = 4,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 15.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = 0.0f,
        .Unit = "Status",
        .SPN_Name = "Turn Signal Switch"
    },

    // 34. Hazard Lights (PGN 0xFDCC)
    {
        .PGN_Number = PGN_LIGHTING,
        .SPN_Number = SPN_HAZARD_LIGHTS,
        .Data_Length = 8,
        .Start_Position_Byte = 2.5f,
        .Length_Bits = 2,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 3.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = 0.0f,
        .Unit = "Status",
        .SPN_Name = "Hazard Light Switch"
    },

    // 35. High Beam (PGN 0xFDCC)
    {
        .PGN_Number = PGN_LIGHTING,
        .SPN_Number = SPN_HIGH_BEAM,
        .Data_Length = 8,
        .Start_Position_Byte = 2.7f,
        .Length_Bits = 2,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 3.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = 0.0f,
        .Unit = "Status",
        .SPN_Name = "High-Low Beam Switch"
    },

    // 36. Water In Fuel (PGN 0xFEFF)
    {
        .PGN_Number = PGN_WATER_IN_FUEL,
        .SPN_Number = SPN_WATER_IN_FUEL,
        .Data_Length = 8,
        .Start_Position_Byte = 1.1f,
        .Length_Bits = 2,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 3.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = 0.0f,
        .Unit = "Status",
        .SPN_Name = "Water In Fuel Indicator"
    },

    // 37. Washer Fluid Level (PGN 0xFEFC)
    {
        .PGN_Number = PGN_WASHER_FUEL,
        .SPN_Number = SPN_WASHER_FLUID_LEVEL,
        .Data_Length = 8,
        .Start_Position_Byte = 1.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 100.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.4f,
        .Offset = 0.0f,
        .Unit = "%",
        .SPN_Name = "Washer Fluid Level"
    },

    // 38. Intake Air Temperature (PGN 0xFEF5)
    {
        .PGN_Number = PGN_AMBIENT_CONDITIONS,
        .SPN_Number = SPN_INTAKE_AIR_TEMP,
        .Data_Length = 8,
        .Start_Position_Byte = 6.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = -40.0f,
        .Maximum_Data_Value = 210.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 1.0f,
        .Offset = -40.0f,
        .Unit = "deg C",
        .SPN_Name = "Engine Air Inlet Temperature"
    },

    // 39. Intake Manifold Pressure (PGN 0xFEF6)
    {
        .PGN_Number = PGN_ENGINE_FLUIDS2,
        .SPN_Number = SPN_INTAKE_MANIFOLD_PRESSURE,
        .Data_Length = 8,
        .Start_Position_Byte = 2.0f,
        .Length_Bits = 8,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 500.0f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 2.0f,
        .Offset = 0.0f,
        .Unit = "kPa",
        .SPN_Name = "Engine Intake Manifold #1 Pressure"
    },

    // 40. Turbo Boost Pressure (PGN 0xFEA6)
    {
        .PGN_Number = PGN_TURBO_BOOST,
        .SPN_Number = SPN_TURBO_BOOST_PRESSURE,
        .Data_Length = 8,
        .Start_Position_Byte = 1.0f, // Bytes 1-2
        .Length_Bits = 16,
        .Minimum_Data_Value = 0.0f,
        .Maximum_Data_Value = 8031.875f,
        .Minimum_Operation_Value = -99.0f,
        .Maximum_Operation_Value = 99.0f,
        .Value_Per_Bit = 0.125f,
        .Offset = 0.0f,
        .Unit = "kPa",
        .SPN_Name = "Engine Turbocharger 1 Boost Pressure"
    },

    // Add more SPNs here as needed...
};

const uint16_t PGN_SPN_Table_Size = sizeof(PGN_SPN_Table) / sizeof(PGN_SPN_Data_t);

// Helper function to extract bits from data
static uint32_t ExtractBits(const uint8_t* data, uint8_t start_bit, uint8_t length_bits) {
    uint32_t value = 0;
    uint8_t bit_index = start_bit;

    for (uint8_t i = 0; i < length_bits; i++) {
        uint8_t byte_index = bit_index / 8;
        uint8_t bit_in_byte = bit_index % 8;

        if (data[byte_index] & (1 << bit_in_byte)) {
            value |= (1 << i);
        }

        bit_index++;
    }

    return value;
}

// Helper function to set bits in data
static void SetBits(uint8_t* data, uint8_t start_bit, uint8_t length_bits, uint32_t value) {
    uint8_t bit_index = start_bit;

    for (uint8_t i = 0; i < length_bits; i++) {
        uint8_t byte_index = bit_index / 8;
        uint8_t bit_in_byte = bit_index % 8;

        if (value & (1 << i)) {
            data[byte_index] |= (1 << bit_in_byte);
        } else {
            data[byte_index] &= ~(1 << bit_in_byte);
        }

        bit_index++;
    }
}

// Get SPN value from CAN data
float J1939_GetSPNValue(const PGN_SPN_Data_t* spn, uint8_t* data) {
    if (spn == NULL || data == NULL) {
        return 0.0f;
    }

    // Calculate start bit
    uint8_t start_byte = (uint8_t)spn->Start_Position_Byte;
    uint8_t start_bit_offset = (uint8_t)((spn->Start_Position_Byte - start_byte) * 10);
    uint8_t start_bit = start_byte * 8 + start_bit_offset;

    // Extract bits
    uint32_t raw_value = ExtractBits(data, start_bit, spn->Length_Bits);

    // Convert to physical value
    float physical_value = (float)raw_value * spn->Value_Per_Bit + spn->Offset;

    return physical_value;
}

// Set SPN value in CAN data
bool J1939_SetSPNValue(const PGN_SPN_Data_t* spn, uint8_t* data, float value) {
    if (spn == NULL || data == NULL) {
        return false;
    }

    // Check value range
    if (value < spn->Minimum_Operation_Value || value > spn->Maximum_Operation_Value) {
        return false;
    }

    // Calculate start bit
    uint8_t start_byte = (uint8_t)spn->Start_Position_Byte;
    uint8_t start_bit_offset = (uint8_t)((spn->Start_Position_Byte - start_byte) * 10);
    uint8_t start_bit = start_byte * 8 + start_bit_offset;

    // Convert to raw value
    float raw_value_float = (value - spn->Offset) / spn->Value_Per_Bit;
    uint32_t raw_value = (uint32_t)raw_value_float;

    // Set bits
    SetBits(data, start_bit, spn->Length_Bits, raw_value);

    return true;
}

// Find SPN in table
const PGN_SPN_Data_t* J1939_FindSPN(uint16_t spn_number) {
    for (uint16_t i = 0; i < PGN_SPN_Table_Size; i++) {
        if (PGN_SPN_Table[i].SPN_Number == spn_number) {
            return &PGN_SPN_Table[i];
        }
    }
    return NULL;
}

// Find PGN in table (first occurrence)
const PGN_SPN_Data_t* J1939_FindPGN(uint32_t pgn_number) {
    for (uint16_t i = 0; i < PGN_SPN_Table_Size; i++) {
        if (PGN_SPN_Table[i].PGN_Number == pgn_number) {
            return &PGN_SPN_Table[i];
        }
    }
    return NULL;
}


