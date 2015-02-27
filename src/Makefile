# This is a Makefile fragment, see ../Makefile

TOP = ..
include $(TOP)/configure/CONFIG

LIBRARY += easyPVA

INC += easyPVA.h

LIBSRCS += easyPVA.cpp
LIBSRCS += easyPVStructure.cpp
LIBSRCS += easyChannel.cpp
LIBSRCS += easyGet.cpp
#LIBSRCS += easyPut.cpp
#LIBSRCS += easyMonitor.cpp
#LIBSRCS += easyRPC.cpp

easyPVA_LIBS += pvAccess pvData Com
easyPVA_LIBS +=  $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES

