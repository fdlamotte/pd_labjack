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
#include "LJ_InputSample.h"


static void *is_acquire_thread(void *w)
{
    	t_LJ_InputSample *x = (t_LJ_InputSample*) w;

    	// Setup the variables we will need.
    	int r = 0; // For checking return values

	long stateIO;
	long gains[] = {0, 0, 0, 0};
	long idnum = -1;
	long overvoltage;

	int i;

    	while(1) {
    		sem_wait(&x->go);

		for (i = 0; i<8; i++) x->ai_v[i] = 0; // erase the values or else it does not work :(

		sem_wait(&LJ_busy);
		r = AISample (&idnum, 0, &stateIO, 0, 1, 4, x->ch_config, gains, 0, &overvoltage, x->ai_v );
		sem_post(&LJ_busy);

		// if we have more than 4 channels, read once more
		if (x->channels > 4) {
			sem_wait(&LJ_busy);
			r = AISample (&idnum, 0, &stateIO, 0, 1, 4, x->ch_config + 4, gains, 0, &overvoltage, x->ai_v+4 );
			sem_post(&LJ_busy);
		}

		x->lasterror = r;

		if (r == 0) {
			x->io_v[0] = (stateIO & 1);
			x->io_v[1] = (stateIO & 2) >> 1;
			x->io_v[2] = (stateIO & 4) >> 2;
			x->io_v[3] = (stateIO & 8) >> 3;
		}

		sem_post(&x->acquired); // acquisition terminee
    	}

}

static void is_thread_start(t_LJ_InputSample *x)
{
	// create the worker thread
    	if(pthread_attr_init(&x->LJ_InputSample_thread_attr) < 0){
       		error("labjack: could not launch acquire thread");
       		return;
    	} else if(pthread_attr_setdetachstate(&x->LJ_InputSample_thread_attr, PTHREAD_CREATE_DETACHED) < 0) {
	   	error("labjack: could not launch acquire thread");
	   	return;
    	}

    	if(pthread_create(&x->x_threadid, &x->LJ_InputSample_thread_attr, is_acquire_thread, x) < 0) {
	   	error("labjack: could not launch aquire thread");
       		return;
    	}
}


/*
 * Executed when banged ... take a shot ;)
 */
void LJ_InputSample_sample(t_LJ_InputSample *x)
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
static void LJ_InputSample_update(t_LJ_InputSample *x)
{
	int i;
	if (sem_trywait(&x->acquired) != -1) {
		if (x->lasterror == 0) {
			for (i = 0; i < x->channels; i++) {
				outlet_float(x->ai[i], x->ai_v[i]);
			}
			for (i = 0; i < 4; i++) {
				outlet_float(x->io[i], x->io_v[i]);
			}
		} else {
			outlet_float(x->err, x->lasterror);
		}
    		sem_post(&x->busy);     // on libere le thread
	} else { // if data not ready, wait another 2ms
		clock_delay (x->x_clock, 2);
	}
}


void *LJ_InputSample_new(t_symbol *s, int argc, t_atom *argv)
{
	t_LJ_InputSample *x;	
	x = (t_LJ_InputSample *)pd_new(LJ_InputSample_class);
	int i;
	
	// one outlet for each analog input
	x->channels = argc;
	for (i = 0; i < argc; i++) {
		x->ai[i] = outlet_new(&x->p_ob, &s_float);
		x->ch_config[i] = atom_getfloat(argv+i);		
	}
	for (i = argc; i<8; i++) x->ch_config[i] = 0;

	for (i = 0; i < 4; i++) {
		x->io[i] = outlet_new(&x->p_ob, &s_float);
	}

	x->err = outlet_new(&x->p_ob, &s_bang);
	x->errors = 0;

	// a clock to update outputs 
  	x->x_clock = clock_new(x, (t_method) LJ_InputSample_update);

	// setting up thread and semaphores
	sem_init (&x->busy, 0, 1);
	sem_init (&x->go, 0, 0);
	sem_init (&x->acquired, 0, 0);

	is_thread_start(x);

	return x; 
}

void LJ_InputSample_free(t_LJ_InputSample *x)
{
	while(pthread_cancel(x->x_threadid) < 0);	
}

