
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

#ifndef LJ_UTILS_H
#define LJ_UTILS_H

#define U12_COMMAND_LENGTH 8


long DigitalIO (    long *idnum,
                    long demo,
                    long *trisD,
                    long trisIO,
                    long *stateD,
                    long *stateIO,
                    long updateDigital,
                    long *outputD );


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
                   float analogOut1);


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
                   float *voltages );
#endif
