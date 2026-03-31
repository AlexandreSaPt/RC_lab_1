#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include "stateMachine.h"

#define MAX_FRAME 15000 //Bytes

#define FLAG 0x7E

//Alarm params
#define TIMEOUT 3 //seconds
#define TIMEOUT_RECEIVER 5 // seconds
#define TIMEOUT_TRANSMITER 3 //Seconds

#define MAX_ALARM_COUNT_RX 4 

#define RR0 0x05
#define RR1 0x85


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

/**
 * @return 
 * \n -1 -> could not open serial port 
 * \n -2 -> could not tcgetattr 
 * \n -3 -> could not set tcsetattr
 * \n -4 -> could not set alarm (sigaction)
 * \n -5 -> timeout waiting for set frame
 */
int llopen(char portName[], bool isTransmitter){
    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(serialPortName, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror(serialPortName);
        return -1;
    }

    
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        return -2;
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
        return (-3);
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
        return (-4);
    }
    printf("Alarm configured\n");

    if (isTransmitter == 0)
    {
       //receiver stuff
        if(wait_set_frame(fd) < 0){
            return -5;
        }
        //set frame received
        send_ua(fd);
    }//falta trasmitter
    
    
    return fd;
}

/**
 * @return 
 * \n 0 -> set reeived
 * \n -1 -> alarm count > MAX_ALARM_COUNT_RX
 */
int wait_set_frame(int fd){
    alarmCount = 0; //global variable
    alarmEnabled = 0; //global variable
    STATE currentState = STATE_START;
    while (1)
    {
        if (alarmEnabled == FALSE)
        {   
            printf("Setting up receiver alarm, with alarm count of %d...", alarmCount);
            alarm(TIMEOUT_RECEIVER); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        char byte;
        int bytesRead = read(fd, &byte, 1);
        if(bytesRead > 0){
            //update state machine
            updateSupervisionFrame(byte, &currentState, 1);
        }
        if(alarmCount > MAX_ALARM_COUNT_RX){
            alarm(0);
            return -1;
        }
        if (currentState == STOP){
            alarm(0);
            return 0;
        }
    }
}

int send_ua(int fd){
    alarmCount = 0; //global variable
    alarmEnabled = 0; //global variable
    STATE currentState = STATE_START;
    char buf[5]; //UA Frame
    buf[0]= FLAG;
    buf[1]= 0x03;
    buf[2]= 0x03;
    buf[3]= buf[1]^buf[2];
    buf[4]= FLAG;

    //Trying to send
    int bytes = write(fd, buf, 5);
    printf("%d bytes written\n", bytes);
    printf("Sent data... waiting\n");
    //PROBLEM -> NOT RETRY LOGGIC
    //Não sei ainda como vou fazer
    //pq ler I frame em si tem de estar no llread,
    //então terei de receber o número de bytes lidos pelo llread e se há erro no primeiro para reenviar o UA frame

}

/**
 * @return 
 * \n -1 -> buffer demasiado pequeno
 * \n -2 -> max retries excedida
 */
int llread(int fd, char* buf){
    if(sizeof(buf) < MAX_FRAME){
        return -1;
    }
    
    alarmEnabled = 0; //global variable
    alarmCount = 0; //global variable
    bool frame_number_to_receive = 0;

    char bufSend[5];

    bufSend[0] = FLAG;
    bufSend[1] = 0x03;
    bufSend[2] = 0; //will be set in the loop
    bufSend[3] = 0; //same
    bufSend[4] = FLAG;

    STATE currentState = STATE_START;

    uint8_t bufCounter = 0;
    while (1)
    {
        if (alarmEnabled == FALSE)
        {   
            //Trying to send
            bufSend[2] = frame_number_to_receive == 0 ? RR0 : RR1;
            bufSend[3] = bufSend[1]^bufSend[2]; //BCC1

            int bytes = write(fd, bufSend, 5);
            printf("Supervision frame sent. %d bytes written\n", bytes);
            printf("Frame number waiting to receive: %d\n", frame_number_to_receive);

            alarm(TIMEOUT_RECEIVER); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        char byte;
        int bytesRead = read(fd, &byte, 1);
        if(bytesRead > 0){
            //byte
            //update state machine
            updateSupervisionFrame(byte, &currentState, 1);
        }
        if (currentState == BCC_OK){
            //receiving data
            buf[bufCounter] = byte;
            bufCounter++;
            alarm(0); //como recebeu com sucesso, é pra resetar os alarmes
            alarmEnabled = 0; //para enviar o próximo S frame
            frame_number_to_receive = !frame_number_to_receive;
        }
        if (currentState == STOP){
            alarm(0);
            return bufCounter;
        }
        if(alarmCount > MAX_ALARM_COUNT_RX){
            alarm(0);
            return -2;
        }
    }
}