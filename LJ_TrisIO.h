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

#ifndef LJ_TrisIO_H
#define LJ_TrisIO_H

// Definition of the labjack class
t_class *LJ_TrisIO_class;

typedef struct _LJ_TrisIO
{
	t_object p_ob;

	int pad;

	// outlets 
	t_outlet *err;

	t_float tris[4];
	long ports [4];
	int nb_ports;
	
	long lasterror;

	// thread data
	pthread_attr_t LJ_TrisIO_thread_attr;
	pthread_t x_threadid;
	sem_t busy;
	sem_t go; // launch 
} t_LJ_TrisIO;




void *LJ_TrisIO_new(t_symbol *s, int argc, t_atom *argv);
void LJ_TrisIO_close(t_LJ_TrisIO *x);
void LJ_TrisIO_open(t_LJ_TrisIO *x);
void LJ_TrisIO_sample(t_LJ_TrisIO *x);

#endif
