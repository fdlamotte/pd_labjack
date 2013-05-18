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
#include "LJ_DIn.h"

static void *LJ_DIn_setup_thread(void *w)
{
    	t_LJ_DIn *x = (t_LJ_DIn*) w;

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

		sem_wait (&LJ_busy);
		r = DigitalIO (&idnum, 0, &trisD, trisIO, &dataD, &dataIO, 0, &outputD);
		sem_post (&LJ_busy);
		x->lasterror = r;

		for (i=0; i<x->nb_ports; i++) {
			x->io_v[i] = (dataD >> x->ports[i]) & 1;
		}

		sem_post(&x->acquired);
    	}

}

static void LJ_DIn_thread_start(t_LJ_DIn *x)
{
	// create the worker thread
    	if(pthread_attr_init(&x->LJ_DIn_thread_attr) < 0){
       		error("labjack: could not launch acquire thread");
       		return;
    	} else if(pthread_attr_setdetachstate(&x->LJ_DIn_thread_attr, PTHREAD_CREATE_DETACHED) < 0) {
	   	error("labjack: could not launch acquire thread");
	   	return;
    	}
    	if(pthread_create(&x->x_threadid, &x->LJ_DIn_thread_attr, LJ_DIn_setup_thread, x) < 0) {
	   	error("labjack: could not launch aquire thread");
       		return;
    	}
}


/*
 * Executed when banged ... take a shot ;)
 */
void LJ_DIn_sample(t_LJ_DIn *x)
{
	if (sem_trywait(&(x->busy)) != -1) {	
		sem_post(&x->go);
  		clock_delay (x->x_clock, 10);
	} else {
		outlet_bang(x->err);
	}
}

/*
 * After thread is released, periodically check if outputs are updated
 */
static void LJ_DIn_update(t_LJ_DIn *x)
{
	int i;
	if (sem_trywait(&x->acquired) != -1) {
		if (x->lasterror == 0) {
			for (i = 0; i < x->nb_ports; i++) {
				outlet_float(x->io[i], (t_float) x->io_v[i]);
			}
		} else {
			outlet_float(x->err, x->lasterror);
		}
    		sem_post(&x->busy);     // on libere le thread
	} else { // if data not ready, wait another 2ms
		clock_delay (x->x_clock, 2);
	}
}


void *LJ_DIn_new(t_symbol *s, int argc, t_atom *argv)
{
	t_LJ_DIn *x;	
	x = (t_LJ_DIn *)pd_new(LJ_DIn_class);
	int i;
	
	// outlets
	argc = (argc > 16) ? 16 : argc;
	x->nb_ports = argc;
	for (i = 0; i < argc; i++) {
		x->io[i] = outlet_new(&x->p_ob, &s_float);
		x->ports[i] = atom_getfloat(argv+i);
	}

	x->err = outlet_new(&x->p_ob, &s_bang);

	// a clock to update outputs 
  	x->x_clock = clock_new(x, (t_method) LJ_DIn_update);

	// setting up thread and semaphores
	sem_init (&x->busy, 0, 1);
	sem_init (&x->go, 0, 0);
	sem_init (&x->acquired, 0, 1);

	LJ_DIn_thread_start(x);

	return x; 
}

void LJ_DIn_free(t_LJ_DIn *x)
{
	while(pthread_cancel(x->x_threadid) < 0);	
    	//Close the device.

    	//LJUSB_CloseDevice(x->devHandle);
}

