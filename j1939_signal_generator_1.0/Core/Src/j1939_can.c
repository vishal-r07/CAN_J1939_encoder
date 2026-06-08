/*
 * j1939_can.c
 *
 *  Created on: Dec 8, 2025
 *      Author: abian
 */

#include "j1939_can.h"
#include <string.h>

static CAN_HandleTypeDef* can_handle = NULL;
static J1939_Periodic_TX_t* periodic_tx_list[20];
static uint8_t periodic_tx_count = 0;
static uint32_t system_time_ms = 0;

// Initialize J1939 CAN module
void J1939_CAN_Init(CAN_HandleTypeDef* hcan) {
    if (hcan == NULL) {
        return;
    }

    can_handle = hcan;

    // Start CAN in normal mode
    if (HAL_CAN_Start(can_handle) != HAL_OK) {
        // Error handling - you might want to add error callback
    }

    // Activate CAN notification
    if (HAL_CAN_ActivateNotification(can_handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        // Error handling
    }
}

// Build J1939 CAN ID from components
uint32_t J1939_Build_ID(uint8_t priority, uint32_t pgn, uint8_t source_address) {
    J1939_ID_t id;
    id.bits.priority = priority & 0x07;
    id.bits.reserved = 0;
    id.bits.data_page = 0;
    id.bits.pdu_format = (pgn >> 8) & 0xFF;

    if (id.bits.pdu_format >= 240) {
        // PDU2 format
        id.bits.pdu_specific = pgn & 0xFF;
    } else {
        // PDU1 format
        id.bits.pdu_specific = source_address;  // Destination address
    }

    id.bits.source_address = source_address;

    return id.value;
}

// Extract J1939 ID components from CAN ID
void J1939_Extract_ID(uint32_t can_id, J1939_ID_t* j1939_id) {
    if (j1939_id == NULL) {
        return;
    }
    j1939_id->value = can_id;
}

// Get PGN from CAN ID
uint32_t J1939_Get_PGN(uint32_t can_id) {
    J1939_ID_t id;
    J1939_Extract_ID(can_id, &id);

    uint32_t pgn = (id.bits.data_page << 16) | (id.bits.pdu_format << 8);

    if (id.bits.pdu_format < 240) {
        // PDU1 format
        pgn |= id.bits.pdu_specific;
    } else {
        // PDU2 format
        pgn |= id.bits.pdu_specific;
    }

    return pgn;
}

// Get source address from CAN ID
uint8_t J1939_Get_Source_Address(uint32_t can_id) {
    J1939_ID_t id;
    J1939_Extract_ID(can_id, &id);
    return id.bits.source_address;
}

// Transmit J1939 message
bool J1939_CAN_Transmit(uint32_t pgn, uint8_t* data, uint8_t data_length, uint8_t priority, uint8_t source_address) {
    if (can_handle == NULL || data == NULL || data_length > 8) {
        return false;
    }

    CAN_TxHeaderTypeDef tx_header;
    uint32_t mailbox;

    // Extract PDU Format from PGN
    uint8_t pdu_format = (pgn >> 8) & 0xFF;

    uint32_t can_id = 0;

    if (pdu_format >= 240) {
        // PDU2 format (Broadcast) - 29-bit Extended ID
        can_id = (priority << 26) |              // Priority (3 bits)
                 (0 << 25) |                     // Reserved
                 (0 << 24) |                     // Data Page
                 (pdu_format << 16) |            // PDU Format
                 ((pgn & 0xFF) << 8) |           // PDU Specific
                 source_address;                 // Source Address
    } else {
        // PDU1 format (Directed) - 29-bit Extended ID
        can_id = (priority << 26) |              // Priority (3 bits)
                 (0 << 25) |                     // Reserved
                 (0 << 24) |                     // Data Page
                 (pdu_format << 16) |            // PDU Format
                 (source_address << 8) |         // PDU Specific = Destination (for PDU1)
                 source_address;                 // Source Address
    }

    // Configure CAN TX header
    tx_header.StdId = 0;                         // Not used for Extended ID
    tx_header.ExtId = can_id;                    // 29-bit Extended ID
    tx_header.IDE = CAN_ID_EXT;                  // EXTENDED ID (29-bit) - CRITICAL for J1939!
    tx_header.RTR = CAN_RTR_DATA;                // Data frame, not remote
    tx_header.DLC = (data_length <= 8) ? data_length : 8;
    tx_header.TransmitGlobalTime = DISABLE;

    // Start transmission
    if (HAL_CAN_AddTxMessage(can_handle, &tx_header, data, &mailbox) != HAL_OK) {
        return false;
    }

    return true;
}

// Receive J1939 message
bool J1939_CAN_Receive(J1939_Message_t* msg) {
    if (can_handle == NULL || msg == NULL) {
        return false;
    }

    CAN_RxHeaderTypeDef rx_header;

    // Check if message is available in FIFO0
    if (HAL_CAN_GetRxFifoFillLevel(can_handle, CAN_RX_FIFO0) == 0) {
        return false;
    }

    // Read message from FIFO0
    if (HAL_CAN_GetRxMessage(can_handle, CAN_RX_FIFO0, &rx_header, msg->data) != HAL_OK) {
        return false;
    }

    // Extract J1939 information based on ID type (Standard or Extended)
    if (rx_header.IDE == CAN_ID_EXT) {
        // Extended ID (29-bit) - J1939 uses this
        msg->id.value = rx_header.ExtId;
    } else {
        // Standard ID (11-bit) - Not typical for J1939, but handle it
        msg->id.value = rx_header.StdId;
    }

    msg->data_length = rx_header.DLC;
    msg->timestamp = HAL_GetTick();

    return true;
}

// Add periodic transmission
void J1939_Add_Periodic_TX(J1939_Periodic_TX_t* tx) {
    if (tx == NULL || periodic_tx_count >= 20) {
        return;
    }

    periodic_tx_list[periodic_tx_count] = tx;
    periodic_tx_count++;
}

// Update periodic transmissions
void J1939_Update_Periodic_TX(void) {
    system_time_ms = HAL_GetTick();

    for (uint8_t i = 0; i < periodic_tx_count; i++) {
        J1939_Periodic_TX_t* tx = periodic_tx_list[i];
        if (tx == NULL) {
            continue;
        }

        if ((system_time_ms - tx->last_tx_time) >= tx->period_ms) {
            J1939_CAN_Transmit(tx->pgn, tx->data, tx->data_length, tx->priority, tx->source_address);
            tx->last_tx_time = system_time_ms;
        }
    }
}

// Send specific PGN
void J1939_Send_PGN(uint32_t pgn) {
    for (uint8_t i = 0; i < periodic_tx_count; i++) {
        J1939_Periodic_TX_t* tx = periodic_tx_list[i];
        if (tx != NULL && tx->pgn == pgn) {
            J1939_CAN_Transmit(tx->pgn,
                              tx->data,
                              tx->data_length,
                              tx->priority,
                              tx->source_address);
            tx->last_tx_time = system_time_ms;
            break;
        }
    }
}

// Process incoming messages
void J1939_Process(void) {
    J1939_Message_t msg;

    while (J1939_CAN_Receive(&msg)) {
        uint32_t pgn = J1939_Get_PGN(msg.id.value);
        uint8_t source = J1939_Get_Source_Address(msg.id.value);

        // Process based on PGN
        switch (pgn) {
            case PGN_ENGINE_TORQUE_REQUEST:
                // Process engine torque request
                // Example: msg.data[0] might contain torque percentage
                break;

            case PGN_VEHICLE_SPEED:
                // Process vehicle speed
                // Example: msg.data[0-1] might contain speed in 0.00390625 km/h resolution
                break;

            // Add more PGN handlers as needed

            default:
                // Unknown PGN - could log or ignore
                break;
        }
    }

    // Update periodic transmissions
    J1939_Update_Periodic_TX();
}
