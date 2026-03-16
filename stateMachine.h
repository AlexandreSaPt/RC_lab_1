#ifndef STATEMACHINE_H
#define STATEMACHINE_H





uint8_t frameReady;

typedef enum{
    STATE_START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
}STATE;

void init();
STATE updateSet(uint8_t byte, STATE st);
STATE updateUA(uint8_t byte, STATE st);
#endif