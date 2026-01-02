/* main.c - VL53L7CX Ranging Application */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vl53l7cx_api.h"
#include "platform.h"

int main(int argc, char *argv[])
{
    VL53L7CX_Configuration  Dev;
    VL53L7CX_ResultsData    Results;
    uint8_t status, isAlive, isReady, i;
    
    // 1. Initialize Platform (I2C7 and GPIO Reset)
    Dev.platform.address = 0x52; // Default address
    if (VL53L7CX_CommsInit(&Dev.platform) != 0) {
        printf("I2C Init Error\n");
        return -1;
    }

    // 2. Hardware Reset (Toggle GPIO4_A7)
    printf("Resetting Sensor on GPIO4_A7...\n");
    VL53L7CX_Reset_Sensor(&Dev.platform);

    // 3. Check connection
    status = vl53l7cx_is_alive(&Dev, &isAlive);
    if(!isAlive || status != VL53L7CX_STATUS_OK) {
        printf("Sensor not detected! Status: %u\n", status);
        return -1;
    }
    printf("Sensor Detected!\n");

    // 4. Initialize Sensor
    status = vl53l7cx_init(&Dev);
    if(status != VL53L7CX_STATUS_OK) {
        printf("Init failed\n");
        return -1;
    }

    // 5. Configure Settings
    // Set resolution to 4x4 (16 zones) for faster updates
    status = vl53l7cx_set_resolution(&Dev, VL53L7CX_RESOLUTION_4X4);
    // Set ranging frequency to 10Hz
    status = vl53l7cx_set_ranging_frequency_hz(&Dev, 10);
    // Set Ranging Mode to Continuous
    status = vl53l7cx_set_ranging_mode(&Dev, VL53L7CX_RANGING_MODE_CONTINUOUS);

    // 6. Start Ranging
    printf("Starting Ranging...\n");
    status = vl53l7cx_start_ranging(&Dev);

    // 7. Loop Loop
    while(1) {
        // Poll for data ready
        status = vl53l7cx_check_data_ready(&Dev, &isReady);
        
        if(isReady) {
            vl53l7cx_get_ranging_data(&Dev, &Results);

            // Clear screen and print 4x4 grid
            printf("\033[2J\033[H"); 
            printf("--- VL53L7CX 4x4 Distance (mm) ---\n");
            
            for(i = 0; i < 16; i++) {
                // Print distance of each zone
                printf("%4d ", Results.distance_mm[i]);
                if((i+1) % 4 == 0) printf("\n");
            }
            printf("\nTemperature: %d C\n", Results.silicon_temp_degc);
        }
        
        // Short delay to prevent CPU hogging
        VL53L7CX_WaitMs(&Dev.platform, 5); 
    }

    VL53L7CX_CommsClose(&Dev.platform);
    return 0;
}