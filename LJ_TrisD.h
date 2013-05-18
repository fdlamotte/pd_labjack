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

#ifndef LJ_TrisD_H
#define LJ_TrisD_H

// Definition of the labjack class
t_class *LJ_TrisD_class;

typedef struct _LJ_TrisD
{
	t_object p_ob;

	int pad;

	// outlets 
	t_outlet *err;

	t_float tris[16];
	long ports [16];
	int nb_ports;
	
	long lasterror;

	// thread data
	pthread_attr_t LJ_TrisD_thread_attr;
	pthread_t x_threadid;
	sem_t busy;
	sem_t go; // launch 
} t_LJ_TrisD;




void *LJ_TrisD_new(t_symbol *s, int argc, t_atom *argv);
void LJ_TrisD_close(t_LJ_TrisD *x);
void LJ_TrisD_open(t_LJ_TrisD *x);
void LJ_TrisD_sample(t_LJ_TrisD *x);

#endif
