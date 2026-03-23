//#include <stdio.h>

#include "stateMachine.h"


void init()
{
    //maybe update or set the state 
}

STATE updateReceiveSM(uint8_t byte, STATE st, uint8_t flag, uint8_t a, uint8_t c)
{        
    uint8_t expected_BCC = a ^ c; 
    switch (st){

        case STATE_START:
            if(byte == flag){
                st= FLAG_RCV;
            }
            break;

        case FLAG_RCV:
            if(byte== flag)
            {
                st= FLAG_RCV;
            } 
            else if(byte == a)
            {
                st = A_RCV;
            } 
            else{
                st = STATE_START;
            }
            break;
        case A_RCV:

            if(byte == flag)
            {
                st = FLAG_RCV;
            }
            else if(byte == c)
            {
                st = C_RCV;
            }
            else st = STATE_START;
            break;

        case C_RCV:
            if(byte == flag)
            {
                st = FLAG_RCV;
            }
            else if(byte == expected_BCC)
            {
                st = BCC_OK;
            }
            else {
                st = STATE_START;
            }
            break;
        case BCC_OK:
            if(byte == flag){
                st = STOP;
            }
            else{
                st = STATE_START;
            }
            break;
        case STOP:
            break;
    }   
    return st;  
}
