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
#include "LJ_AISample.h"


static void *acquire_thread(void *w)
{
    	t_LJ_AISample *x = (t_LJ_AISample*) w;

	long stateIO;
	long gains[] = {0, 0, 0, 0};
	long idnum = -1;
	long overvoltage;


    	int r = 0; // For checking return values

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

		sem_post(&x->acquired); // acquisition terminee
//    		sem_post(&x->busy);     // on libere le thread
    	}

}

static void thread_start(t_LJ_AISample *x)
{
	// create the worker thread
    	if(pthread_attr_init(&x->LJ_AISample_thread_attr) < 0){
		sys_lock();
       		error("labjack: could not launch acquire thread");
	   	sys_unlock();
       		return;
    	} else if(pthread_attr_setdetachstate(&x->LJ_AISample_thread_attr, PTHREAD_CREATE_DETACHED) < 0) {
       		sys_lock();
	   	error("labjack: could not launch acquire thread");
       		sys_unlock();
	   	return;
    	}

    	if(pthread_create(&x->x_threadid, &x->LJ_AISample_thread_attr, acquire_thread, x) < 0) {
       		sys_lock();
	   	error("labjack: could not launch aquire thread");
	   	sys_unlock();
       		return;
    	} 
}

/*
 * Executed when banged ... take a shot ;)
 */
void LJ_AISample_sample(t_LJ_AISample *x)
{
	if (sem_trywait(&x->busy) == 0) {	
		sem_post(&x->go);
  		clock_delay (x->x_clock, 10);
	} else {
		outlet_float(x->err, -1);
	}
}

/*
 * After thread is released, periodically check if outputs are updated
 */
static void LJ_AISample_update(t_LJ_AISample *x)
{
	int i;
	if (sem_trywait(&x->acquired) != -1) {
		if (x->lasterror == 0) {
			for (i = 0; i < x->channels; i++) {
				if (x->ai_v[i] < 50 && x->ai_v[i] > -50)
				outlet_float(x->ai[i], x->ai_v[i]);
			}
		} else {
			outlet_float(x->err, x->lasterror);
		}
    		sem_post(&x->busy);     // on autorise une autre acquisition
	} else { // if data not ready, wait another 2ms
		clock_delay (x->x_clock, 2);
	}
}


void *LJ_AISample_new(t_symbol *s, int argc, t_atom *argv)
{
	t_LJ_AISample *x;	
	x = (t_LJ_AISample *)pd_new(LJ_AISample_class);
	int i;
	
	// one outlet for each analog input
	x->channels = argc;
	for (i = 0; i < argc; i++) {
		(x->ai)[i] = outlet_new(&x->p_ob, &s_float);
		(x->ch_config)[i] = atom_getfloat(argv+i);
	}
	for (i = argc; i<8; i++) x->ch_config[i] = 0;

	x->err = outlet_new(&x->p_ob, &s_float);
	x->errors = 0;

	// a clock to update outputs 
  	x->x_clock = clock_new(x, (t_method) LJ_AISample_update);

	// setting up thread and semaphores
	sem_init (&x->busy, 0, 1);
	sem_init (&x->go, 0, 0);
	sem_init (&x->acquired, 0, 0);

	thread_start(x);

	return x; 
}

void LJ_AISample_free(t_LJ_AISample *x)
{
	while(pthread_cancel(x->x_threadid) < 0);	
}

