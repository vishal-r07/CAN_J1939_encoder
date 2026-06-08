/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "j1939_can.h"
#include "j1939_data.h"
#include "j1939_pgn_spn.h"
#include "j1939_patterns.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>  // For rand()
#define PI 3.14159265358979323846f
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Duration options (seconds)
#define DURATION_5_MIN      300
#define DURATION_30_MIN     1800
#define DURATION_1_HOUR     3600
#define DURATION_2_HOURS    7200

// Current duration setting
static uint32_t current_duration = DURATION_5_MIN;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

/* USER CODE BEGIN PV */
// Periodic transmission structures
static J1939_Periodic_TX_t periodic_msgs[50];
static uint8_t periodic_msg_count = 0;

// Data buffers for each PGN
static uint8_t pgn_engine_info_data[8] = {0};
static uint8_t pgn_vehicle_speed_data[8] = {0};
static uint8_t pgn_engine_fluids_data[8] = {0};
//static uint8_t pgn_engine_fuel_economy_data[8] = {0};
static uint8_t pgn_ambient_data[8] = {0};
static uint8_t pgn_electrical_data[8] = {0};
//static uint8_t pgn_transmission_fluids_data[8] = {0};
static uint8_t pgn_accelerator_data[8] = {0};
static uint8_t pgn_transmission_gear_data[8] = {0};
static uint8_t pgn_vehicle_dynamics_data[8] = {0};
static uint8_t pgn_engine_temp_data[8] = {0};
static uint8_t pgn_brake_abs_data[8] = {0};
//static uint8_t pgn_retarder_data[8] = {0};
static uint8_t pgn_scr_dosing_data[8] = {0};
static uint8_t pgn_fan_info_data[8] = {0};
static uint8_t pgn_turbo_boost_data[8] = {0};
//static uint8_t pgn_brake_pressures_data[8] = {0};
//static uint8_t pgn_lighting_data[8] = {0};
static uint8_t pgn_washer_fuel_data[8] = {0};
//static uint8_t pgn_scr_tank_data[8] = {0};
static uint8_t pgn_gps_data[8] = {0};
//static uint8_t pgn_water_in_fuel_data[8] = {0};

// Test mode flags
static bool test_running = false;
static uint32_t test_start_time = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
/* USER CODE BEGIN PFP */
static void J1939_Setup_Periodic_Transmissions(void);
static void J1939_Update_Sensor_Data(void);
static void J1939_Configure_Patterns(void);
static void J1939_Start_Test(uint32_t duration_seconds);
static void J1939_Stop_Test(void);
static void J1939_Update_Display(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Simple LED blinking for status
static void LED_Blink(uint8_t times, uint32_t delay_ms) {
    for (uint8_t i = 0; i < times; i++) {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
        HAL_Delay(delay_ms);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
        HAL_Delay(delay_ms);
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();

  /* USER CODE BEGIN 2 */
  // Initialize J1939 CAN
  J1939_CAN_Init(&hcan);

  // Initialize pattern system
  Pattern_Init();

  // Configure patterns for all signals
  J1939_Configure_Patterns();

  // Setup periodic transmissions
  J1939_Setup_Periodic_Transmissions();

  // Initialize vehicle data
  J1939_Update_Time_Date(0, 0, 0, 1, 1, 24);
  J1939_Update_GPS(0.0f, 0.0f, 0.0f, 0.0f);

  // Signal ready with LED blink
  LED_Blink(3, 200);

  // Start test automatically (remove this if you want manual start)
  J1939_Start_Test(current_duration);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // Get current time
    uint32_t current_time = HAL_GetTick();

    // Update patterns if test is running
    if (test_running) {
        Pattern_Update(current_time);
        J1939_Update_Sensor_Data();
    }

    // Process J1939 communication
    J1939_Process();

    // Update status display
    J1939_Update_Display();

    // Check if test duration expired
    if (test_running && Pattern_IsRunning() == false) {
        J1939_Stop_Test();
    }

    // Delay for main loop
    HAL_Delay(1);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{
  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 4;           // For 250 kbps with 72MHz PCLK1
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

// Configure patterns for all J1939 signals
static void J1939_Configure_Patterns(void) {
    // Engine Speed (SPN 190) - Sine wave pattern
    Pattern_Configure(SPN_ENGINE_SPEED, PATTERN_SINE, 600.0f, 2200.0f, 30.0f); // 30 second period

    // Vehicle Speed (SPN 84) - Ramp pattern
    Pattern_Configure(SPN_VEHICLE_SPEED, PATTERN_RAMP, 0.0f, 120.0f, 60.0f); // 0-120 km/h over 60 seconds

    // Engine Coolant Temp (SPN 110) - Random walk
    Pattern_Configure(SPN_ENGINE_COOLANT_TEMP, PATTERN_RANDOM_WALK, 75.0f, 105.0f, 0.0f);

    // Engine Oil Temp (SPN 175) - Correlated with coolant temp
    Pattern_Configure(SPN_ENGINE_OIL_TEMP, PATTERN_RANDOM_WALK, 80.0f, 110.0f, 0.0f);

    // Engine Oil Pressure (SPN 100) - Step pattern
    Pattern_Configure(SPN_ENGINE_OIL_PRESSURE, PATTERN_STEP, 200.0f, 400.0f, 0.0f);

    // Battery Voltage (SPN 167) - Sine wave
    Pattern_Configure(SPN_BATTERY_VOLTAGE, PATTERN_SINE, 23.5f, 28.5f, 120.0f);

    // Fuel Level (SPN 96) - Ramp down
    Pattern_Configure(SPN_FUEL_LEVEL1, PATTERN_RAMP, 100.0f, 20.0f, 1800.0f); // Empty over 30 minutes

    // Accelerator Position (SPN 91) - Random walk
    Pattern_Configure(SPN_ACCELERATOR_POSITION1, PATTERN_RANDOM_WALK, 0.0f, 100.0f, 0.0f);

    // Engine Percent Load (SPN 92) - Correlated with accelerator
    Pattern_Configure(SPN_ENGINE_PERCENT_LOAD, PATTERN_RANDOM_WALK, 0.0f, 100.0f, 0.0f);

    // Transmission Gear (SPN 523) - State machine
    float gear_states[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f};
    Pattern_ConfigureStateMachine(SPN_TRANSMISSION_GEAR, 14, gear_states, 5000); // 5 seconds per gear

    // Ambient Air Temp (SPN 171) - Sine wave (daily cycle)
    Pattern_Configure(SPN_AMBIENT_AIR_TEMP, PATTERN_SINE, 15.0f, 35.0f, 3600.0f); // 1 hour period

    // Cab Interior Temp (SPN 170) - Correlated with ambient
    Pattern_Configure(SPN_CAB_INTERIOR_TEMP, PATTERN_RANDOM_WALK, 18.0f, 28.0f, 0.0f);

    // Turbo Boost Pressure (SPN 1127) - Sine wave
    Pattern_Configure(SPN_TURBO_BOOST_PRESSURE, PATTERN_SINE, 100.0f, 250.0f, 15.0f);

    // SCR Dosing Rate (SPN 4331) - Step pattern
    Pattern_Configure(SPN_SCR_DOSING_RATE1, PATTERN_STEP, 0.0f, 2000.0f, 0.0f);

    // Fan Speed (SPN 1639) - Ramp pattern
    Pattern_Configure(SPN_FAN_SPEED, PATTERN_RAMP, 500.0f, 2000.0f, 120.0f);

    // Brake Pedal Position (SPN 521) - Random walk
    Pattern_Configure(SPN_BRAKE_PEDAL_POSITION, PATTERN_RANDOM_WALK, 0.0f, 20.0f, 0.0f);

    // Steering Angle (SPN 1807) - Sine wave
    Pattern_Configure(SPN_STEERING_ANGLE, PATTERN_SINE, -0.5f, 0.5f, 10.0f);

    // Add more SPN patterns as needed...
}

// Start test with specified duration
static void J1939_Start_Test(uint32_t duration_seconds) {
    Pattern_SetDuration(duration_seconds);
    Pattern_Start();
    test_running = true;
    test_start_time = HAL_GetTick();

    // Blink LED to indicate start
    LED_Blink(2, 100);
    HAL_Delay(500);
    LED_Blink(2, 100);
}

// Stop test
static void J1939_Stop_Test(void) {
    test_running = false;
    Pattern_Stop();

    // Long blink to indicate end
    LED_Blink(1, 1000);
}

// Setup periodic transmissions (simplified version)
static void J1939_Setup_Periodic_Transmissions(void) {
    uint8_t idx = 0;

    // Key PGNs with their transmission rates
    // Engine Info (10-50ms) - using 20ms
    periodic_msgs[idx].pgn = PGN_ENGINE_INFO;
    periodic_msgs[idx].data = pgn_engine_info_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 20;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_ENGINE;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    // Vehicle Speed (100ms)
    periodic_msgs[idx].pgn = PGN_VEHICLE_SPEED;
    periodic_msgs[idx].data = pgn_vehicle_speed_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 100;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_ECU;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    // Engine Fluids (1s)
    periodic_msgs[idx].pgn = PGN_ENGINE_FLUIDS;
    periodic_msgs[idx].data = pgn_engine_fluids_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 1000;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_ENGINE;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    // Electrical (1s)
    periodic_msgs[idx].pgn = PGN_ELECTRICAL;
    periodic_msgs[idx].data = pgn_electrical_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 1000;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_ECU;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    // Transmission Gear (100ms)
    periodic_msgs[idx].pgn = PGN_TRANSMISSION_GEAR;
    periodic_msgs[idx].data = pgn_transmission_gear_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 100;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_TRANSMISSION;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    // Ambient Conditions (1s)
    periodic_msgs[idx].pgn = PGN_AMBIENT_CONDITIONS;
    periodic_msgs[idx].data = pgn_ambient_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 1000;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_ECU;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    // Engine Temperature (1s)
    periodic_msgs[idx].pgn = PGN_ENGINE_TEMP;
    periodic_msgs[idx].data = pgn_engine_temp_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 1000;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_ENGINE;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    // Fan Info (1s)
    periodic_msgs[idx].pgn = PGN_FAN_INFO;
    periodic_msgs[idx].data = pgn_fan_info_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 1000;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_ENGINE;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    // Brake ABS (100ms)
    periodic_msgs[idx].pgn = PGN_BRAKE_ABS;
    periodic_msgs[idx].data = pgn_brake_abs_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 100;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_ECU;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    // SCR Dosing (50ms)
    periodic_msgs[idx].pgn = PGN_SCR_DOSING1;
    periodic_msgs[idx].data = pgn_scr_dosing_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 50;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_ECU;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    // GPS Location (1s)
    periodic_msgs[idx].pgn = PGN_GPS_LOCATION;
    periodic_msgs[idx].data = pgn_gps_data;
    periodic_msgs[idx].data_length = 8;
    periodic_msgs[idx].period_ms = 1000;
    periodic_msgs[idx].priority = 3;
    periodic_msgs[idx].source_address = J1939_ADDRESS_ECU;
    periodic_msgs[idx].last_tx_time = 0;
    J1939_Add_Periodic_TX(&periodic_msgs[idx++]);

    periodic_msg_count = idx;
}

// Update sensor data from patterns
static void J1939_Update_Sensor_Data(void) {
    Vehicle_Data_t* data = J1939_Get_Vehicle_Data();
    const PGN_SPN_Data_t* spn_def;

    // Get values from pattern generators
    data->engine_speed = Pattern_GetValue(SPN_ENGINE_SPEED);
    data->vehicle_speed = Pattern_GetValue(SPN_VEHICLE_SPEED);
    data->engine_coolant_temp = Pattern_GetValue(SPN_ENGINE_COOLANT_TEMP);
    data->engine_oil_temp = Pattern_GetValue(SPN_ENGINE_OIL_TEMP);
    data->engine_oil_pressure = Pattern_GetValue(SPN_ENGINE_OIL_PRESSURE);
    data->battery_voltage = Pattern_GetValue(SPN_BATTERY_VOLTAGE);
    data->fuel_level = Pattern_GetValue(SPN_FUEL_LEVEL1);
    data->accelerator_position = Pattern_GetValue(SPN_ACCELERATOR_POSITION1);
    data->engine_percent_load = Pattern_GetValue(SPN_ENGINE_PERCENT_LOAD);
    data->transmission_gear = (uint8_t)Pattern_GetValue(SPN_TRANSMISSION_GEAR);
    data->ambient_air_temp = Pattern_GetValue(SPN_AMBIENT_AIR_TEMP);
    data->cab_interior_temp = Pattern_GetValue(SPN_CAB_INTERIOR_TEMP);
    data->turbo_boost_pressure = Pattern_GetValue(SPN_TURBO_BOOST_PRESSURE);
    data->scr_dosing_rate = Pattern_GetValue(SPN_SCR_DOSING_RATE1);
    data->fan_speed = Pattern_GetValue(SPN_FAN_SPEED);
    data->brake_pedal_position = Pattern_GetValue(SPN_BRAKE_PEDAL_POSITION);
    data->steering_angle = Pattern_GetValue(SPN_STEERING_ANGLE);

    // Simulate engine fuel rate based on engine speed and load
    data->engine_fuel_rate = data->engine_speed * data->engine_percent_load / 1000.0f;

    // Simulate transmission oil temp based on engine temp
    data->transmission_oil_temp = data->engine_oil_temp * 0.9f;

    // Simulate SCR tank level decreasing over time
    static float scr_tank_level = 100.0f;
    scr_tank_level -= data->scr_dosing_rate / 360000.0f; // Convert g/h to % per ms
    if (scr_tank_level < 0) scr_tank_level = 100.0f;
    data->scr_tank_level = scr_tank_level;

    // Simulate SCR tank temperature
    data->scr_tank_temp = data->ambient_air_temp + 5.0f;

    // Simulate engine fuel pressure
    data->engine_fuel_pressure = 300.0f + data->engine_speed * 0.01f;

    // Simulate engine fuel temperature
    data->engine_fuel_temp = data->ambient_air_temp + 10.0f;

    // Simulate transmission oil pressure
    data->transmission_oil_pressure = 200.0f + data->engine_speed * 0.02f;

    // Simulate alternator current
    data->alternator_current = data->battery_voltage * 2.0f - 40.0f;

    // Simulate net battery current
    data->net_battery_current = data->alternator_current - 10.0f;

    // Simulate retarder torque
    data->retarder_torque = data->brake_pedal_position * 0.5f;

    // Simulate brake application pressure
    data->brake_application_pressure = data->brake_pedal_position * 50.0f;

    // Simulate yaw rate based on steering
    data->yaw_rate = data->steering_angle * 0.5f;

    // Simulate lateral acceleration
    data->lateral_acceleration = data->steering_angle * 2.0f;

    // Simulate longitudinal acceleration
    float accel = data->accelerator_position / 100.0f;
    float brake = data->brake_pedal_position / 100.0f;
    data->longitudinal_acceleration = (accel - brake) * 2.0f;

    // Simulate road surface temperature
    data->road_surface_temp = data->ambient_air_temp + 5.0f;

    // Simulate barometric pressure
    data->barometric_pressure = 101.3f + sinf(HAL_GetTick() / 10000.0f) * 2.0f;

    // Simulate washer fluid level (slow decrease)
    static float washer_level = 100.0f;
    washer_level -= 0.001f;
    if (washer_level < 0) washer_level = 100.0f;
    data->washer_fluid_level = washer_level;

    // Simulate brake lining wear (slow decrease)
    static float brake_lining = 100.0f;
    brake_lining -= data->brake_pedal_position * 0.0001f;
    if (brake_lining < 0) brake_lining = 100.0f;
    data->brake_lining_front_left = brake_lining;
    data->brake_lining_front_right = brake_lining;
    data->brake_lining_rear_left = brake_lining * 0.9f;
    data->brake_lining_rear_right = brake_lining * 0.9f;

    // Simulate DPF parameters
    data->dpf_inlet_pressure = data->turbo_boost_pressure * 0.8f;
    data->engine_exhaust_temp = data->engine_coolant_temp + 300.0f;
    data->dpf_outlet_temp = data->engine_exhaust_temp;

    // Simulate turbo speed
    data->turbo_speed = data->engine_speed * 12.0f;

    // Simulate intake parameters
    data->intake_air_temp = data->ambient_air_temp + 10.0f;
    data->intake_manifold_pressure = data->turbo_boost_pressure * 0.9f;

    // Simulate GPS coordinates (moving in a circle)
    static float gps_angle = 0.0f;
    gps_angle += 0.001f;
    data->latitude = 25.0330f + 0.01f * sinf(gps_angle);
    data->longitude = 121.5654f + 0.01f * cosf(gps_angle);
    data->compass_bearing = fmodf(gps_angle * 180.0f / PI, 360.0f);
    data->pitch = sinf(gps_angle * 2.0f) * 5.0f;

    // Update status indicators
    static uint32_t status_timer = 0;
    if (HAL_GetTick() - status_timer > 1000) {
        status_timer = HAL_GetTick();
        data->seat_belt_buckled = !data->seat_belt_buckled;
        data->turn_signal_left = (rand() % 10) > 7;
        data->turn_signal_right = (rand() % 10) > 7;
        data->hazard_lights = (rand() % 10) > 8;
        data->high_beam = (rand() % 10) > 8;
        data->water_in_fuel = (rand() % 100) > 95;
        data->abs_offroad_active = (rand() % 10) > 8;
    }

    // Update trip information
    data->trip_distance += data->vehicle_speed / 3600.0f; // km per ms
    data->total_distance = data->trip_distance * 1.1f;
    data->trip_fuel += data->engine_fuel_rate / 3600000.0f; // L per ms
    data->total_fuel = data->trip_fuel * 10.0f;
    data->trip_idle_time += (data->vehicle_speed < 1.0f) ? 0.001f : 0.0f;
    data->trip_engine_runtime += 0.001f;

    // Now update the CAN data buffers
    // Engine Info PGN (SPN 190 - Engine Speed)
    spn_def = J1939_FindSPN(SPN_ENGINE_SPEED);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_engine_info_data, data->engine_speed);
    }

    // Vehicle Speed PGN (SPN 84)
    spn_def = J1939_FindSPN(SPN_VEHICLE_SPEED);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_vehicle_speed_data, data->vehicle_speed);
    }

    // Engine Temperature PGN (SPN 110)
    spn_def = J1939_FindSPN(SPN_ENGINE_COOLANT_TEMP);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_engine_temp_data, data->engine_coolant_temp);
    }

    // Engine Oil Temperature (SPN 175)
    spn_def = J1939_FindSPN(SPN_ENGINE_OIL_TEMP);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_engine_temp_data, data->engine_oil_temp);
    }

    // Battery Voltage (SPN 167)
    spn_def = J1939_FindSPN(SPN_BATTERY_VOLTAGE);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_electrical_data, data->battery_voltage);
    }

    // Fuel Level (SPN 96)
    spn_def = J1939_FindSPN(SPN_FUEL_LEVEL1);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_washer_fuel_data, data->fuel_level);
    }

    // Accelerator Position (SPN 91)
    spn_def = J1939_FindSPN(SPN_ACCELERATOR_POSITION1);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_accelerator_data, data->accelerator_position);
    }

    // Transmission Gear (SPN 523)
    spn_def = J1939_FindSPN(SPN_TRANSMISSION_GEAR);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_transmission_gear_data, (float)data->transmission_gear);
    }

    // Ambient Air Temperature (SPN 171)
    spn_def = J1939_FindSPN(SPN_AMBIENT_AIR_TEMP);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_ambient_data, data->ambient_air_temp);
    }

    // Cab Interior Temperature (SPN 170)
    spn_def = J1939_FindSPN(SPN_CAB_INTERIOR_TEMP);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_ambient_data, data->cab_interior_temp);
    }

    // Turbo Boost Pressure (SPN 1127)
    spn_def = J1939_FindSPN(SPN_TURBO_BOOST_PRESSURE);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_turbo_boost_data, data->turbo_boost_pressure);
    }

    // SCR Dosing Rate (SPN 4331)
    spn_def = J1939_FindSPN(SPN_SCR_DOSING_RATE1);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_scr_dosing_data, data->scr_dosing_rate);
    }

    // Fan Speed (SPN 1639)
    spn_def = J1939_FindSPN(SPN_FAN_SPEED);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_fan_info_data, data->fan_speed);
    }

    // Brake Pedal Position (SPN 521)
    spn_def = J1939_FindSPN(SPN_BRAKE_PEDAL_POSITION);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_brake_abs_data, data->brake_pedal_position);
    }

    // Steering Angle (SPN 1807)
    spn_def = J1939_FindSPN(SPN_STEERING_ANGLE);
    if (spn_def) {
        J1939_SetSPNValue(spn_def, pgn_vehicle_dynamics_data, data->steering_angle);
    }

    // Note: You would need to add similar updates for all other SPNs
    // This is just a subset for demonstration
}

// Update display/status
static void J1939_Update_Display(void) {
    static uint32_t last_display_update = 0;
    uint32_t current_time = HAL_GetTick();

    // Update status every second
    if (current_time - last_display_update > 1000) {
        last_display_update = current_time;

        // Blink LED to show activity
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);

        // You could add serial output here for debugging:
        // printf("Test: %s, Frames: %lu, Remaining: %lu s\r\n",
        //        test_running ? "Running" : "Stopped",
        //        Pattern_GetFramesGenerated(),
        //        Pattern_GetRemainingTimeMs() / 1000);
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    // Rapid LED blinking on error
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
    HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
