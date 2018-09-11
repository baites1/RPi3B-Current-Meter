/*
 * Library used to read differential voltage from TI ADS1115 analogue to digital converter
 * ADS1115 Data Sheet: http://www.ti.com/lit/ds/symlink/ads1115.pdf
 * Non-standard dependancy(s): WiringPI GPIO C++ library ( http://wiringpi.com/download-and-install/ )
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/i2c-dev.h> // I2C bus definitions
#include <math.h>

float getRMSCurrent(int ADC_Address, int Desired_ADC_Channel, int Number_Of_Samples, int CT_Amp_Rating)
{
    // Create I2C file for connecting to the I2C bus
    int I2CFile;
    I2CFile = open("/dev/i2c-1", O_RDWR);
    if (I2CFile < 0)
    {
        printf("There was an error opening the I2C device file. \n\r");
        close(I2CFile);
        exit(1);
    }
    
    // Specify the address of the I2C Slave to communicate with and error check.
    if ((ioctl(I2CFile, I2C_SLAVE, ADC_Address)) < 0) 
    {
        printf("There was an error initializing a connection to the I2C device. \n\r");
        close(I2CFile);
        exit(1);
    }
    
    // Create buffer to store the 3 bytes of data that we are going to write to the I2C device
    // Byte 1 = Pointer register control
    // Byte 2 = MSBs of configuration register
    // Byte 3 = LSBs of configuration register
    
    uint8_t writeBuf[3];
    
    // The following three bytes are written to the ADS1115 to set the config register and start a continuous conversion
    
    // Byte 1: 
    // Bits 0:1 = Pointer register. Set to 0 to read data, set to 1 to write data.
    // Bits 2:7 = Not used
    
    writeBuf[0] = 1;
    
    // Byte 2:
    // Bit 15 = Operation status, or conversion start (0 Not used in continuous conversion mode)
    // Bit 14:12 = Input MUX Configuration (000 to measure In0/In1 Diff, 011 to measure In2/In3 Diff)
    // Bit 11:9 = Gain amplifier (011 for +-1.024 volts)
    // Bit 8 = Operating mode (0 for continuous conversion)
    if (Desired_ADC_Channel == 1)
    {
        writeBuf[1] = 0x06; // 0000 0110
    }
    if (Desired_ADC_Channel == 2)
    {
        writeBuf[1] = 0x36; // 0011 0110
    }
    if (!((Desired_ADC_Channel == 1) | (Desired_ADC_Channel == 2)))
    {
        printf("The differential channel you have selected in not compatible. \n\r"
               "Please select either 1 or 2.  \n\r");
        close(I2CFile);
        exit(1);
    }
    
    // Byte 3:
    // Bit 7:5 = Data rate (110 for 475 samples per second)
    // Bit 4 = Comparator mode (0 for traditional comparator)
    // Bit 3 = Comparator polarity (0 for active low)
    // Bit 2 = Latching comparator (0 for non latching)
    // Bit 1:0 = Comparator queue and disable (11 to disable comparator)
    writeBuf[2] = 0xC3; // 1100 0011

    // Write writeBuf to the ADS1115, the 3 specifies the number of bytes we are writing.
    if (write(I2CFile, writeBuf, 3) != 3) 
    {
        printf("Writing 1st config to the ADS115 failed. \n\r");
        close(I2CFile);
        exit(1);
    }
    
    // 500ms delay to allow first conversion to occur.
    usleep(500000);
    
    // Set the pointer register to read mode, and write this to the ADC.
    writeBuf[0] = 0;
    if (write(I2CFile, writeBuf, 1) != 1) 
    {
    printf("Writing 2nd config to the ADS115 failed.  \n\r");
    }
    
    // Create buffer to store the 2 bytes of data that we read from the ADC
    uint8_t readBuf[2];
    
    // Create variable to store the combined 16 bit value of our ADC conversion
    int16_t rawValue;
    
    // Create an array, and read 'n' number of samples of the AC waveform.
    float samples[Number_Of_Samples] = {};
    for (int i = 0; i < Number_Of_Samples; i = i+1)
    {
        // Initialize the buffer used to read data from the ADS1115 to 0
        readBuf[0]= 0;
        readBuf[1]= 0;
        // Read the contents of the conversion register into readBuf
        read(I2CFile, readBuf, 2);
        // Combine the two bytes of readBuf into a single 16 bit result
        rawValue = readBuf[0] << 8 | readBuf[1];
        // Convert raw value to voltage (based on a fixed ADC gain of 1.024 volts)
        float voltage = (float)rawValue * 1.024 / 32767.0;
        // Convert voltage to current (based on the CT amp rating, assuming the CT secondary is 1 volt at max CT amp rating)
        float current = CT_Amp_Rating * voltage;
        // Add current value to sample array
        samples[i] = current;
    }
    
    close(I2CFile);
    
    // Square all values in sample array.
    float sum = 0.0;
    for (int i = 0; i < Number_Of_Samples; i = i+1)
    {
        float currentSquared = pow (samples[i], 2.0);
        samples[i] = currentSquared;
        sum = sum + currentSquared;
    }
    
    // Find the average in sample array
    float average = sum / Number_Of_Samples;
    
    // Return the square root of the average (which is now a single RMS current value)
    return sqrt(average);
}