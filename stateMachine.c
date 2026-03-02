#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

enum STATE{
    STATE_START,
    FLAG_RCV,

};



void init()
{

}

enum STATE update(enum STATE currState, uint8_t byte)
{
    switch (currState){
        case STATE_START:
            if(byte == )
            break;
    }
       

}