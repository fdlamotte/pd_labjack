
# labjack - PD External for LabJack Cards
# 
# Ecole Nationale Superieure d'Ingenieurs de Bretagne Sud
# http://ensibs.fr
# Copyright (C) 2012 Florent de Lamotte
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1
# of the License, or (at your option) any later version. (see LICENCE.txt)


OS=$(findstring NT,$(shell uname))

LABJACK_SOURCES=labjack.c 
LABJACK_SOURCES+=LJ_AISample.c 
LABJACK_SOURCES+=LJ_InputSample.c 
LABJACK_SOURCES+=LJ_AOOut.c 
LABJACK_SOURCES+=LJ_TrisIO.c 
LABJACK_SOURCES+=LJ_IOOut.c 
LABJACK_SOURCES+=LJ_IOIn.c 
LABJACK_SOURCES+=LJ_TrisD.c 
LABJACK_SOURCES+=LJ_DOut.c 
LABJACK_SOURCES+=LJ_DIn.c

ifeq ($(OS), NT)
	LJ_DRIVER_DIR=/c/Program\ Files/LabJackU12Legacy/drivers
	PD_DIR=/c/Program\ Files/pd
	PD_INCLUDE_DIR=$(PD_DIR)/include
	PD_DLL=$(PD_DIR)/bin/pd.dll
	CFLAGS=-I $(LJ_DRIVER_DIR) -I $(PD_INCLUDE_DIR) -DWIN32
all : labjack.dll
else
	LABJACK_SOURCES+=LJ_Utils.c 
	CFLAGS=-fPIC
all : labjack.pd_linux
endif

LDFLAGS = -shared

LABJACK_OBJS = $(LABJACK_SOURCES:%.c=%.o)

labjack.pd_linux : $(LABJACK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(LABJACK_OBJS) -llabjackusb -lm

labjack.dll : $(LABJACK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(LABJACK_OBJS) $(PD_DLL) -Wl,--subsystem,windows -lpthread -L$(LJ_DRIVER_DIR) -lljackuw


clean :
	rm -rf *.o *.dll *.pd_linux

