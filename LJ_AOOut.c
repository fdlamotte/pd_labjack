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
#include "LJ_AOOut.h"

/*
 * Waits the go semaphore to update analog ports of the Labjack
 * Using AOUpdate function as detailed in chapter 4.6 of U12 user guide
 * http://labjack.com/support/u12/users-guide/4.6
 */
static void *LJ_AOOut_setup_thread(void *w)
{
    	t_LJ_AOOut *x = (t_LJ_AOOut*) w;

    	// Setup the variables we will need.
    	int r = 0; // For checking return values

	long stateD;
	long stateIO;
	long gains[] = {0, 0, 0, 0};
	long idnum = -1;
	long overvoltage;
	long count;

	int i;

    	while(1) {
    		sem_wait(&x->go);

		sem_wait (&LJ_busy);
		r = AOUpdate(&idnum, 0, 0, 0, &stateD, &stateIO, 0, 0, &count,
				x->ao0, x->ao1);
		sem_post (&LJ_busy);

    		sem_post(&x->busy);     // on libere le thread
    	}

}

static void LJ_AOOut_thread_start(t_LJ_AOOut *x)
{
	// create the worker thread
    	if(pthread_attr_init(&x->LJ_AOOut_thread_attr) < 0){
       		error("labjack: could not launch acquire thread");
       		return;
    	} else if(pthread_attr_setdetachstate(&x->LJ_AOOut_thread_attr, PTHREAD_CREATE_DETACHED) < 0) {
	   	error("labjack: could not launch acquire thread");
	   	return;
    	}

    	if(pthread_create(&x->x_threadid, &x->LJ_AOOut_thread_attr, LJ_AOOut_setup_thread, x) < 0) {
	   	error("labjack: could not launch aquire thread");
       		return;
    	}
}


/*
 * Executed when banged ... take a shot ;)
 */
void LJ_AOOut_sample(t_LJ_AOOut *x)
{
	if (sem_trywait(&(x->busy)) != -1) {	
		sem_post(&x->go);
	} else {
		outlet_bang(x->err);
	}
}

void *LJ_AOOut_new(t_symbol *s, int argc, t_atom *argv)
{
	t_LJ_AOOut *x;	
	x = (t_LJ_AOOut *)pd_new(LJ_AOOut_class);
	int i;
	
	// inlets
	floatinlet_new (&x->p_ob, &x->ao0);
	floatinlet_new (&x->p_ob, &x->ao1);

	x->err = outlet_new(&x->p_ob, &s_bang);
	x->errors = 0;

	// setting up thread and semaphores
	sem_init (&x->busy, 0, 1);
	sem_init (&x->go, 0, 0);

	LJ_AOOut_thread_start(x);

	return x; 
}

void LJ_AOOut_free(t_LJ_AOOut *x)
{
	while(pthread_cancel(x->x_threadid) < 0);	
}

