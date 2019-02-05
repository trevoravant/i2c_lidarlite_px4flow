/*

A script to read data from the px4flow using a gumstix (tested on an Overo FireSTORM COM and Summit expansion board).


NOTES:

1. If any of the ncurses functionality is to be used in this program, then compile with "gcc -o PX PX.c -lncurses".

2. Due to the lack of structure packing of the i2c_integral_frame, 26 bytes must be read from the px4flow device to receive
   the integral frame, even though the i2c_integral_frame only consists of 25 bytes. For more information, see the Arduino
   example linked to by the px4flow website.

3) The px4flow must be connected to the i2c-3 port using a logic level converter, as the gumstix logic levels are set to 1.8V and those of
   the px4flow require 5V.

4) Most of the I2C communication commands were copied from the /include/linux/i2c-dev.h file in the i2c-tools linux package. This file is a
   good reference if other commands are to be implemented.

*/

#include <fcntl.h> 		// for open
#include <time.h> 		// for nanosleep,...
#include <sys/ioctl.h> 		// for ioctl
#include <inttypes.h> 		// for uint8_t,uint16_t,...
#include <ncurses.h> 		// for screen update stuff
#include <linux/types.h>	// for __u16

/* copied from the /include/linux/i2c-dev.h file in the i2c-tools linux package */
/*
 * I2C Message - used for pure i2c transaction, also from /dev interface
 */
struct i2c_msg {
	__u16 addr;	/* slave address			*/
	unsigned short flags;		
#define I2C_M_TEN		0x10	/* we have a ten bit chip address	*/
#define I2C_M_RD		0x01
#define I2C_M_NOSTART		0x4000
#define I2C_M_REV_DIR_ADDR	0x2000
#define I2C_M_IGNORE_NAK	0x1000
#define I2C_M_NO_RD_ACK		0x0800
	short len;		/* msg length				*/
	char *buf;		/* pointer to msg data			*/
};
#define I2C_M_RD	0x01
#define I2C_RDWR	0x0707	/* Combined R/W transfer (one stop only)*/
/* This is the structure as used in the I2C_RDWR ioctl call */
struct i2c_rdwr_ioctl_data {
	struct i2c_msg *msgs;	/* pointers to i2c_msgs */
	int nmsgs;		/* number of i2c_msgs */
};

#define PX4FLOW_ADDR    0x42    // i2c address of the px4flow device
#define FRAME_BYTES     22      // number of bytes in the i2c_frame
#define IFRAME_BYTES    25      // number of bytes in the i2c_integral_frame

/* i2c_frame data packet, defined in the PX4Flow documentation */
typedef struct i2c_frame
{
    uint16_t frame_count;// counts created I2C frames [#frames]
    int16_t pixel_flow_x_sum;// latest x flow measurement in pixels*10 [pixels]
    int16_t pixel_flow_y_sum;// latest y flow measurement in pixels*10 [pixels]
    int16_t flow_comp_m_x;// x velocity*1000 [meters/sec]
    int16_t flow_comp_m_y;// y velocity*1000 [meters/sec]
    int16_t qual;// Optical flow quality / confidence [0: bad, 255: maximum quality]
    int16_t gyro_x_rate; // latest gyro x rate [rad/sec]
    int16_t gyro_y_rate; // latest gyro y rate [rad/sec]
    int16_t gyro_z_rate; // latest gyro z rate [rad/sec]
    uint8_t gyro_range; // gyro range [0 .. 7] equals [50 deg/sec .. 2000 deg/sec] 
    uint8_t sonar_timestamp;// time since last sonar update [milliseconds]
    int16_t ground_distance;// Ground distance in meters*1000 [meters]. Positive value: distance known. Negative value: Unknown distance
} i2c_frame;

/* i2c_integral_frame data packet, defined in the PX4Flow documentation */
typedef struct i2c_integral_frame
{
    uint16_t frame_count_since_last_readout;//number of flow measurements since last I2C readout [#frames]
    int16_t pixel_flow_x_integral;//accumulated flow in radians*10000 around x axis since last I2C readout [rad*10000]
    int16_t pixel_flow_y_integral;//accumulated flow in radians*10000 around y axis since last I2C readout [rad*10000]
    int16_t gyro_x_rate_integral;//accumulated gyro x rates in radians*10000 since last I2C readout [rad*10000]
    int16_t gyro_y_rate_integral;//accumulated gyro y rates in radians*10000 since last I2C readout [rad*10000]
    int16_t gyro_z_rate_integral;//accumulated gyro z rates in radians*10000 since last I2C readout [rad*10000]
    uint32_t integration_timespan;//accumulation timespan in microseconds since last I2C readout [microseconds]
    uint32_t sonar_timestamp;// time since last sonar update [microseconds]
    int16_t ground_distance;// Ground distance in meters*1000 [meters*1000]
    int16_t gyro_temperature;// Temperature * 100 in centi-degrees Celsius [degcelsius*100]
    uint8_t quality;// averaged quality of accumulated flow values [0:bad quality;255: max quality]
} __attribute__((packed)) i2c_integral_frame;

/* write one byte to and then read read_length bytes from the px4flow */
int i2c_wr_px4flow(int file, uint8_t write_data[], uint8_t read_data[], uint8_t read_length)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    /* write */
    messages[0].addr  = PX4FLOW_ADDR;
    messages[0].flags = 0;  // 0 indicates this message is to be written
    messages[0].len   = 1;
    messages[0].buf   = write_data;

    /* read */
    messages[1].addr  = PX4FLOW_ADDR;
    messages[1].flags = I2C_M_RD; // I2C_M_RD indicates this message is to be read
    messages[1].len   = read_length;
    messages[1].buf   = read_data;

    packets.msgs      = messages;
    packets.nmsgs     = 2;

    return ioctl(file, I2C_RDWR, &packets);
}

/* put the 22 bytes of the read_read_data array into a px4flow frame */
void fillFrame(uint8_t read_data[FRAME_BYTES], i2c_frame *frame_ptr)
{
    frame_ptr->frame_count = read_data[0] | (read_data[1]<<8);
    frame_ptr->pixel_flow_x_sum = (int16_t) (read_data[2] | (read_data[3]<<8));
    frame_ptr->pixel_flow_y_sum = (int16_t) (read_data[4] | (read_data[5]<<8));
    frame_ptr->flow_comp_m_x = (int16_t) (read_data[6] | (read_data[7]<<8));
    frame_ptr->flow_comp_m_y = (int16_t) (read_data[8] | (read_data[9]<<8));
    frame_ptr->qual = (int16_t) (read_data[10] | (read_data[11]<<8));
    frame_ptr->gyro_x_rate = (int16_t) (read_data[12] | (read_data[13]<<8));
    frame_ptr->gyro_y_rate = (int16_t) (read_data[14] | (read_data[15]<<8));
    frame_ptr->gyro_z_rate = (int16_t) (read_data[16] | (read_data[17]<<8));
    frame_ptr->gyro_range = read_data[18];
    frame_ptr->sonar_timestamp = read_data[19];
    frame_ptr->ground_distance = (int16_t) (read_data[20] | (read_data[21]<<8));
}

/* put the 25 bytes of the read_read_data array into a px4flow integral frame */
void fillIntegralFrame(uint8_t read_data[IFRAME_BYTES+1], i2c_integral_frame *frame_ptr)
{
    frame_ptr->frame_count_since_last_readout = read_data[0] | (read_data[1]<<8);
    frame_ptr->pixel_flow_x_integral = (int16_t) (read_data[2] | (read_data[3]<<8));
    frame_ptr->pixel_flow_y_integral = (int16_t) (read_data[4] | (read_data[5]<<8));
    frame_ptr->gyro_x_rate_integral = (int16_t) (read_data[6] | (read_data[7]<<8));
    frame_ptr->gyro_y_rate_integral = (int16_t) (read_data[8] | (read_data[9]<<8));
    frame_ptr->gyro_z_rate_integral = (int16_t) (read_data[10] | (read_data[11]<<8));
    frame_ptr->integration_timespan = read_data[12] | (read_data[13]<<8) | (read_data[14]<<16) | (read_data[15]<<24);
    frame_ptr->sonar_timestamp = read_data[16] | (read_data[17]<<8) | (read_data[18]<<16) | (read_data[19]<<24);
    frame_ptr->ground_distance = (int16_t) (read_data[20] | (read_data[21]<<8));
    frame_ptr->gyro_temperature = (int16_t) (read_data[22] | (read_data[23]<<8));
    frame_ptr->quality = read_data[24];
}

/* read px4flow frames over and over again */
int autoReadFrame(int file)
{
    int c = 0;
    uint8_t write_data[1] = {0x00}; // write 0x00 to the px4flow to receive a frame
    uint8_t read_data[FRAME_BYTES];

    struct timespec request, remain;
    request.tv_sec = 0;
    request.tv_nsec = 500*1000*1000;
    i2c_frame frame;
    initscr();
    curs_set(0);
    while (c < 1000) {
        if (i2c_wr_px4flow(file, write_data, read_data, FRAME_BYTES) < 0) { // note: passing "data" to a function is the same as passing &data[0]
            break;
        }

        fillFrame(read_data, &frame);

        erase();
        mvprintw(0, 0, "frame_count");
        mvprintw(1, 0, "%u", frame.frame_count);
        mvprintw(0, 20, "pixel_flow_x_sum");
        mvprintw(1, 20, "%d", frame.pixel_flow_x_sum);
        mvprintw(0, 40, "pixel_flow_y_sum");
        mvprintw(1, 40, "%d", frame.pixel_flow_y_sum);
        mvprintw(0, 60, "flow_comp_m_x");
        mvprintw(1, 60, "%d", frame.flow_comp_m_x);
        mvprintw(0, 80, "flow_comp_m_y");
        mvprintw(1, 80, "%d", frame.flow_comp_m_y);
        mvprintw(0, 100, "qual");
        mvprintw(1, 100, "%d", frame.qual);
        mvprintw(3, 0, "gyro_x_rate");
        mvprintw(4, 0, "%d", frame.gyro_x_rate);
        mvprintw(3, 20, "gyro_y_rate");
        mvprintw(4, 20, "%d", frame.gyro_y_rate);
        mvprintw(3, 40, "gyro_z_rate");
        mvprintw(4, 40, "%d", frame.gyro_z_rate);
        mvprintw(3, 60, "gyro_range");
        mvprintw(4, 60, "%u", frame.gyro_range);
        mvprintw(3, 80, "sonar_timestamp");
        mvprintw(4, 80, "%u", frame.sonar_timestamp);
        mvprintw(3, 100, "ground_distance");
        mvprintw(4, 100, "%d", frame.ground_distance);
        mvprintw(6, 0, "LOOP COUNT: %d", c);

        refresh();
        nanosleep(&request, &remain);
        c++;
    }
    endwin();

    return 0;
}

/* read px4flow integral frames over and over again */
int autoReadIntegralFrame(int file)
{
    int c = 0;
    uint8_t write_data[1] = {0x16}; // write 0x16 to the px4flow to receive an integral frame
    uint8_t read_data[IFRAME_BYTES+1];

    struct timespec request, remain;
    request.tv_sec = 0;
    request.tv_nsec = 500*1000*1000;
    i2c_integral_frame iframe;
    initscr();
    curs_set(0);
    while (c < 1000) {
        if (i2c_wr_px4flow(file, write_data, read_data, IFRAME_BYTES+1) < 0) { // note: passing "data" to a function is the same as passing &data[0]
            break;
        }

        fillIntegralFrame(read_data, &iframe);

        erase();
        mvprintw(0, 0, "frame_count_since_");
        mvprintw(1, 0, "last_readout");
        mvprintw(2, 0, "%u", iframe.frame_count_since_last_readout);
        mvprintw(0, 20, "pixel_flow_");
        mvprintw(1, 20, "x_integral");
        mvprintw(2, 20, "%d", iframe.pixel_flow_x_integral);
        mvprintw(0, 40, "pixel_flow_");
        mvprintw(1, 40, "y_integral");
        mvprintw(2, 40, "%d", iframe.pixel_flow_y_integral);
        mvprintw(0, 60, "gyro_x_rate_");
        mvprintw(1, 60, "integral");
        mvprintw(2, 60, "%d", iframe.gyro_x_rate_integral);
        mvprintw(0, 80, "gyro_y_rate_");
        mvprintw(1, 80, "integral");
        mvprintw(2, 80, "%d", iframe.gyro_y_rate_integral);
        mvprintw(0, 100, "gyro_z_rate_");
        mvprintw(1, 100, "integral");
        mvprintw(2, 100, "%d", iframe.gyro_z_rate_integral);
        mvprintw(4, 0, "integration_timespan");
        mvprintw(5, 0, "%d", iframe.integration_timespan);
        mvprintw(4, 30, "sonar_timestamp");
        mvprintw(5, 30, "%d", iframe.sonar_timestamp);
        mvprintw(4, 50, "ground_distance");
        mvprintw(5, 50, "%d", iframe.ground_distance);
        mvprintw(4, 70, "gyro_temperature");
        mvprintw(5, 70, "%u", iframe.gyro_temperature);
        mvprintw(4, 90, "quality");
        mvprintw(5, 90, "%u", iframe.quality);
        mvprintw(7, 0, "LOOP COUNT: %d", c);

        refresh();
        nanosleep(&request, &remain);
        c++;
    }
    endwin();

    return 0;
}

/* read a px4flow frame one time */
int readOnceFrame(int file)
{
    uint8_t write_data[1] = {0x00};
    uint8_t read_data[FRAME_BYTES];
    i2c_frame frame;

    // note: passing "data" to a function is the same as passing "&data[0]"
    if (i2c_wr_px4flow(file, write_data, read_data, FRAME_BYTES) < 0) {
        printf("i2c write/read failed\n");
        return -1;
    }

    fillFrame(read_data, &frame);

    printf("frame_count is %u\n", frame.frame_count);
    printf("pixel_flow_x_sum is %d\n", frame.pixel_flow_x_sum);
    printf("pixel_flow_y_sum is %d\n", frame.pixel_flow_y_sum);
    printf("flow_comp_m_x is %d\n", frame.flow_comp_m_x);
    printf("flow_comp_m_y is %d\n", frame.flow_comp_m_y);
    printf("qual is %d\n", frame.qual);
    printf("gyro_x_rate is %d\n", frame.gyro_x_rate);
    printf("gyro_y_rate is %d\n", frame.gyro_y_rate);
    printf("gyro_z_rate is %d\n", frame.gyro_z_rate);
    printf("gyro_range is %u\n", frame.gyro_range);
    printf("sonar_timestamp is %u\n", frame.sonar_timestamp);
    printf("ground_distance is %d\n", frame.ground_distance);

    return 0;
}

/* read a px4flow integral frame one time */
int readOnceIntegralFrame(int file)
{
    uint8_t write_data[1] = {0x16};
    uint8_t read_data[IFRAME_BYTES+1];
    i2c_integral_frame iframe;

    // note: passing "data" to a function is the same as passing "&data[0]"
    if (i2c_wr_px4flow(file, write_data, read_data, IFRAME_BYTES+1) < 0) {
        printf("i2c write/read failed\n");
        return -1;
    }

    fillIntegralFrame(read_data, &iframe);

    printf("frame_count_since_last_readout is %u\n", iframe.frame_count_since_last_readout);
    printf("pixel_flow_x_integral is %d\n", iframe.pixel_flow_x_integral);
    printf("pixel_flow_x_integral is %d\n", iframe.pixel_flow_y_integral);
    printf("gyro_x_rate_integral is %d\n", iframe.gyro_x_rate_integral);
    printf("gyro_y_rate_integral is %d\n", iframe.gyro_y_rate_integral);
    printf("gyro_z_rate_integral is %d\n", iframe.gyro_z_rate_integral);
    printf("integration_timespan is %u\n", iframe.integration_timespan);
    printf("sonar_timestamp is %u\n", iframe.sonar_timestamp);
    printf("ground_distance is %d\n", iframe.ground_distance);
    printf("gyro_temperature is %d\n", iframe.gyro_temperature);
    printf("quality is %u\n", iframe.quality);

    return 0;
}

int main(int argc, char **argv)
{
    int file = open("/dev/i2c-3", O_RDWR);
    if (file < 0) {
        printf("could not open file\n");
        return -1;
    }
    autoReadFrame(file);
    //autoReadIntegralFrame(file);
    close(file);

    return 0;
}
