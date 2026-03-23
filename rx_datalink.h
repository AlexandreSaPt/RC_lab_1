#ifndef RX_DATALINK_H
#define RX_DATALINK_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include "alarm_sigaction.h" 
#include "stateMachine.h"


#define BUF_SIZE 256
#define BAUDRATE B38400


int setup_termios(int fd);

/*
llopen
» opens serial port (invokes openSerialPort)
» sends SET frame
» reads one byte at a time to receive UA frame (see state machine in slide 40)
» returns success or failure*/
int llopen(int argc, char *argv[]);


int llwrite();

/*
 llwrite
» implements error detection (compute BCC over the data packet)
» parses data packet to implement byte stuffing (transparency)
» builds Information frame Ns (Ns=0 or 1)
» sends I frame
» reads one byte at a time to receive response
» if negative response (REJ) or no response, resends I frame up to a maximum number of times
(retransmission mechanism explained in slide 46)
» return success or failure

» llclose
» sends DISC frame
» reads one byte at a time to receive DISC frame
» sends UA frame
» closes serial port
*/


int setup_termios(int fd);


#endif