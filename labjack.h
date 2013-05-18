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

#ifndef LABJACK_H
#define LABJACK_H

#ifdef WIN32

#include <ljackuw.h>


#else

#include "labjackusb.h"
#include <libusb-1.0/libusb.h>

#include "LJ_Utils.h"


// Labjack Handle
extern HANDLE devHandle;

#endif


extern sem_t LJ_busy;
extern sem_t LJ_IO;
extern long LJ_trisD_fantom;
extern long LJ_trisIO_fantom;
extern long LJ_dataD_fantom;
extern long LJ_dataIO_fantom;

#endif
