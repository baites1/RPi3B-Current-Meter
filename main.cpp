#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <exception>
#include <stdlib.h>
#include <unistd.h>
#include "ads1115.h" // Custom definition of ADS1115 ADC Board

using namespace std;

int main(int argc, char* argv[])
{
    // Calculate RMS current on differential channel 1 of ADC
    // Address: 48 | Ch: 1 | Samples: 500 | CT Amps: 60
    float ch1RMS = getRMSCurrent(0x48, 1, 500, 60);
    printf("%f", ch1RMS);
    
    // Calculate RMS current on differential channel 2 of ADC
    // Address: 48 | Ch: 2 | Samples: 500 | CT Amps: 20
    float ch2RMS = getRMSCurrent(0x48, 2, 500, 20);
    printf(",%f", ch2RMS);
    
    return 0;
}

