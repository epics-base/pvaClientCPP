# This is a Makefile fragment, see ../Makefile

TOP = ..
include $(TOP)/configure/CONFIG

LIBRARY += easyPVA

INC += easyPVA.h
INC += easyMultiDouble.h
INC += easyNTMultiChannel.h

LIBSRCS += easyPVA.cpp
LIBSRCS += easyPutData.cpp
LIBSRCS += easyGetData.cpp
LIBSRCS += easyMonitorData.cpp
LIBSRCS += easyChannel.cpp
LIBSRCS += easyProcess.cpp
LIBSRCS += easyGet.cpp
LIBSRCS += easyPut.cpp
LIBSRCS += easyMonitor.cpp
LIBSRCS += easyPutGet.cpp
LIBSRCS += easyMultiChannel.cpp
LIBSRCS += easyMultiDouble.cpp
LIBSRCS += easyNTMultiChannel.cpp
#LIBSRCS += easyRPC.cpp

easyPVA_LIBS += pvAccess pvData nt Com
easyPVA_LIBS +=  $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES

