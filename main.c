#include "tx_datalink.h"
#include <stdio.h>

/*
Compile: gcc 
Execute: ./ex /dev/ttyUSB0
*/


int main(int argc, char *argv[])
{

    
    int res = llopen(argc,argv);
    if(res == 0) printf("All ok ");
    else printf("Error, %d", res);

    // Wait until all bytes have been written to the serial port
    //sleep(1);


    return 0;
}
