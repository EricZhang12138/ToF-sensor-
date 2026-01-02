/* platform.c - Linux I2C implementation for Nethermind Processor Board */
#include "platform.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdio.h>

// I2C Bus defined in your schematic (I2C7)
#define I2C_BUS "/dev/i2c-7"

int VL53L7CX_CommsInit(VL53L7CX_Platform *p_platform)
{
    // Open the I2C bus (I2C7)
    if ((p_platform->fd = open(I2C_BUS, O_RDWR)) < 0) {
        printf("Failed to open I2C bus %s\n", I2C_BUS);
        return -1;
    }

    // Set I2C target address (Default VL53L7CX address is 0x52 >> 1 = 0x29)
    if (ioctl(p_platform->fd, I2C_SLAVE, p_platform->address >> 1) < 0) {
        printf("Failed to acquire bus access/talk to slave\n");
        return -1;
    }
    return 0;
}

int VL53L7CX_CommsClose(VL53L7CX_Platform *p_platform)
{
    close(p_platform->fd);
    return 0;
}

int VL53L7CX_RdByte(VL53L7CX_Platform *p_platform, uint16_t RegisterAdress, uint8_t *p_value)
{
    uint8_t data_write[2];
    data_write[0] = (RegisterAdress >> 8) & 0xFF;
    data_write[1] = RegisterAdress & 0xFF;

    if (write(p_platform->fd, data_write, 2) != 2) return -1;
    if (read(p_platform->fd, p_value, 1) != 1) return -1;
    return 0;
}

int VL53L7CX_WrByte(VL53L7CX_Platform *p_platform, uint16_t RegisterAdress, uint8_t value)
{
    uint8_t data_write[3];
    data_write[0] = (RegisterAdress >> 8) & 0xFF;
    data_write[1] = RegisterAdress & 0xFF;
    data_write[2] = value;

    if (write(p_platform->fd, data_write, 3) != 3) return -1;
    return 0;
}

int VL53L7CX_RdMulti(VL53L7CX_Platform *p_platform, uint16_t RegisterAdress, uint8_t *p_values, uint32_t size)
{
    uint8_t data_write[2];
    data_write[0] = (RegisterAdress >> 8) & 0xFF;
    data_write[1] = RegisterAdress & 0xFF;

    if (write(p_platform->fd, data_write, 2) != 2) return -1;
    if (read(p_platform->fd, p_values, size) != size) return -1;
    return 0;
}

int VL53L7CX_WrMulti(VL53L7CX_Platform *p_platform, uint16_t RegisterAdress, uint8_t *p_values, uint32_t size)
{
    // Implementation requires combining address + data into single write buffer
    // or using i2c_msg structs for scattered write (simplified here)
    uint8_t buffer[size + 2];
    buffer[0] = (RegisterAdress >> 8) & 0xFF;
    buffer[1] = RegisterAdress & 0xFF;
    for(int i=0; i<size; i++) buffer[i+2] = p_values[i];
    
    if (write(p_platform->fd, buffer, size + 2) != (size + 2)) return -1;
    return 0;
}

int VL53L7CX_Reset_Sensor(VL53L7CX_Platform *p_platform)
{
    // Toggle GPIO4_A7 (Sysfs GPIO 135)
    // Note: It is often cleaner to export this via script before running the app
    system("echo 135 > /sys/class/gpio/export");
    system("echo out > /sys/class/gpio/gpio135/direction");
    
    // Toggle Reset: Low -> High
    system("echo 0 > /sys/class/gpio/gpio135/value");
    usleep(10000); // 10ms
    system("echo 1 > /sys/class/gpio/gpio135/value");
    usleep(10000); // 10ms
    
    return 0;
}

int VL53L7CX_SwapBuffer(VL53L7CX_Platform *p_platform, uint8_t *buffer, uint16_t size)
{
    uint32_t i;
    uint8_t tmp;
    for(i = 0; i < size; i = i + 4) {
        tmp = buffer[i];
        buffer[i] = buffer[i + 3];
        buffer[i + 3] = tmp;
        tmp = buffer[i + 1];
        buffer[i + 1] = buffer[i + 2];
        buffer[i + 2] = tmp;
    }
    return 0;
}

int VL53L7CX_WaitMs(VL53L7CX_Platform *p_platform, uint32_t TimeMs)
{
    usleep(TimeMs * 1000);
    return 0;
}
