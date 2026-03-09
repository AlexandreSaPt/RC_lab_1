//#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "Config.h"

uint8_t frameReady;

typedef enum{
    STATE_START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
}STATE;

void init()
{
    //maybe update or set the state 
}

STATE uptadeSet(uint8_t byte, STATE st)
{
    switch (st){

        case STATE_START:
            if(byte == FLAG){
                st= FLAG_RCV;
            }
            break;

        case FLAG_RCV:
            if(byte== FLAG)
            {
                st= FLAG_RCV;
            } 
            else if(byte == A_SENDER)
            {
                st = C_RCV;
            } 
            else{
                st = STATE_START;
            }
            break;
        case A_RCV:
            if(byte == FLAG)
            {
                st = FLAG_RCV;
            }
            else if(byte == 0x01)
            {
                st = C_RCV;
            }
            else st = STATE_START;
            break;
        case C_RCV:
            if(byte == FLAG)
            {
                st = FLAG_RCV;
            }
            uint8_t calc = A_SENDER ^ 0x01;
            if(calc == byte)
            {
                st = BCC_OK;
            }
            else {
                st = STATE_START;
            }
            break;
        case BCC_OK:
            if(byte == FLAG){
                st = STOP;
            }
            else{
                st = STATE_START;
            }
    }   

    return st;  
}

