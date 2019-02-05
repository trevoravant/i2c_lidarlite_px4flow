/*

A script to get distance data from the LIDAR-Lite using a gumstix (tested on an Overo FireSTORM COM and Summit expansion board).

NOTES:

1) The LIDAR-Lite must be connected to the i2c-3 port using a logic level converter, as the gumstix logic levels are set to 1.8V and those of
   the LIDAR-Lite require 5V.

2) Most of the i2c communication commands were copied from the /include/linux/i2c-dev.h file in the i2c-tools linux package. This file is a
   good reference if other commands are to be implemented.

*/

#include <fcntl.h> 		// for open
#include <time.h> 		// for nanosleep, clock_gettime
#include <stdio.h> 		// for stderr
#include <sys/ioctl.h> 		// for ioctl
#include <linux/types.h> 	// for s32,...

/* definitions copied from the /include/linux/i2c-dev.h file in the i2c-tools linux package */
#define I2C_SMBUS_READ		1
#define I2C_SMBUS_WRITE		0
#define I2C_SMBUS_BYTE_DATA	2
#define I2C_SMBUS_WORD_DATA	3
#define I2C_SLAVE		0x0703
#define I2C_SMBUS		0x0720	/* SMBus-level access */
#define I2C_SMBUS_BLOCK_MAX	32	/* As specified in SMBus standard */

union i2c_smbus_data {
    __u8 byte;
    __u16 word;
    __u8 block[I2C_SMBUS_BLOCK_MAX + 2]; /* block[0] is used for length */
                                                /* and one more for PEC */
};

/* This is the structure as used in the I2C_SMBUS ioctl call */
struct i2c_smbus_ioctl_data {
    char read_write;
    __u8 command;
    int size;
    union i2c_smbus_data *data;
};

#define LL_ADDR    0x62    // i2c address of the lidar-lite device

/* function copied from the /include/linux/i2c-dev.h file in the i2c-tools linux package */
static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command,
                                     int size, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args;

    args.read_write = read_write;
    args.command = command;
    args.size = size;
    args.data = data;
    return ioctl(file,I2C_SMBUS,&args);
}

/* function copied from the /include/linux/i2c-dev.h file in the i2c-tools linux package */
static inline __s32 i2c_smbus_read_byte_data(int file, __u8 command)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
                         I2C_SMBUS_BYTE_DATA,&data))
        return -1;
    else
        return 0x0FF & data.byte;
}

/* function copied from the /include/linux/i2c-dev.h file in the i2c-tools linux package */
static inline __s32 i2c_smbus_write_byte_data(int file, __u8 command,
                                              __u8 value)
{
    union i2c_smbus_data data;
    data.byte = value;
    return i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
                            I2C_SMBUS_BYTE_DATA, &data);
}

/* function copied from the /include/linux/i2c-dev.h file in the i2c-tools linux package */
static inline __s32 i2c_smbus_read_word_data(int file, __u8 command)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
                         I2C_SMBUS_WORD_DATA,&data))
        return -1;
    else
        return 0x0FFFF & data.word;
}

/* read continuous distance measurements (and see how long after the write command the read takes) */
void continuousDistance(int file)
{
    struct timespec t_start={0,0}, t_end={0,0};
    struct timespec req1, rem1;
    req1.tv_sec = 0;
    req1.tv_nsec = 1*1000*1000;  // 1 millisecond
    struct timespec req2, rem2;
    req2.tv_sec = 1;             // 1 second
    req2.tv_nsec = 0*1000*1000;

    unsigned char ack = 0; // flag: did the read command work?
    int c = 0;  // loop counter
    int read, distance;
    unsigned char lower, upper;
    long long int time_elapsed; // time after write command for read command to complete, in nanoseconds

    // loop over multiple write/read sequences
    while(1) {
        ack = 0;
        c = 0;

        // write command
        if (i2c_smbus_write_byte_data(file, 0x00, 0x04) == -1) {
            printf("smbus write failed\n");
            close(file);
            break;
        }
        clock_gettime(CLOCK_MONOTONIC, &t_start); // start the "stopwatch"

        // loop until read works
        while(!ack) {
            file = open("/dev/i2c-3", O_RDWR);
            ioctl(file, I2C_SLAVE, LL_ADDR);
            read = i2c_smbus_read_word_data(file, 0x8f);
            if (read == -1) {
                c++;
                nanosleep(&req1, &rem1); // wait 1ms
            }
            else {
                clock_gettime(CLOCK_MONOTONIC, &t_end); // stop the "stopwatch"
                time_elapsed = ((long long int)1.0e9*t_end.tv_sec + t_end.tv_nsec) -
                               ((long long int)1.0e9*t_start.tv_sec + t_start.tv_nsec);
                ack = 1;
                lower = (read>>8) & 0xFF;
                upper = read & 0xFF;
                distance = lower | (upper<<8);
		// print how long it took from write command to read command, how many loops passed from write command to read command, and distance
                printf("took %lli ns, waited %d loops, distance = %d cm\n", time_elapsed, c, distance);
            }
        }
        nanosleep(&req2, &rem2); // wait a sec
    }
}

int main(int argc, char **argv)
{
    int file;
    file = open("/dev/i2c-3", O_RDWR);
    /* tell the driver we want the device with lidar-lite's address */
    if (ioctl(file, I2C_SLAVE, LL_ADDR) < 0) {
        fprintf(stderr, "Failed to set slave address: %m\n");
        close(file);
        return 1;
    }
    continuousDistance(file);
    close(file);

    return 0;
}
