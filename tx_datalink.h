#ifndef TX_DATALINK_H
#define TX_DATALINK_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include "alarm_sigaction.h" 


#define BUF_SIZE 256
#define BAUDRATE B38400



int setup_termios(int fd);
int llopen(int argc, char *argv[]);
int llwrite(unsigned char *data, uint8_t size);

#endif