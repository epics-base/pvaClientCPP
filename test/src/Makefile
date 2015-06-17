# Makefile for the pvAccess tests

TOP = ..
include $(TOP)/configure/CONFIG

TESTPROD_HOST += pvaClientTestPutData
pvaClientTestPutData_SRCS = pvaClientTestPutData 
testHarness_SRCS += pvaClientTestPutData.cpp 
TESTS += pvaClientTestPutData

TESTPROD_HOST += pvaClientTestGetData
pvaClientTestGetData_SRCS = pvaClientTestGetData 
testHarness_SRCS += pvaClientTestGetData.cpp 
TESTS += pvaClientTestGetData

TESTPROD_HOST += pvaClientTestMonitorData
pvaClientTestMonitorData_SRCS = pvaClientTestMonitorData 
testHarness_SRCS += pvaClientTestMonitorData.cpp 
TESTS += pvaClientTestMonitorData

TESTPROD_HOST += pvaClientTestPutGetMonitor
pvaClientTestPutGetMonitor_SRCS = pvaClientTestPutGetMonitor 
testHarness_SRCS += pvaClientTestPutGetMonitor.cpp 
TESTS += pvaClientTestPutGetMonitor

TESTPROD_HOST += pvaClientTestPutGet
pvaClientTestPutGet_SRCS = pvaClientTestPutGet
testHarness_SRCS += pvaClientTestPutGet.cpp 
TESTS += pvaClientTestPutGet

TESTPROD_HOST += pvaClientTestMultiDouble
pvaClientTestMultiDouble_SRCS = pvaClientTestMultiDouble
testHarness_SRCS += pvaClientTestMultiDouble.cpp 
TESTS += pvaClientTestMultiDouble

TESTPROD_HOST += pvaClientTestNTMultiChannel
pvaClientTestNTMultiChannel_SRCS = pvaClientTestNTMultiChannel
testHarness_SRCS += pvaClientTestNTMultiChannel.cpp 
TESTS += pvaClientTestNTMultiChannel


PROD_LIBS += pvaClient pvAccess pvData nt Com

testHarness_SRCS += pvaClientAllTests.c

PROD_vxWorks = vxTestHarness
vxTestHarness_SRCS += $(testHarness_SRCS)
TESTSPEC_vxWorks = vxTestHarness.$(MUNCH_SUFFIX); pvaClientAllTests

PROD_RTEMS += rtemsTestHarness
rtemsTestHarness_SRCS += rtemsTestHarness.c rtemsConfig.c
rtemsTestHarness_SRCS += $(testHarness_SRCS)
TESTSPEC_RTEMS = rtemsTestHarness.$(MUNCH_SUFFIX); pvaClientAllTests

TESTSCRIPTS_HOST += $(TESTS:%=%.t)

include $(TOP)/configure/RULES
