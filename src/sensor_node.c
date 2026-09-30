#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>
#include "can_protocol.h"

typedef struct {
    uint32_t timestamp_ms;
    float raw_value;
    float filtered_value;
}Sensordata;

float ema_filter(float current_raw,float previous_filtered,float alpha)
{
    return(alpha * current_raw + (1.0f - alpha) * previous_filtered);
}

float noise(float max_noise)
{
    float normalized = ((float)(rand()%1000) / 500.0f)-1.0f;
    return normalized * max_noise;
}

void pack_frame(uint32_t timestamp, float filtered_val, uint8_t status, uint8_t counter, uint8_t payload[8])
{
    payload[0]=(uint8_t)(timestamp & 0xFF);
    payload[1]=(uint8_t)((timestamp >> 8) & 0xFF);
    payload[2]=(uint8_t)((timestamp >> 16) & 0xFF);
    payload[3]=(uint8_t)((timestamp >> 24) & 0xFF);

    uint16_t scaled_val = (uint16_t)(filtered_val * 100.0f); //scale to 2 decimal places to efficiently store in can payload
    //stored values as decimals take lot of space

    //scaled value more into upper bytes and lower bytes
    payload[4]=(uint8_t)(scaled_val & 0xFF); //lower
    payload[5]=(uint8_t)((scaled_val >> 8) & 0xFF);//upper

    payload[6] = status; // status check 0x00 is ok
    payload[7] = (uint8_t)(counter & 0xFF); //rolling counter 0 to 15
    //rolling counter to check whether CAN communication is working properly or not
}

/*void unpack_frame(const uint8_t payload[8], uint32_t *out_timestamp, float *out_val, uint8_t *out_counter)
{//rebuilding timestamp by proper shifting
    *out_timestamp = ((uint32_t)payload[0] | ((uint32_t)payload[1] << 8) | ((uint32_t)payload[2] << 16) | ((uint32_t)payload[3] << 24));
    //rebuilding scaled value by proper shifting
    uint16_t raw_int = ((uint16_t)payload[4] | ((uint16_t)payload[5] << 8));
    //restoring to original float value by dividing by 100.0f
    *out_val = (float)raw_int / 100.0f;
    // rolling counter
    *out_counter = payload[7] & 0x0F; //masking to get only lower 4 bits
}*/

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);//set buffer for stream to teriminal output to avoid buffering issues INPUT OUTPUT NO BUFFERING
    //baseline readings
    const float baseline = 25.0f;
    const float alpha = 0.15f; //15% new sample weight 85 % previous sample weight
    const float noise_amplitude = 2.0f; //+- 2% fluctuation

    Sensordata sensor={
        .timestamp_ms=0,
        .raw_value=baseline,
        .filtered_value=baseline
    };
    uint8_t payload[PAYLOAD_SIZE];
    uint8_t counter=0;
    DWORD bytes_written=0; //variable created and set to    

    // 2. Connect to the pipe created by vcu_logger
    HANDLE hPipe = CreateFileA(
        PIPE_NAME,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,//find exisiting pipline and open it for writing
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Cannot connect to pipe! (Run vcu_logger.exe first)\n");
        return 1;
    }

    // 3. Transmission loop 
    for (int i = 0; i < 40; i++)
    {
        sensor.timestamp_ms += 100;
        sensor.raw_value = baseline + noise(noise_amplitude);
         uint8_t status = 0x00; //status check 0x00 is ok
         if(i==15)
        {
            sensor.raw_value=125.0f; //simulate a spike in temperature;
            status=0x01; 
            printf("Out of Bound Spinke in temperature detected at %d ms\n",sensor.timestamp_ms);
        }
        

        sensor.filtered_value = ema_filter(sensor.raw_value, sensor.filtered_value, alpha);

        if(i==25)
        {
            counter = (counter+2)& 0x0F; //simulate a missing frame by skipping a counter value
            printf("Dropping Counter Sequence");
        }

        /* Pack the 8 bytes */
        pack_frame(sensor.timestamp_ms, sensor.filtered_value, status, counter, payload);

        /* Transmit 8 bytes through the pipe */
        WriteFile(hPipe, payload, PAYLOAD_SIZE, &bytes_written, NULL);

        printf("[TX] T:%5u ms | Raw:%5.2f | Filt:%5.2f | Cnt:%2u | Bytes: %02X %02X %02X %02X %02X %02X %02X %02X\n",
               sensor.timestamp_ms, sensor.raw_value, sensor.filtered_value, counter,
               payload[0], payload[1], payload[2], payload[3],
               payload[4], payload[5], payload[6], payload[7]);

        /* 4. Bitwise AND (&) instead of logical AND (&&) */
        counter = (counter + 1) & 0x0F;

        /* 5. 100ms real-time delay (10 Hz rate) */
        Sleep(100);
    }

    printf("\n[COMPLETED] 40 frames sent. Closing pipe.\n");
    CloseHandle(hPipe);
    return 0;
}