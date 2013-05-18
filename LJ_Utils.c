
/*
 * labjack - PD External for LabJack Cards
 * 
 * Ecole Nationale Superieure d'Ingenieurs de Bretagne Sud
 * http://www.ensibs.fr
 * Copyright (C) 2012 Florent de Lamotte
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version. (see LICENCE.txt)
 *
 */

#include <m_pd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>

#include "labjack.h"

/*
 * Writes to Labjack and reads reply
 *
 * Automatically connects if disconnected
 * 
 * 
 * args :
 *  cur_handle : labjack handle
 *  sendBuffer : data sent to LabJack
 *  recBuffer  : received data
 *
 * returns : -1 on failure
 */
int writeRead(HANDLE cur_handle, BYTE * sendBuffer, BYTE * recBuffer ) 
{
    	int r = 0;

    	// Open the U12
	// TODO: Support also U3 and U6
	if (devHandle == NULL) { //open the labjack
		post("Opening Labjack");
   		devHandle = LJUSB_OpenDevice(1, 0, U12_PRODUCT_ID);
   		if( devHandle == NULL ) {
        		error("Couldn't open U12. Please connect one and try again.");
        		devHandle = 0;
			//sem_post(&LJ_busy);
			return -1;
    		} else {
			post("Labjack opened");
		}
	}

    	r = LJUSB_Write( devHandle, sendBuffer, U12_COMMAND_LENGTH );
    	if( r != U12_COMMAND_LENGTH ) {
        	error("An error occurred when trying to write to the U12.");
		LJUSB_CloseDevice(devHandle);
		devHandle = NULL;
		//sem_post(&LJ_busy);
		return -1;
    	}
    
    	r = LJUSB_Read( devHandle, recBuffer, U12_COMMAND_LENGTH );
   	if( r != U12_COMMAND_LENGTH ) {
        	if(errno == LIBUSB_ERROR_TIMEOUT) {
			error ("Timeout while talking to U12.");
    			r = LJUSB_Write( devHandle, sendBuffer, U12_COMMAND_LENGTH );

    			if( r == U12_COMMAND_LENGTH ) {
    				r = LJUSB_Read( devHandle, recBuffer, U12_COMMAND_LENGTH );
			}
        	}
        
		if (r != U12_COMMAND_LENGTH) {
       		 	error("An error occurred when trying to read from the U12.");// The error was: %d\n", errno);
        		LJUSB_CloseDevice(devHandle);
			devHandle = NULL;
			//sem_post(&LJ_busy);
			return -1;
		}
    	}
    
//#define DEBUG_DIO
#ifdef DEBUG_DIO
	printf("sent 0 %x\n", (int)sendBuffer[0]);
	printf("sent 1 %x\n", (int)sendBuffer[1]);
	printf("sent 2 %x\n", (int)sendBuffer[2]);
	printf("sent 3 %x\n", (int)sendBuffer[3]);
	printf("sent 4 %x\n", (int)sendBuffer[4]);
	printf("sent 5 %x\n", (int)sendBuffer[5]);
	printf("sent 6 %x\n", (int)sendBuffer[6]);
	printf("sent 7 %x\n", (int)sendBuffer[7]);

	printf("recv 0 %x\n", (int)recBuffer[0]);
	printf("recv 1 %x\n", (int)recBuffer[1]);
	printf("recv 2 %x\n", (int)recBuffer[2]);
	printf("recv 3 %x\n", (int)recBuffer[3]);
	printf("recv 4 %x\n", (int)recBuffer[4]);
	printf("recv 5 %x\n", (int)recBuffer[5]);
	printf("recv 6 %x\n", (int)recBuffer[6]);
	printf("recv 7 %x\n", (int)recBuffer[7]);

	printf("returned %d\n", r);
#endif

    	return 0;
}

void parseAISampleBytes(BYTE * recBuffer, t_float * ai){
    int temp;
    
    // Apply the single-ended conversion to results
    temp = (recBuffer[2] >> 4) & 0xf;
    temp = (temp << 8) + recBuffer[3];
    ai[0] = ((double)temp * 20.0 / 4096.0) - 10;
    
    temp = recBuffer[2] & 0xf;
    temp = (temp << 8) + recBuffer[4];
    ai[1] = ((double)temp * 20.0 / 4096.0) - 10;
    
    temp = (recBuffer[5] >> 4) & 0xf;
    temp = (temp << 8) + recBuffer[6];
    ai[2] = ((double)temp * 20.0 / 4096.0) - 10;
    
    temp = recBuffer[5] & 0xf;
    temp = (temp << 8) + recBuffer[7];
    ai[3] = ((double)temp * 20.0 / 4096.0) - 10;

}

/*
 * Reads the voltages from 1,2, or 4 analog inputs. Also controls/reads the 4 IO ports. 
 *
 * Uses information from section 5.1 of the U12 User's Guide
 * http://labjack.com/support/u12/users-guide/5.1
 *
 * Inputs:
 * 
 *     *idnum – Local ID, serial number, or -1 for first found.
 *     demo – Send 0 for normal operation, >0 for demo mode. Demo mode allows this function to be called without a LabJack.
 *     *stateIO – Output states for IO0-IO3. Has no effect if IO are configured as inputs, so a different function must be used to configure as output.
 *     updateIO – If >0, state values will be written. Otherwise, just a read is performed.
 *     ledOn – If >0, the LabJack LED is turned on.
 *     numChannels – Number of analog input channels to read (1,2, or 4).
 *     *channels – Pointer to an array of channel commands with at least numChannels elements. Each channel command is 0-7 for single-ended, or 8-11 for differential.
 *     *gains – Pointer to an array of gain commands with at least numChannels elements. Gain commands are 0=1, 1=2, …, 7=20. This amplification is only available for differential channels.
 *     disableCal – If >0, voltages returned will be raw readings that are not corrected using calibration constants.
 *     *voltages – Pointer to an array where voltage readings are returned. Send a 4-element array of zeros.
 * 
 * Outputs:
 * 
 *     *idnum – Returns the local ID or –1 if no LabJack is found.
 *     *stateIO – Returns input states of IO0-IO3.
 *     *overVoltage – If >0, an overvoltage has been detected on one of the selected analog inputs.
 *     *voltages – Pointer to an array where numChannels voltage readings are returned.
 */
long AISample (    long *idnum,
                   long demo,
                   long *stateIO,
                   long updateIO,
                   long ledOn,
                   long numChannels,
                   long *channels,
                   long *gains,
                   long disableCal,
                   long *overVoltage,
                   float *voltages )
{
	char sendBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0}; 
	char recvBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int r;
	int temp;
	int i;

	for (i=0; i<4; i++) {
    		sendBuffer[i] = /*((gains[i] & 0x3) << 4)*/8 | (channels[i] & 0xF); 
	}

    	sendBuffer[4] = ledOn | (updateIO << 1);
    	sendBuffer[5] = 0xC0 + *stateIO;
    	sendBuffer[6] = 0;
    	sendBuffer[7] = 0;

    	r = writeRead(devHandle, sendBuffer, recvBuffer);
    
	parseAISampleBytes(recvBuffer, voltages);

	*overVoltage = (recvBuffer[0] >> 4) & 1;
	*stateIO = recvBuffer[0] & 0xF;

	return r;
}


/*
 * AOUpdate
 * 
 * Sets the voltages of the analog outputs. Also controls/reads all 20 digital I/O and the counter.
 *
 *
 * Returns: LabJack errorcodes or 0 for no error.
 * Inputs:
 * 
 *     *idnum – Local ID, serial number, or -1 for first found.
 *     demo – Send 0 for normal operation, >0 for demo mode. Demo mode allows this function to be called without a LabJack.
 *     trisD – Directions for D0-D15. 0=Input, 1=Output.
 *     trisIO – Directions for IO0-IO3. 0=Input, 1=Output.
 *     *stateD – Output states for D0-D15.
 *     *stateIO – Output states for IO0-IO3.
 *     updateDigital – If >0, tris and state values will be written. Otherwise, just a read is performed.
 *     resetCounter – If >0, the counter is reset to zero after being read.
 *     analogOut0 – Voltage from 0.0 to 5.0 for AO0.
 *     analogOut1 – Voltage from 0.0 to 5.0 for AO1.
 * 
 * Outputs:
 * 
 *     *idnum – Returns the local ID or –1 if no LabJack is found.
 *     *stateD – States of D0-D15.
 *     *stateIO – States of IO0-IO3.
 *     *count – Current value of the 32-bit counter (CNT). This value is read before the counter is reset.
 */
long AOUpdate (    long *idnum,
                   long demo,
                   long trisD,
                   long trisIO,
                   long *stateD,
                   long *stateIO,
                   long updateDigital,
                   long resetCounter,
                   unsigned long *count,
                   float analogOut0,
                   float analogOut1)
{
	char sendBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0}; 
	char recvBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int r = 0;

	int ao0_out = (analogOut0 / 5.0) * 0x3FF;
	int ao1_out = (analogOut1 / 5.0) * 0x3FF;

	// Build up the bytes
    	sendBuffer[0] = trisD >> 8;
    	sendBuffer[1] = trisD & 0xFF;
    	sendBuffer[2] = *stateD >> 8;
    	sendBuffer[3] = *stateD & 0xFF;
    	sendBuffer[4] = (trisIO << 4) | (*stateIO & 0xF);
    	sendBuffer[5] = ((resetCounter & 1) << 5) 
			| ((updateDigital & 1) << 4) 
			| ((ao0_out & 3) << 2) 
			| (ao1_out & 3);
    	sendBuffer[6] = ao0_out >> 2;
    	sendBuffer[7] = ao1_out >> 2;

    	r = writeRead(devHandle, sendBuffer, recvBuffer);

	*stateD = (recvBuffer[0] << 8) | (recvBuffer[1] & 0xFF);
	*stateIO = recvBuffer[3] >> 4;
	*count = ((recvBuffer[4] << 24) & 0xFF)
		|((recvBuffer[5] << 16) & 0xFF)
		|((recvBuffer[6] << 8) & 0xFF)
		|(recvBuffer[7] & 0xFF);

	return r;
}


/*
 * DigitalIO Wrapper function for Linux
 * see http://labjack.com/support/u12/users-guide/4.17
 *
 * Returns: LabJack errorcodes or 0 for no error.
 * Inputs:
 * 
 *     *idnum – Local ID, serial number, or -1 for first found.
 *     demo – Send 0 for normal operation, >0 for demo mode. Demo mode allows this function to be called without a LabJack.
 *     *trisD – Directions for D0-D15. 0=Input, 1=Output.
 *     trisIO – Directions for IO0-IO3. 0=Input, 1=Output.
 *     *stateD – Output states for D0-D15.
 *     *stateIO – Output states for IO0-IO3.
 *     updateDigital – If >0, tris and state values will be written. Otherwise, just a read is performed.
 * 
 * Outputs:
 * 
 *     *idnum – Returns the local ID or –1 if no LabJack is found.
 *     *trisD – Returns a read of the direction registers for D0-D15.
 *     *stateD – States of D0-D15.
 *     *stateIO – States of IO0-IO3.
 *     *outputD – Returns a read of the output registers for D0-D15.
 */
long DigitalIO (    long *idnum,
                    long demo,
                    long *trisD,
                    long trisIO,
                    long *stateD,
                    long *stateIO,
                    long updateDigital,
                    long *outputD )
{
	char sendBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0}; 
	char recvBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int r;

	*trisD ^= 0xFF;
    	sendBuffer[0] = (*trisD) >> 8;
    	sendBuffer[1] = (*trisD) & 0xFF;
    	sendBuffer[2] = (*stateD) >> 8;
    	sendBuffer[3] = (*stateD) & 0xFF;
    	sendBuffer[4] = ((trisIO ^ 0xF) << 4) | ((*stateIO) & 0xF);
    	sendBuffer[5] = 0x57;
    	sendBuffer[6] = updateDigital & 0x1;
    	sendBuffer[7] = 0;

	r = writeRead(devHandle, sendBuffer, recvBuffer);

	*trisD   = ((recvBuffer[4] << 8) | (recvBuffer[5])) ^ 0xFFFF;
	*stateD  = (recvBuffer[1] << 8) | (recvBuffer[2]);
	*stateIO = recvBuffer[3] >> 4;
	*outputD = (recvBuffer[6] << 8) | (recvBuffer[7]);

	return r;

}

