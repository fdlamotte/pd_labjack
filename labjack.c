
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
#include "LJ_AOOut.h"
#include "LJ_TrisIO.h"
#include "LJ_IOOut.h"
#include "LJ_IOIn.h"
#include "LJ_TrisD.h"
#include "LJ_DOut.h"
#include "LJ_DIn.h"
#include "LJ_InputSample.h"


/* valid for one Labjack */
#ifndef WIN32
HANDLE devHandle = NULL;
#endif


sem_t LJ_busy;
sem_t LJ_IO;
long LJ_trisD_fantom = 0;
long LJ_trisIO_fantom = 0;
long LJ_dataD_fantom = 0;
long LJ_dataIO_fantom = 0;

/*
 * run when importing library to pd
 * 
 * creates the classes for LJ objects
 */
int labjack_setup(void) 
{

	sem_init(&LJ_busy, 0, 1);
	sem_init(&LJ_IO, 0, 1);

	/*
	 * Definition of LJ_AISample
	 */
	LJ_AISample_class = class_new ( gensym("LJ_AISample"),
					(t_newmethod)LJ_AISample_new, 0, sizeof(t_LJ_AISample), 	
					CLASS_DEFAULT, A_GIMME, 0);
	
	class_addbang(LJ_AISample_class, (t_method)LJ_AISample_sample);


	/*
	 * Definition of LJ_InputSample
	 */
	LJ_InputSample_class = class_new ( gensym("LJ_InputSample"),
					(t_newmethod)LJ_InputSample_new, 0, sizeof(t_LJ_InputSample), 	
					CLASS_DEFAULT, A_GIMME, 0);
	
	class_addbang(LJ_InputSample_class, (t_method)LJ_InputSample_sample);


	/*
	 * Definition of LJ_AOOut
	 */
	LJ_AOOut_class = class_new ( gensym("LJ_AOOut"),
					(t_newmethod)LJ_AOOut_new, 0, sizeof(t_LJ_AOOut), 
					CLASS_DEFAULT, A_GIMME, 0);
	
	class_addbang(LJ_AOOut_class, (t_method)LJ_AOOut_sample);

	/*
	 * Definition of LJ_TrisIO
	 */
	LJ_TrisIO_class = class_new ( gensym("LJ_TrisIO"),
					(t_newmethod)LJ_TrisIO_new, 0, sizeof(t_LJ_TrisIO), 
					CLASS_DEFAULT, A_GIMME, 0);
	
	class_addbang(LJ_TrisIO_class, (t_method)LJ_TrisIO_sample);

	/*
	 * Definition of LJ_IOOut
	 */
	LJ_IOOut_class = class_new ( gensym("LJ_IOOut"),
					(t_newmethod)LJ_IOOut_new, 0, sizeof(t_LJ_IOOut), 
					CLASS_DEFAULT, A_GIMME, 0);
	
	class_addbang(LJ_IOOut_class, (t_method)LJ_IOOut_sample);


	/*
	 * Definition of LJ_IOIn
	 */
	LJ_IOIn_class = class_new ( gensym("LJ_IOIn"),
					(t_newmethod)LJ_IOIn_new, 0, sizeof(t_LJ_IOIn), 
					CLASS_DEFAULT, A_GIMME, 0);
	
	class_addbang(LJ_IOIn_class, (t_method)LJ_IOIn_sample);

	/*
	 * Definition of LJ_TrisD
	 */
	LJ_TrisD_class = class_new ( gensym("LJ_TrisD"),
					(t_newmethod)LJ_TrisD_new, 0, sizeof(t_LJ_TrisD), 
					CLASS_DEFAULT, A_GIMME, 0);
	
	class_addbang(LJ_TrisD_class, (t_method)LJ_TrisD_sample);

	/*
	 * Definition of LJ_DOut
	 */
	LJ_DOut_class = class_new ( gensym("LJ_DOut"),
					(t_newmethod)LJ_DOut_new, 0, sizeof(t_LJ_DOut), 
					CLASS_DEFAULT, A_GIMME, 0);
	
	class_addbang(LJ_DOut_class, (t_method)LJ_DOut_sample);


	/*
	 * Definition of LJ_DIn
	 */
	LJ_DIn_class = class_new ( gensym("LJ_DIn"),
					(t_newmethod)LJ_DIn_new, 0, sizeof(t_LJ_DIn), 
					CLASS_DEFAULT, A_GIMME, 0);
	
	class_addbang(LJ_DIn_class, (t_method)LJ_DIn_sample);



	post("labjack for pd version 0.1 (C) 2012 F. de Lamotte, ENSIBS", 0);
	return 1;
}


