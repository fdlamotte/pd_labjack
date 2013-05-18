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

#ifndef LJ_IOIn_H
#define LJ_IOIn_H

// Definition of the labjack class
t_class *LJ_IOIn_class;

typedef struct _LJ_IOIn
{
	t_object p_ob;

	int pad;

	// outlets 
	t_outlet *io[4];
	t_outlet *err;

	long ports [4];
	long io_v [4];
	int nb_ports;
	
  	t_clock *x_clock;
	long lasterror;

	// thread data
	pthread_attr_t LJ_IOIn_thread_attr;
	pthread_t x_threadid;
	sem_t busy;
	sem_t go; // launch 
	sem_t acquired;
} t_LJ_IOIn;




void *LJ_IOIn_new(t_symbol *s, int argc, t_atom *argv);
void LJ_IOIn_close(t_LJ_IOIn *x);
void LJ_IOIn_open(t_LJ_IOIn *x);
void LJ_IOIn_sample(t_LJ_IOIn *x);

#endif
