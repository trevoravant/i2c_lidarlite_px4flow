/*

A simple script to get distance data from the LIDAR-Lite using a gumstix (tested on an Overo FireSTORM COM and Summit expansion board).

NOTES:

1) Most of the i2c communication commands were copied from the /include/linux/i2c-dev.h file in the i2c-tools linux package. This file is a
   good reference if other commands are to be implemented.

2) The LIDAR-Lite must be connected to the i2c-3 port using a logic level converter, as the gumstix logic levels are set to 1.8V and those of
   the LIDAR-Lite require 5V.

*/

#include <fcntl.h> // for open
#include <time.h> // for nanosleep, clock_gettime
//#include <linux/i2c-dev.h> /* for I2C_SLAVE */
//#include "i2c-dev.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/types.h> // for s32,...

/* definitions copied from the /include/linux/i2c-dev.h file in the i2c-tools linux package */
#define I2C_SMBUS_READ	1
#define I2C_SMBUS_WRITE	0
#define I2C_SMBUS_BYTE_DATA	    2
#define I2C_SMBUS_WORD_DATA	    3
#define I2C_SLAVE	0x0703
#define I2C_SMBUS	0x0720	/* SMBus-level access */
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

void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

/* read continuous distance measurements (and see how long after the write command the read takes) */
void continuousDistance(int file)
{
    struct timespec t_start={0,0}, t_end={0,0};
    struct timespec req1, rem1;
    req1.tv_sec = 0;
    req1.tv_nsec = 1*1000*1000;  // 1 milliseconds
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
            ioctl(file, I2C_SLAVE, 0x62);
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
    printf("file is %d\n", file);
    
    /* tell the driver we want the device with 7-bit address 0x62 */
    if (ioctl(file, I2C_SLAVE, 0x62) < 0) {
        fprintf(stderr, "Failed to set slave address: %m\n");
        close(file);
        return 2;
    }
    printf("set i2c device using ioctl\n");

    unsigned char reg_w, val_w, reg_r1, reg_r2, reg_r3;
    int read_1, read_2, read_3, read_4;
    reg_w = 0x00;
    val_w = 0x04;
    reg_r1 = 0x0f;
    reg_r2 = 0x10;
    reg_r3 = 0x8f;

    if (i2c_smbus_write_byte_data(file, reg_w, val_w) == -1) {
        printf("smbus write failed\n");
        close(file);
        return 1;
    }

    struct timespec request, remain;
    request.tv_sec = 0;
    request.tv_nsec = 20*1000*1000; // 20 milliseconds
    nanosleep(&request, &remain); // from the LIDAR-Lite documentation, wait 20ms before read
    
    // get upper 8 bytes of distance
    read_1 = i2c_smbus_read_byte_data(file, reg_r1);
    if (read_1 == -1) {
        printf("smbus read 1 failed\n");
        close(file);
        return 2;
    }
    printf("read 1 returned 0x%x = %d = ", read_1, read_1);
    printBits(sizeof(read_1), &read_1);
    
    // get lower 8 bytes of distance
    read_2 = i2c_smbus_read_byte_data(file, reg_r2);
    if (read_2 == -1) {
        printf("smbus read 2 failed\n");
        close(file);
        return 3;
    }
    printf("read 2 returned 0x%x = %d = ", read_2, read_2);
    printBits(sizeof(read_2), &read_2);
    
    /* read_3 below will NOT work correctly since open and ioctl need to be re-called,
       read_4 will work because open and ioctl are re-called */
    read_3 = i2c_smbus_read_word_data(file, reg_r3);
    if (read_3 == -1) {
        printf("smbus read 3 failed\n");
        close(file);
        return 4;
    }
    
    /* get both (upper and lower) bytes of distance
       note:  we need to re-call BOTH open and ioctl to get this register value */ 
    file = open("/dev/i2c-3", O_RDWR);
    ioctl(file, I2C_SLAVE, 0x62);
    read_4 = i2c_smbus_read_word_data(file, reg_r3);
    if (read_4 == -1) {
        printf("smbus read 4 failed\n");
        close(file);
        return 5;
    }
    printf("read 3 returned 0x%x = %i = ", read_3, read_3);
    printBits(sizeof(read_3), &read_3);
    printf("read 4 returned 0x%x = %i = ", read_4, read_4);
    printBits(sizeof(read_4), &read_4);
    int distance = read_1<<8 | read_2; // form distance value from the upper and lower bytes
    float distance_in = distance/2.54; // distance in inches
    printf("distance is %d cm (%.1f in)\n", distance, distance_in);

    continuousDistance(file);

    close(file);

    return 0;
}
