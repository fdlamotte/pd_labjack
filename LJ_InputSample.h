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

#ifndef LJ_INPUTSAMPLE_H
#define LJ_INPUTSAMPLE_H

// Definition of the labjack class
t_class *LJ_InputSample_class;

typedef struct _LJ_InputSample
{
	t_object p_ob;

	int padding;

	// outlets 
	t_outlet *ai[8];
	t_outlet *io[4];
	t_outlet *err;

	// channel config
	t_int channels;
	t_int ch_config[8];

	// values
	t_float ai_v[8];
	t_float io_v[4];

  	t_clock *x_clock;
	t_int errors;
	
	int lasterror;

	// thread data
	pthread_attr_t LJ_InputSample_thread_attr;
	pthread_t x_threadid;
	sem_t busy;
	sem_t go; // launch mesure
	sem_t acquired;
} t_LJ_InputSample;




void *LJ_InputSample_new(t_symbol *s, int argc, t_atom *argv);
void LJ_InputSample_close(t_LJ_InputSample *x);
void LJ_InputSample_open(t_LJ_InputSample *x);
void LJ_InputSample_sample(t_LJ_InputSample *x);

#endif
