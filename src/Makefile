# This is a Makefile fragment, see ../Makefile

TOP = ..
include $(TOP)/configure/CONFIG

LIBRARY += pvaClient

INC += pvaClient.h

LIBSRCS += pvaClient.cpp
LIBSRCS += pvaClientPutData.cpp
LIBSRCS += pvaClientGetData.cpp
LIBSRCS += pvaClientMonitorData.cpp
LIBSRCS += pvaClientChannel.cpp
LIBSRCS += pvaClientProcess.cpp
LIBSRCS += pvaClientGet.cpp
LIBSRCS += pvaClientPut.cpp
LIBSRCS += pvaClientMonitor.cpp
LIBSRCS += pvaClientPutGet.cpp

pvaClient_LIBS += pvAccess pvData nt Com
pvaClient_LIBS +=  $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES

