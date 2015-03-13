# Makefile for the pvAccess tests

TOP = ..
include $(TOP)/configure/CONFIG

TESTPROD_HOST += testEasyPutData
testEasyPutData_SRCS = testEasyPutData 
testHarness_SRCS += testEasyPutData.cpp 
TESTS += testEasyPutData

TESTPROD_HOST += testEasyGetData
testEasyGetData_SRCS = testEasyGetData 
testHarness_SRCS += testEasyGetData.cpp 
TESTS += testEasyGetData

TESTPROD_HOST += testEasyMonitorData
testEasyMonitorData_SRCS = testEasyMonitorData 
testHarness_SRCS += testEasyMonitorData.cpp 
TESTS += testEasyMonitorData

TESTPROD_HOST += testEasyPutGetMonitor
testEasyPutGetMonitor_SRCS = testEasyPutGetMonitor 
testHarness_SRCS += testEasyPutGetMonitor.cpp 
TESTS += testEasyPutGetMonitor

TESTPROD_HOST += testEasyPutGet
testEasyPutGet_SRCS = testEasyPutGet
testHarness_SRCS += testEasyPutGet.cpp 
TESTS += testEasyPutGet


PROD_LIBS += easyPVA pvAccess pvData Com

testHarness_SRCS += easyAllTests.c

PROD_vxWorks = vxTestHarness
vxTestHarness_SRCS += $(testHarness_SRCS)
TESTSPEC_vxWorks = vxTestHarness.$(MUNCH_SUFFIX); easyAllTests

PROD_RTEMS += rtemsTestHarness
rtemsTestHarness_SRCS += rtemsTestHarness.c rtemsConfig.c
rtemsTestHarness_SRCS += $(testHarness_SRCS)
TESTSPEC_RTEMS = rtemsTestHarness.$(MUNCH_SUFFIX); easyAllTests

TESTSCRIPTS_HOST += $(TESTS:%=%.t)

include $(TOP)/configure/RULES
