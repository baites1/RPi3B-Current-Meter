/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ads1115.h
 * Author: HP EliteBook 850
 *
 * Created on 29 August 2018, 5:51 AM
 */

#ifndef ADS1115_H
#define ADS1115_H

// Method to read RMS current (amps) from ADC
float getRMSCurrent(int ADS_Address, int Desired_Channel, int Number_Of_Samples, int CT_Amp_Rating);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* ADS1115_H */

