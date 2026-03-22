// Write to serial port in non-canonical mode
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
#include <signal.h>


#define FALSE 0
#define TRUE 1

#define FLAG 0x7E

//Alarm params
#define TIMEOUT 3 //seconds


int alarmEnabled = FALSE;
int alarmCount  = 0;

// Alarm function handler.
// This function will run whenever the signal SIGALRM is received.
void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;

    printf("Alarm #%d received\n", alarmCount);
}


// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 5

volatile int STOP = FALSE;

//serial port cenas
int fd;
struct termios oldtio;
void init(int size, char portName[]){
    // Program usage: Uses either COM1 or COM2
    const char *serialPortName = portName;

    if (size < 2)
    {
        printf("Incorrect program usage\n"
               "Usage: %s <SerialPort>\n"
               "Example: %s /dev/ttyS1\n",
               portName[0],
               portName[0]);
        exit(1);
    }
    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(serialPortName, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror(serialPortName);
        exit(-1);
    }

    
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
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
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


    // Set alarm function handler.
    // Install the function signal to be automatically invoked when the timer expires,
    // invoking in its turn the user function alarmHandler
    struct sigaction act = {0};
    act.sa_handler = &alarmHandler;
    if (sigaction(SIGALRM, &act, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("Alarm configured\n");
}

//Function that fills the headers and computes BCC 
int create_header(char flag, char A, char C, char buf[]){
    // Create string to send
    if(sizeof(buf) < 4){
        printf("Buffer size wrong");
        return -1;
    }
    buf[0]= flag;
    buf[1]= A;
    buf[2]= C;
    buf[3]= buf[1]^buf[2];
}

int send_set_frame(){
    char buf[5];
    create_header(FLAG, 0x03, 0x03, buf);

    alarmCount = 0; //global variable
    
    while (alarmCount < 4)
    {
        if (alarmEnabled == FALSE)
        {   
            //Trying to send
            int bytes = write(fd, buf, 5);
            printf("%d bytes written\n", bytes);
            printf("Sent data... waiting\n");

            alarm(TIMEOUT); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        char byte;
        int bytesRead = read(fd, byte, 1);
        if(bytesRead > 0){
            //byte
            //update state machine
            //if state == stop
        }

        //if current state == stop
        //alarm(0)
        //break;
    }
}

void close(){
    // Wait until all bytes have been written to the serial port
    sleep(1);
    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    close(fd);
}


int main(int argc, char *argv[])
{
    return 0;
}
