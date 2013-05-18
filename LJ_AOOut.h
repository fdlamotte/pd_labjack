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

#ifndef LJ_AOOUT_H
#define LJ_AOOUT_H

// Definition of the labjack class
t_class *LJ_AOOut_class;

typedef struct _LJ_AOOut
{
	t_object p_ob;

	int pad;

	// outlets 
	t_outlet *err;

	// values
	t_float ao0;
	t_float ao1;

	t_int errors;

	// thread data
	pthread_attr_t LJ_AOOut_thread_attr;
	pthread_t x_threadid;
	sem_t busy;
	sem_t go; // launch 
} t_LJ_AOOut;




void *LJ_AOOut_new(t_symbol *s, int argc, t_atom *argv);
void LJ_AOOut_close(t_LJ_AOOut *x);
void LJ_AOOut_open(t_LJ_AOOut *x);
void LJ_AOOut_sample(t_LJ_AOOut *x);

#endif
