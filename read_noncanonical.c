// Read from serial port in non-canonical mode
//
// Modified by: Eduardo Nuno Almeida [enalmeida@fe.up.pt]

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 5

volatile int STOP = FALSE;

enum State
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
};



int main(int argc, char *argv[])
{
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
    }

    // Open serial port device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(serialPortName, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(serialPortName);
        exit(-1);
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 1; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    // Loop for input
    unsigned char byte = 0;

    enum State currentState = START;

    const unsigned char FLAG = 0x7E;
    const unsigned char A = 0x03;
    const unsigned char C = 0x03;
    const unsigned char BCC1 = A^C;

    while (STOP == FALSE)
    {
        // Returns after 5 chars have been input
        int bytes = read(fd, byte, 1);

        
        switch (currentState)
        {
        case START:
            if(byte == FLAG){
                currentState = FLAG_RCV;
            }
            break;
        case FLAG_RCV:
            if(byte == A){
                currentState = A_RCV;
            }else if(byte != FLAG){
                currentState = START;
            }
        case A_RCV:
            if(byte == FLAG){
                currentState = FLAG_RCV;
            }else if(byte == C){
                currentState = C_RCV;
            }else{
                currentState = START;
            }
        default:
            break;
        }

        
        

        //Buffer has all the bytes that were sent
        //Check for flag
        
        
        int flagReceived = 0;
        unsigned int idx = 0;

        for(; idx < bytes; idx++){
            if(buf[idx] == FLAG)
                flagReceived = 1;
                break;
        }


        unsigned char buf_send[BUF_SIZE] = {0};
        if(flagReceived){
            printf("Flag Received\n");
            //verify BCC
            printf("Checking BCC...\n");
            if(buf[idx + 1]^buf[idx + 2] == buf[idx + 3]){
                printf("No errors found \n");
            }
            //Sent response - UA
            buf_send[0] = FLAG;
            buf_send[1] = 0x01; //Address
            buf_send[2] = 0x07; //Control
            buf_send[3] = buf_send[1]^buf_send[2];
            buf_send[4] = FLAG;

            int bytes_sent = write(fd, buf_send, BUF_SIZE);
            printf("%d bytes written\n", bytes_sent);
        }
                
        
        if (buf[0] == 'z')
            STOP = TRUE;
    }

    // The while() cycle should be changed in order to respect the specifications
    // of the protocol indicated in the Lab guide

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}
