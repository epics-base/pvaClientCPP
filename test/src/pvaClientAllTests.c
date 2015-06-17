/*
 * Run pvaClient tests as a batch.
 *
 * Do *not* include performance measurements here, they don't help to
 * prove functionality (which is the point of this convenience routine).
 */

#include <stdio.h>
#include <epicsThread.h>
#include <epicsUnitTest.h>

int pvaClientTestGetData(void);
int pvaClientTestPutData(void);
int pvaClientTestMonitorData(void);
int pvaClientTestPutGetMonitor(void);
int pvaClientTestPutGet(void);
int pvaClientTestMultiDouble(void);
int pvaClientTestNTMultiChannel(void);

void easyAllTests(void)
{
    testHarness();
    runTest(pvaClientTestGetData);
    runTest(pvaClientTestPutData);
    runTest(pvaClientTestMonitorData);
    runTest(pvaClientTestPutMonitor);
    runTest(pvaClientTestPut);
    runTest(pvaClientTestMultiDouble);
    runTest(pvaClientTestNTMultiChannel);
}

