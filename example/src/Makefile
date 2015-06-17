TOP=..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE


PROD_HOST += examplePvaClientProcess
examplePvaClientProcess_SRCS += examplePvaClientProcess.cpp
examplePvaClientProcess_LIBS += pvaClient
examplePvaClientProcess_LIBS += pvAccess
examplePvaClientProcess_LIBS += pvData
examplePvaClientProcess_LIBS += Com

PROD_HOST += examplePvaClientGet
examplePvaClientGet_SRCS += examplePvaClientGet.cpp
examplePvaClientGet_LIBS += pvaClient
examplePvaClientGet_LIBS += pvAccess
examplePvaClientGet_LIBS += pvData
examplePvaClientGet_LIBS += Com

PROD_HOST += examplePvaClientPut
examplePvaClientPut_SRCS += examplePvaClientPut.cpp
examplePvaClientPut_LIBS += pvaClient
examplePvaClientPut_LIBS += pvAccess
examplePvaClientPut_LIBS += pvData
examplePvaClientPut_LIBS += Com

PROD_HOST += examplePvaClientMonitor
examplePvaClientMonitor_SRCS += examplePvaClientMonitor.cpp
examplePvaClientMonitor_LIBS += pvaClient
examplePvaClientMonitor_LIBS += pvAccess
examplePvaClientMonitor_LIBS += pvData
examplePvaClientMonitor_LIBS += Com

PROD_HOST += examplePvaClientMultiDouble
examplePvaClientMultiDouble_SRCS += examplePvaClientMultiDouble.cpp
examplePvaClientMultiDouble_LIBS += pvaClient
examplePvaClientMultiDouble_LIBS += pvAccess
examplePvaClientMultiDouble_LIBS += pvData
examplePvaClientMultiDouble_LIBS += Com

PROD_HOST += examplePvaClientNTMultiChannel
examplePvaClientNTMultiChannel_SRCS += examplePvaClientNTMultiChannel.cpp
examplePvaClientNTMultiChannel_LIBS += pvaClient
examplePvaClientNTMultiChannel_LIBS += pvAccess
examplePvaClientNTMultiChannel_LIBS += nt
examplePvaClientNTMultiChannel_LIBS += pvData
examplePvaClientNTMultiChannel_LIBS += Com

PROD_HOST += helloWorldRPC
helloWorldRPC_SRCS += helloWorldRPC.cpp
helloWorldRPC_LIBS += pvaClient
helloWorldRPC_LIBS += pvAccess
helloWorldRPC_LIBS += nt
helloWorldRPC_LIBS += pvData
helloWorldRPC_LIBS += Com

PROD_HOST += helloWorldPutGet
helloWorldPutGet_SRCS += helloWorldPutGet.cpp
helloWorldPutGet_LIBS += pvaClient
helloWorldPutGet_LIBS += pvAccess
helloWorldPutGet_LIBS += nt
helloWorldPutGet_LIBS += pvData
helloWorldPutGet_LIBS += Com

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

