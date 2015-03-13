/*monitorPowerSupply.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 */

/* Author: Marty Kraimer */

#include <epicsThread.h>

#include <iostream>

#include <pv/easyPVA.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::easyPVA;


static void exampleMonitor(EasyPVAPtr const &easyPVA)
{
    EasyMonitorPtr monitor = easyPVA->channel("examplePowerSupply")->monitor("");
    EasyMonitorDataPtr easyData = monitor->getData();
    while(true) {
         monitor->waitEvent();
         cout << "changed\n";
         easyData->showChanged(cout);
         cout << "overrun\n";
         easyData->showOverrun(cout);
         monitor->releaseEvent();
     }
}


int main(int argc,char *argv[])
{
    EasyPVAPtr easyPVA = EasyPVA::create();
    exampleMonitor(easyPVA);
    cout << "done\n";
    return 0;
}
