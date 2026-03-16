//#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "Config.h"
#include "stateMachine.h"

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
            else if(byte == TRANSMITER)
            {
                st = A_RCV;
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
            uint8_t calc = TRANSMITER ^ 0x01;
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
            break;
    }   
    return st;  
}



STATE uptadeUA(uint8_t byte, STATE st)
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
            else if(byte == TRANSMITER)
            {
                st = A_RCV;
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
            else if(byte == 0x07)//UA Cont adress
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
            uint8_t calc = TRANSMITER ^ 0x03;
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
            break;
    }   
    return st;  
}
