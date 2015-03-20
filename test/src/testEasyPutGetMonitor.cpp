/*testEasyPutGetMonitor.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 */

/* Author: Marty Kraimer */

#include <iostream>

#include <pv/easyPVA.h>
#include <epicsUnitTest.h>
#include <testMain.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::easyPVA;


class MyMonitor : public EasyMonitorRequester
{
public:
     MyMonitor() {}
     virtual ~MyMonitor() {}
     virtual void event(EasyMonitorPtr monitor)
     {
         while(true) {
            if(!monitor->poll()) return;
            EasyMonitorDataPtr easyData = monitor->getData();
            cout << "changed\n";
            easyData->showChanged(cout);
            cout << "overrun\n";
            easyData->showOverrun(cout);
            monitor->releaseEvent();
            
         }
     }
};

static void exampleDouble(EasyPVAPtr const &easyPVA)
{
    cout << "\nstarting exampleDouble\n";
    try {
        cout << "long way\n";
        EasyChannelPtr easyChannel = easyPVA->createChannel("exampleDouble");
        easyChannel->connect(2.0);
        testOk(true==true,"connected");
        EasyPutPtr put = easyChannel->createPut();
        EasyPutDataPtr putData = put->getData();
        testOk(true==true,"put connected");
        EasyGetPtr get = easyChannel->createGet();
        EasyGetDataPtr getData = get->getData();
        testOk(true==true,"get connected");
        EasyMonitorRequesterPtr requester(new MyMonitor());
        EasyMonitorPtr monitor = easyChannel->monitor(requester);
        testOk(true==true,"monitor connected");
        double out;
        double in;
        for(size_t i=0 ; i< 5; ++i) {
             out = i;
             putData->putDouble(out);
             put->put();
             get->get();
             in = getData->getDouble();
             cout << "out " << out << " in " << in << endl;
        }
        EasyProcessPtr process = easyChannel->createProcess();
        process->connect();
        process->process();
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
}


MAIN(testEasyPutGetMonitor)
{
    cout << "\nstarting testEasyPutGetMonitor\n";
    testPlan(4);
    EasyPVAPtr easyPVA = EasyPVA::create();
    exampleDouble(easyPVA);
    cout << "done\n";
    return 0;
}
