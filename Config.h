#include <unistd.h>
#include <stdint.h>

#define FLAG 0x7E
#define A_SENDER 0x03 
#define A_RECEIVER 0x01


struct Frame{
    uint8_t flag;
    uint8_t adress;
    uint8_t control;
    uint8_t bcc1;
    uint8_t frameReady;
}frame;







