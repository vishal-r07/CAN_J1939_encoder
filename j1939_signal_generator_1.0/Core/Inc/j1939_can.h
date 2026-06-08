/*
 * j1939_can.h
 *
 *  Created on: Dec 8, 2025
 *      Author: abian
 */

#ifndef J1939_CAN_H
#define J1939_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "j1939_pgn_spn.h"

// J1939 CAN ID structure
typedef union {
    struct {
        uint32_t source_address : 8;
        uint32_t pdu_format : 8;
        uint32_t data_page : 1;
        uint32_t reserved : 1;
        uint32_t pdu_specific : 8;
        uint32_t priority : 3;
    } bits;
    uint32_t value;
} J1939_ID_t;

// J1939 Message structure
typedef struct {
    J1939_ID_t id;
    uint8_t data[8];
    uint8_t data_length;
    uint32_t timestamp;
} J1939_Message_t;

// Timing structure for periodic transmission
typedef struct {
    uint32_t pgn;
    uint32_t period_ms;
    uint32_t last_tx_time;
    uint8_t* data;  // FIXED: Changed from uint8_t data[8] to pointer
    uint8_t data_length;
    uint8_t priority;
    uint8_t source_address;
} J1939_Periodic_TX_t;

// Function prototypes
void J1939_CAN_Init(CAN_HandleTypeDef* hcan);
bool J1939_CAN_Transmit(uint32_t pgn, uint8_t* data, uint8_t data_length, uint8_t priority, uint8_t source_address);
bool J1939_CAN_Receive(J1939_Message_t* msg);
void J1939_Process(void);
uint32_t J1939_Build_ID(uint8_t priority, uint32_t pgn, uint8_t source_address);
void J1939_Extract_ID(uint32_t can_id, J1939_ID_t* j1939_id);
uint32_t J1939_Get_PGN(uint32_t can_id);
uint8_t J1939_Get_Source_Address(uint32_t can_id);

// Periodic transmission management
void J1939_Add_Periodic_TX(J1939_Periodic_TX_t* tx);
void J1939_Update_Periodic_TX(void);
void J1939_Send_PGN(uint32_t pgn);

#ifdef __cplusplus
}
#endif

#endif // J1939_CAN_H
