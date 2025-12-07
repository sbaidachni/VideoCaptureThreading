#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <mavlink/common/mavlink.h>

int open_serial_port(const char* device, int baud_rate) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open_serial_port: Unable to open");
        return -1;
    }

    // Clear flags
    fcntl(fd, F_SETFL, 0);

    struct termios options;
    tcgetattr(fd, &options);

    // Set baud rate
    speed_t speed;
    switch(baud_rate) {
        case 57600: speed = B57600; break;
        case 115200: speed = B115200; break;
        case 230400: speed = B230400; break;
        case 460800: speed = B460800; break;
        case 500000: speed = B500000; break;
        case 921600: speed = B921600; break;
        default: speed = B115200; 
                 printf("Warning: Baudrate %d not explicitly handled, defaulting to 115200\n", baud_rate);
                 break;
    }
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    // 8N1
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CRTSCTS;

    // Raw input
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    // Set timeouts
    options.c_cc[VMIN] = 0;  // Non-blocking read
    options.c_cc[VTIME] = 0;

    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

int main() {
    const char* uart_name = "/dev/ttyAMA0";
    int baudrate = 115200;

    printf("Connecting to %s at %d baud (Raw MAVLink)...\n", uart_name, baudrate);
    int fd = open_serial_port(uart_name, baudrate);
    if (fd < 0) return 1;

    printf("Connected! Waiting for MAVLink messages...\n");

    mavlink_status_t status;
    mavlink_message_t msg;
    uint8_t buf[1024];
    int count = 0;

    while (1) {
        int n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            for (int i = 0; i < n; ++i) {
                // Parse one byte at a time
                if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &msg, &status)) {

                    if (msg.msgid == MAVLINK_MSG_ID_RC_CHANNELS) {
                        mavlink_rc_channels_t rc;
                        mavlink_msg_rc_channels_decode(&msg, &rc);
                        printf("RC_CHANNELS: chan9=%d (raw)\n", rc.chan9_raw);
                    }
                }
            }
        } else if (n < 0) {
            if (errno != EAGAIN) {
                perror("read error");
                usleep(100000); // Wait a bit on error
            }
        }
        
        // Small sleep to prevent 100% CPU usage in this loop
        usleep(1000); 
    }

    close(fd);
    return 0;
}
