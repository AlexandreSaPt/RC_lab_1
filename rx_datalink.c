#include "rx_datalink.h"

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

struct termios oldtio;
STATE current_state = STATE_START;

int setup_termios(int fd);

/*
llopen
» opens serial port (invokes openSerialPort)
» sends SET frame
» reads one byte at a time to receive UA frame (see state machine in slide 40)
» returns success or failure*/
int llopen(int argc, char *argv[])
{
    //openSerialPort:
    // Program usage: Uses either COM1 or COM2
    const char *serialPortName = argv[1];

    if (argc < 2)
    {
        printf("Incorrect program usage\n"
               "Usage: %s <SerialPort>\n"
               "Example: %s /dev/ttyS1\n",
               argv[0],
               argv[0]);
        exit(1);
        return 1;
    }

    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(serialPortName, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(serialPortName);
        return -1;
    }

    if (setup_termios(fd) == -1) {
        return -1; 
    }

    
    unsigned char uaFrame[5] = {
        FLAG,
        TRANSMITER,
        0x07,                   
        TRANSMITER ^ 0x07,
        FLAG
    };


    setup();//setup the alarm! 
    
    alarmEnabled = FALSE;
    alarmCount = 0;
    while (alarmCount < 4 && current_state != STOP)//read 5 bytes! 
    {
        if (alarmEnabled == FALSE) {
            alarm(3);
            alarmEnabled = TRUE;
            printf("Waiting for SET Frame - Try number %d\n", alarmCount);
        }

        uint8_t byte = 0;
        if(read(fd, &byte, 1) > 0)
        {
            current_state = updateSupervisionFrame(byte, current_state, false);
            if(current_state == STOP)
            {
                break;
            } 
        } 
    }
    alarm(0);//reset alarm! 
    if (current_state == STOP){
        printf("Received SET frame...\n Sending UA Frame\n");
        int bytes = write(fd, uaFrame, 5);
        return 0;
    }
    // === FAILED ===
    printf("Failed to receive SET after %d retries\n", alarmCount);
    close(fd);
    return -1;  
}


int llread()
{
    
}

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


int setup_termios(int fd)
{
    struct termios newtio;

    // Save current port settings to the global oldtio
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        return -1;
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;

    // --- CRITICAL FIX FOR YOUR STATE MACHINE ---
    newtio.c_cc[VTIME] = 0; // Do not use the termios timer
    newtio.c_cc[VMIN] = 0;  // Block read() until exactly 0 byte arrives

    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        return -1;
    }

    printf("New termios structure set\n");
    return 0;
}