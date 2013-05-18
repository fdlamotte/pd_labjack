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

#ifndef LJ_IOOut_H
#define LJ_IOOut_H

// Definition of the labjack class
t_class *LJ_IOOut_class;

typedef struct _LJ_IOOut
{
	t_object p_ob;

	int pad;

	// outlets 
	t_outlet *err;

	t_float data[4];
	long ports [4];
	int nb_ports;
	
	long lasterror;

	// thread data
	pthread_attr_t LJ_IOOut_thread_attr;
	pthread_t x_threadid;
	sem_t busy;
	sem_t go; // launch 
} t_LJ_IOOut;




void *LJ_IOOut_new(t_symbol *s, int argc, t_atom *argv);
void LJ_IOOut_close(t_LJ_IOOut *x);
void LJ_IOOut_open(t_LJ_IOOut *x);
void LJ_IOOut_sample(t_LJ_IOOut *x);

#endif
