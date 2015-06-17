/*pvaClientTestPutGetMonitor.cpp */
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

#include <pv/pvaClient.h>
#include <epicsUnitTest.h>
#include <testMain.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;


class MyMonitor : public PvaClientMonitorRequester
{
public:
     MyMonitor() {}
     virtual ~MyMonitor() {}
     virtual void event(PvaClientMonitorPtr monitor)
     {
         while(true) {
            if(!monitor->poll()) return;
            PvaClientMonitorDataPtr pvaData = monitor->getData();
            cout << "changed\n";
            pvaData->showChanged(cout);
            cout << "overrun\n";
            pvaData->showOverrun(cout);
            monitor->releaseEvent();
            
         }
     }
};

static void exampleDouble(PvaClientPtr const &pvaClient)
{
    cout << "\nstarting exampleDouble\n";
    try {
        cout << "long way\n";
        PvaClientChannelPtr pvaChannel = pvaClient->createChannel("exampleDouble");
        pvaChannel->connect(2.0);
        testOk(true==true,"connected");
        PvaClientPutPtr put = pvaChannel->createPut();
        PvaClientPutDataPtr putData = put->getData();
        testOk(true==true,"put connected");
        PvaClientGetPtr get = pvaChannel->createGet();
        PvaClientGetDataPtr getData = get->getData();
        testOk(true==true,"get connected");
        PvaClientMonitorRequesterPtr requester(new MyMonitor());
        PvaClientMonitorPtr monitor = pvaChannel->monitor(requester);
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
        PvaClientProcessPtr process = pvaChannel->createProcess();
        process->connect();
        process->process();
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
}


MAIN(pvaClientTestPutGetMonitor)
{
    cout << "\nstarting pvaClientTestPutGetMonitor\n";
    testPlan(4);
    PvaClientPtr pvaClient = PvaClient::create();
    exampleDouble(pvaClient);
    cout << "done\n";
    return 0;
}
