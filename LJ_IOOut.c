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

#include "m_pd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>

#include "labjack.h"
#include "LJ_IOOut.h"

static void *LJ_IOOut_setup_thread(void *w)
{
    	t_LJ_IOOut *x = (t_LJ_IOOut*) w;

    	// Setup the variables we will need.
    	int r = 0; // For checking return values


	long idnum = -1;
	long stateD;
	long stateIO;
	long gains[] = {0, 0, 0, 0};
	long overvoltage;
	long count;
	long outputD;

	long trisD;
	long trisIO;
	long dataD;
	long dataIO;

	int i;

    	while(1) {
    		sem_wait(&x->go); // start working

		sem_wait (&LJ_IO);
		trisD  = LJ_trisD_fantom;
		trisIO = LJ_trisIO_fantom;
		dataD  = LJ_dataD_fantom;
		dataIO = LJ_dataIO_fantom;
		sem_post (&LJ_IO);

		for (i = 0; i < x->nb_ports; i++) {
			dataIO &= ~(1 << (x->ports[i]));
			dataIO |= ((int) x->data[i]) << (x->ports[i]);
		}

		sem_wait(&LJ_IO);
		LJ_dataIO_fantom = dataIO;
		sem_post(&LJ_IO);

		sem_wait(&LJ_busy);
		r = DigitalIO (&idnum, 0, &trisD, trisIO, &dataD, &dataIO, 1, &outputD);
		sem_post(&LJ_busy);

    		sem_post(&x->busy);     // on libere le thread
    	}

}

static void LJ_IOOut_thread_start(t_LJ_IOOut *x)
{
	// create the worker thread
    	if(pthread_attr_init(&x->LJ_IOOut_thread_attr) < 0){
       		error("labjack: could not launch acquire thread");
       		return;
    	} else if(pthread_attr_setdetachstate(&x->LJ_IOOut_thread_attr, PTHREAD_CREATE_DETACHED) < 0) {
	   	error("labjack: could not launch acquire thread");
	   	return;
    	}
    	if(pthread_create(&x->x_threadid, &x->LJ_IOOut_thread_attr, LJ_IOOut_setup_thread, x) < 0) {
	   	error("labjack: could not launch aquire thread");
       		return;
    	}
}


/*
 * Executed when banged ... take a shot ;)
 */
void LJ_IOOut_sample(t_LJ_IOOut *x)
{
	if (sem_trywait(&(x->busy)) != -1) {	
		sem_post(&x->go);
	} else {
		outlet_bang(x->err);
	}
}


void *LJ_IOOut_new(t_symbol *s, int argc, t_atom *argv)
{
	t_LJ_IOOut *x;	
	x = (t_LJ_IOOut *)pd_new(LJ_IOOut_class);
	int i;
	
	// inlets
	x->nb_ports = argc;
	for (i = 0; (i < argc) && (i < 4); i++) {
		floatinlet_new(&x->p_ob, &x->data[i]);
		x->ports[i] = atom_getfloat(argv+i);
	}

	x->err = outlet_new(&x->p_ob, &s_bang);

	// setting up thread and semaphores
	sem_init (&x->busy, 0, 1);
	sem_init (&x->go, 0, 0);

	LJ_IOOut_thread_start(x);

	return x; 
}

void LJ_IOOut_free(t_LJ_IOOut *x)
{
	while(pthread_cancel(x->x_threadid) < 0);	
    	//Close the device.

    	//LJUSB_CloseDevice(x->devHandle);
}

