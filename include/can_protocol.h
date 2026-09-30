#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include <stdint.h>


#define PIPE_NAME    "\\\\.\\pipe\\SensorBus"

// Standard CAN 2.0B payload 
#define PAYLOAD_SIZE 8

// Diagnostic Status Flags (Byte 6 of payload) 
typedef enum {
    STATUS_OK            = 0x00,
    STATUS_OUT_OF_BOUNDS = 0x01,
    STATUS_FRAME_CORRUPT = 0x02
} SensorStatus;

#endif