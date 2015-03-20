/*
 * Run EasyPVA tests as a batch.
 *
 * Do *not* include performance measurements here, they don't help to
 * prove functionality (which is the point of this convenience routine).
 */

#include <stdio.h>
#include <epicsThread.h>
#include <epicsUnitTest.h>

int testEasyGetData(void);
int testEasyPutData(void);
int testEasyMonitorData(void);
int testEasyPutGetMonitor(void);
int testEasyPutGet(void);
int testEasyMultiDouble(void);
int testEasyNTMultiChannel(void);

void easyAllTests(void)
{
    testHarness();
    runTest(testEasyGetData);
    runTest(testEasyPutData);
    runTest(testEasyMonitorData);
    runTest(testEasyPutMonitor);
    runTest(testEasyPut);
    runTest(testEasyMultiDouble);
    runTest(testEasyNTMultiChannel);
}

