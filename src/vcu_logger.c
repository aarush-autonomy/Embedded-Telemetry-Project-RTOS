#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "can_protocol.h"

void unpack_frame(const uint8_t payload[8], uint32_t *timestamp, float *value, uint8_t *status, uint8_t *counter)
{
    /* 1. Rebuild 32-bit timestamp */
    *timestamp = ((uint32_t)payload[0])       |
                 ((uint32_t)payload[1] << 8)  |
                 ((uint32_t)payload[2] << 16) |
                 ((uint32_t)payload[3] << 24);
    
    //rebuild scaled integer value from payload
    uint16_t scaled_int = ((uint16_t)payload[4]) | ((uint16_t)payload[5] << 8);
    *value=(float)scaled_int / 100.0f;//restore to original float value by dividing by 100.0f

    //extracting status and rolling counter
    *status=payload[6];
    *counter=payload[7] & 0x0F; //masking to
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    //creating a NAMED_PIPE to receive data from sensor_node
    HANDLE hPipe = CreateNamedPipeA(
        PIPE_NAME,
        PIPE_ACCESS_INBOUND, //read only pipe
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, //message type pipe, message-read mode, blocking mode
        1, //no of instances
        512, //output buffer size
        512, //input buffer size
        0, //default timeout
        NULL //default security attributes
    );


    if(hPipe == INVALID_HANDLE_VALUE)
    {
        printf("ERROR TO GET PIPE %lu\n",GetLastError());
        return 1;
    }

    // 2. Wait for sensor_node to connect
    ConnectNamedPipe(hPipe, NULL);
    printf("[CONNECTED] Streaming data...\n\n");
    printf(" Time(ms) | Value(C) | Status | Counter | Continuity\n");
    printf("----------+----------+--------+---------+------------\n");

    uint8_t payload[PAYLOAD_SIZE];
    DWORD bytes_read;
    int last_counter = -1;

    // Read 8 bytes at a time 
    while (ReadFile(hPipe, payload, PAYLOAD_SIZE, &bytes_read, NULL) && bytes_read == PAYLOAD_SIZE)
    {
        uint32_t timestamp = 0;
        float value = 0.0f;
        uint8_t status = 0; 
        uint8_t counter = 0;

        unpack_frame(payload, &timestamp, &value, &status, &counter);
        
        // Simple check of a missing frame
        int expected = (last_counter + 1) & 0x0F;
        if (status == 0x01) {
            printf("  %6u |   %6.2f |   0x%02X |   %2u    | SENSOR FAULT (OUT OF BOUNDS)\n", 
                   timestamp, value, status, counter);
        } else if (last_counter != -1 && counter != expected) {
            printf("  %6u |   %6.2f |   0x%02X |   %2u    | MISSED FRAME\n", 
                   timestamp, value, status, counter);
        } else {
            printf("  %6u |   %6.2f |   0x%02X |   %2u    | OK\n", 
                   timestamp, value, status, counter);
        }   

        last_counter = counter;
    }

    printf("\nSensor node stopped. Closing pipe.\n");
    CloseHandle(hPipe);
    return 0;
}